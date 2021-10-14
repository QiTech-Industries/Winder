#include <Arduino.h>
#include <SPI.h>
#include <TMCStepper.h>
#include <FastAccelStepper.h>

using TMC2130_n::DRV_STATUS_t;

// Diese Werte können interaktiv konfiguriert werden
float_t stepperGearRatio;
float_t minRpm; // 0,5m/min / (0,19m*pi)
float_t maxRpm; // 14m/min / (0,1m*pi)
//

// configuration for spool motor

// Spool
//uint16_t stepperDir = 16;
//uint16_t stepperEn = 12;
//uint16_t stepperStep = 26;
//uint16_t stepperCs = 5;

// Puller
uint16_t stepperDir = 27;
uint16_t stepperEn = 12;
uint16_t stepperStep = 25;
uint16_t stepperCs = 2;

// Ferrari
//uint16_t stepperDir = 14;
//uint16_t stepperEn = 12;
//uint16_t stepperStep = 17;
//uint16_t stepperCs = 13;

uint16_t stepperMicrosteps = 16;
uint16_t stepsPerRotation = 200;
uint16_t calibrationCount = 40;

struct datapoint_s
{
    uint32_t Us;
    float_t Rpm;
    float_t noLoad;
    float_t maxLoad;
};

std::vector<datapoint_s> datapoints;

float_t stepWidth;
float_t microstepsPerRotation;

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper = NULL;
TMC2130Stepper driver(stepperCs, 0.11f); 

float_t rpm2us(float_t rpm)
{
    if (rpm == 0)
        return 0;
    // mikrosekunden pro minute / roatationen pro minute / schritte pro rotation
    return (float)60000000 / rpm / microstepsPerRotation;
}

float_t us2rpm(int32_t us)
{
    if (us == 0)
        return 0;
    return (float)60000000 / abs(us) / microstepsPerRotation;
}

String waitForEnter()
{
    while (true) // remain here until told to return
    {
        while (!Serial.available())
            ;
        return Serial.readStringUntil('\n');
    }
}

void instructions()
{
    Serial.println("\n 🛈 Der Motor wird nun von der minimalen bis zur maximalen Geschwindigkeit beschleunigen.");
    Serial.println("🛈 Sobald der Motor aufhört zu beschleunigen muss der Motor per Hand bis zum Stall gebremst werden.");
    Serial.println("🛈 Der Stall Wert, der bei maximaler Last angezeigt wird muss dann in die Konsole eingegeben werden.");
    Serial.println("🛈 Der Motor beschleunigt dann zum nächsten Punkt weiter.");
    Serial.println("🛈 Sobald die Kalibration abgeschlossen ist werden die gemessenen Werte als CSV für die Weiterverarabeitung ausgegeben.");
    waitForEnter();
}

void printConfig()
{
    Serial.println("\n🛈 Die Kalibrierung startet mit folgenden Parametern:");
    Serial.print("==> minRpm: ");
    Serial.print(minRpm);
    Serial.print("  maxRpm: ");
    Serial.print(maxRpm);
    Serial.print("  gearRatio: ");
    Serial.println(stepperGearRatio);
    waitForEnter();
}

void setConfig()
{
    Serial.println("🛈 Dieses Skript kann verwendet werden, um die Stall Werte der Motoren zu kalibrieren. Dafür müssen zunächst drei Parameter in die Konsole eigegeben werden. Kommazahlen müssen mit . eingegeben werden.");
    Serial.println("Eine Eingabe wird stets mit Enter bestätigt.");
    Serial.println("\nminRpm (minimale Geschwindigkeit, bei der die Stall Erkennung funktionieren muss)");
    minRpm = waitForEnter().toFloat();
    Serial.println("maxRpm (maximale Geschwindigkeit, bei der die Stall Erkennung funktionieren muss)");
    maxRpm = waitForEnter().toFloat();
    Serial.println("stepperGearRatio (entweder 1 oder 1/13.73 oder 1/5.18 als float)");
    stepperGearRatio = waitForEnter().toFloat();

    stepWidth = (maxRpm - minRpm) / (calibrationCount - 1);
    microstepsPerRotation = stepsPerRotation * stepperMicrosteps * stepperGearRatio;
    Serial.println(microstepsPerRotation);
}

void addCalibrationPoint()
{
    datapoint_s datapoint;
    uint16_t cummulatedStall = 0;

    for (uint16_t i = 0; i < 5; i++)
    {
        delay(150);
        DRV_STATUS_t drv_status{0};
        drv_status.sr = driver.DRV_STATUS();
        cummulatedStall += drv_status.sg_result;
    }

    datapoint.noLoad = (float)cummulatedStall / 5;
    datapoint.Us = stepper->getCurrentSpeedInUs();
    datapoint.Rpm = us2rpm(stepper->getCurrentSpeedInUs());

    Serial.println("Neue Zielgeschwindigkeit erreicht. Jetzt bitte den Motor maximal unter Last setzen und 5x Enter drücken um einen Durchschnitt zu berechnen.");
    Serial.println("Sample Wert Nr.:");

    cummulatedStall = 0;

    for (uint16_t i = 0; i < 5; i++)
    {
        Serial.print(i + 1);
        Serial.print(" : ");
        waitForEnter();
        DRV_STATUS_t drv_status{0};
        drv_status.sr = driver.DRV_STATUS();
        cummulatedStall += drv_status.sg_result;
        Serial.println(drv_status.sg_result);
    }

    datapoint.maxLoad = (float)cummulatedStall / 5;
    datapoints.push_back(datapoint);
}

void resultAsCsv()
{
    String result;

    Serial.println("\n🛈 Die Kalibration wurde erfolgreich abgeschlossen!");
    Serial.println("🛈 Die Werte in der CSV sind wie folgt angeordnet: Rpm, Geschwindigkeit Us, Stall-Wert bei keiner Last, Stall-Wert bei maximaler Last.");
    Serial.println("🛈 Punkte müssen für ein korrekte Verarbeitung in der Tabellenkalkulation durch Kommas ersetzt werden");
    Serial.println("🛈 Terminalgröße keinesfalls verändern, da sonst Werte verloren gehen können.\n");
    for (auto i = datapoints.begin(); i != datapoints.end(); i++)
    {
        result += i->Rpm;
        result += ";";
        result += i->Us;
        result += ";";
        result += i->noLoad;
        result += ";";
        result += i->maxLoad;
        result += "\n";
    }

    Serial.println(result);
}

void start()
{
    setConfig();
    printConfig();
    instructions();

    for (uint8_t i = 0; i < calibrationCount; i++)
    {
        float_t targetRpm = minRpm + stepWidth * i;
        float_t targetUs = rpm2us(targetRpm);

        Serial.print("\n==> currentRpm: ");
        Serial.print(us2rpm(stepper->getCurrentSpeedInUs()));
        Serial.print("  targetRpm: ");
        Serial.print(targetRpm);
        Serial.print("  CalibrationPoint: ");
        Serial.println(i + 1);
        waitForEnter();

        stepper->setSpeedInUs(targetUs);
        stepper->applySpeedAcceleration();
        stepper->runForward();

        // wait until motor has started accellerating
        while (!stepper->getCurrentAcceleration())
        {
        };
        // wait until motor has finished accellerating
        while (stepper->getCurrentAcceleration())
        {
        };
        addCalibrationPoint();
    }

    driver.toff(0);
    resultAsCsv();
}

void setup()
{
    digitalWrite(stepperEn, LOW);
    SPI.begin();
    Serial.begin(115200);
    Serial.setTimeout(-1); // stop reading input only on enter (\n)

    driver.begin();
    engine.init();

    driver.toff(1);
    driver.blank_time(5);
    driver.rms_current(700);
    driver.microsteps(stepperMicrosteps);
    driver.sgt(34);
    driver.sfilt(true);

    // StallGuard/Coolstep config
    driver.TCOOLTHRS(1000000);
    driver.semin(0);
    driver.semax(1);

    stepper = engine.stepperConnectToPin(stepperStep);
    if (stepper)
    {
        stepper->setDirectionPin(stepperDir);
        stepper->setEnablePin(stepperEn);
        stepper->setAutoEnable(true);
        stepper->setAcceleration(1000);
    }

    start();
}

void loop() {}