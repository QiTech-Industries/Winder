#include <global.h>
#include <Wifi.cpp>
#include <Webserver.cpp>
#include <Stepper.cpp>
#include <Updater.cpp>

Updater updater;
Wifi wifi;
Webserver server;
Stepper ferrari, spool, puller;
Timer timer, report;
float_t spoolSpeedToFerrariSpeed;
bool ferrariReady = false;

void printBanner() { DEBUG_PRINTLN("\n   ___                  _       _    _ _           _           \n  |_  |                (_)     | |  | (_)         | |          \n    | | __ _ _ ____   ___ ___  | |  | |_ _ __   __| | ___ _ __ \n    | |/ _` | '__\\ \\ / / / __| | |/\\| | | '_ \\ / _` |/ _ \\ '__|\n/\\__/ / (_| | |   \\ V /| \\__ \\ \\  /\\  / | | | | (_| |  __/ |   \n\\____/ \\__,_|_|    \\_/ |_|___/  \\/  \\/|_|_| |_|\\__,_|\\___|_|   \n"); }

void power(bool on)
{
  if (on)
  {
    ferrari.on();
    spool.on();
    puller.on();
    mode = POWER;
  }
  else
  {
    ferrari.off();
    spool.off();
    puller.off();
    mode = STANDBY;
  }
}

bool isMode(mode_e nextMode)
{
  // Mode does not change
  if (mode == nextMode)
  {
    return true;
  }
  // New mode is set
  else
  {
    power(true); // Always reset Motors on Mode change
    timer.reset();
    mode = nextMode;
    return false;
  }
}

void unwind(float mpm)
{
  float speed = mpm * 1000 / hard.motors.puller.mm_per_rotation / 60;

  isMode(UNWINDING);
  spool.off();
  puller.add({.rps = -speed, .mm = 0, ROTATE});
}

void pull(float mpm)
{
  float speed = mpm * 1000 / hard.motors.puller.mm_per_rotation / 60;
  ferrariReady = false;

  isMode(PULLING);
  puller.add({.rps = speed, .mm = 0, ROTATE});
  if (!ferrari.getHomed())
  {
    ferrari.add({.rps = -2, .mm = 0, HOME});
  }
  ferrari.add({.rps = 2, .mm = soft.ferrari_max, POSITION, []
               {
                 ferrariReady = true;
               }});
}

void calibrate(uint16_t position, bool startPos = true)
{
  if (isMode(CALIBRATING))
  {
    ferrari.position(2, position);
  }
  else
  {
    if (!ferrari.getHomed())
      ferrari.add({.rps = -2, .mm = 0, HOME});

    ferrari.add({2, position, POSITION, []
                 {
                   isMode(POWER);
                 }});
  }

  if (startPos)
    soft.ferrari_min = position;
  else
    soft.ferrari_max = position;
  soft.store();
}

void updateFerrariSpeed()
{
  ferrari.setSpeedUs(spool.getSpeedUs() * spoolSpeedToFerrariSpeed);
};

void wind(float mpm)
{
  if (!ferrariReady)
    return;
  mode = WINDING;
  float speed = mpm * 1000 / hard.motors.puller.mm_per_rotation / 60;

  // START RUNNING the motors
  ferrari.add({.rps = 1, .mm = soft.ferrari_min, POSITION});
  ferrari.add({.rps = 1, .mm = soft.ferrari_max, POSITION});
  ferrari.loop(2);
  spool.add({.rps = -3, .mm = 0, ADJUST});
  puller.add({.rps = speed, .mm = 0, ROTATE});

  //SPEED CALCULATIONS
  // tested: when winded filament width = 2.1mm theory 1.75mm
  float_t spoolWidth = (soft.ferrari_max - soft.ferrari_min);
  float_t spoolMotorRotationsPerLayer = spoolWidth * hard.motors.spool.gear_ratio / 1.75;
  float_t ferrariMotorRotationsPerLayer = spoolWidth / hard.motors.ferrari.mm_per_rotation;
  spoolSpeedToFerrariSpeed = spoolMotorRotationsPerLayer / ferrariMotorRotationsPerLayer;

  // SETUP TIMERS for ferrari and spool speed sync
  timer.setCallback(updateFerrariSpeed);
  timer.setInterval(100);
  timer.start();
}

// Send machine status to web ui every Xs
///////////////////////////////////
void reportStatus()
{
  if (connection == CONNECTING)
    return;
  StaticJsonDocument<512> doc;
  String json;

  auto fS = ferrari.getStatus();
  auto sS = spool.getStatus();
  auto pS = puller.getStatus();

  Serial.println(fS.error);

  doc["f"]["r"] = fS.rpm;
  doc["f"]["s"] = fS.stall;
  doc["f"]["a"] = fS.active;
  doc["p"]["r"] = pS.rpm;
  doc["p"]["s"] = pS.stall;
  doc["p"]["a"] = pS.active;
  doc["s"]["r"] = sS.rpm;
  doc["s"]["s"] = sS.stall;
  doc["s"]["a"] = sS.active;
  doc["m"] = mode2string();
  doc["e"] = nullptr;
  doc["w"] = 0; //sS.rotations;
  doc["l"] = 0; //fS.rotations * hard.motors.ferrari.mm_per_rotation / 1000;
  if (!fS.error.isEmpty())
    doc["e"] = fS.error;
  else if (!sS.error.isEmpty())
    doc["e"] = sS.error;
  else if (!pS.error.isEmpty())
    doc["e"] = pS.error;

  if (serializeJson(doc, json) == 0)
  {
    Serial.println("[Winder] JSON serialization failed!");
  }

  server.emit("stats", json);
}
///////////////////////////////////

class Winder
{
public:
  void setup(config_s conf)
  {
    printBanner();

    // Load config from main.cpp
    ///////////////////////////////////
    hard = conf.hard;
    soft = conf.soft;
    ///////////////////////////////////

    // Initialize Stepper Motors
    ///////////////////////////////////
    ferrari.init(hard.motors.ferrari);
    spool.init(hard.motors.spool);
    puller.init(hard.motors.puller);
    ///////////////////////////////////

    // Load config from SPIFFS and BLYNK
    ///////////////////////////////////
    if (soft.load())
      power(true);
    else
      power(false);
    DEBUG_PRINTLN(soft.asJSON());
    ///////////////////////////////////

    // Create HTTP and WS server
    ///////////////////////////////////
    server.create(hard.server.port, hard.server.default_path, soft.wifi.mdns_name, soft.wifi.friendly_name);
    server.createSocket("/ws");
    ///////////////////////////////////

    // Connect to Wifi and create AP
    ///////////////////////////////////
    if (strlen(soft.wifi.ssid))
    {
      wifi.connect(soft.wifi.ssid, soft.wifi.password);
    }
    if (soft.wifi.ap_enabled || !strlen(soft.wifi.ssid))
    {
      wifi.createAP(soft.wifi.ap_ssid, soft.wifi.ap_password);
      server.createCaptive(WiFi.softAPIP());
    }
    ///////////////////////////////////

    // Setup Timers
    ///////////////////////////////////
    updater.setInterval(30000);
    report.setInterval(1000);
    report.setCallback(reportStatus);
    report.start();
    ///////////////////////////////////

    // Handle incoming socket events
    ///////////////////////////////////
    server.on("connect", [](JsonObject data)
              {
                wifi.connect(data["name"], data["password"]);
                return String();
              });
    server.on("scan", [](JsonObject data)
              { return wifi.scan(); });
    server.on("power", [](JsonObject data)
              {
                power(true);
                return String();
              });
    server.on("standby", [](JsonObject data)
              {
                power(false);
                return String();
              });
    server.on("calibrate", [](JsonObject data)
              {
                uint16_t position = data["position"].as<uint16_t>();
                bool startPos = data["startPos"].as<bool>();
                calibrate(position, startPos);
                return String();
              });
    server.on("wind", [](JsonObject data)
              {
                float mpm = data["mpm"].as<float>();
                wind(mpm);
                return String();
              });
    server.on("unwind", [](JsonObject data)
              {
                float mpm = data["mpm"].as<float>();
                unwind(mpm);
                return String();
              });
    server.on("pull", [](JsonObject data)
              {
                float mpm = data["mpm"].as<float>();
                pull(mpm);
                return String();
              });
    server.on("speed", [](JsonObject data)
              {
                float mpm = data["mpm"].as<float>();
                float speed = mpm * 1000 / hard.motors.puller.mm_per_rotation / 60;
                if (mode == WINDING || mode == PULLING)
                  puller.setSpeed(speed);
                else if (mode == UNWINDING)
                  puller.setSpeed(-speed);
                return String();
              });
    server.on("modify", [](JsonObject data)
              {
                soft.wifi.ap_enabled = data["ap_enabled"];
                if (data["mdns_name"] != "")
                {
                  strcpy(soft.wifi.mdns_name, data["mdns_name"]);
                }
                if (data["ap_ssid"] != "")
                {
                  strcpy(soft.wifi.ap_ssid, data["ap_ssid"]);
                }

                soft.store();
                return String("\"stored\"");
              });
    server.on("config", [](JsonObject data)
              { return soft.asJSON(); });
    ///////////////////////////////////

    // Respond to Wifi connection changes
    ///////////////////////////////////
    wifi.conChange([]()
                   {
                     switch (connection)
                     {
                     case OFFLINE:
                       server.emit("connect", "\"failed\"");
                       break;
                     case ONLINE:
                       server.emit("connect", "\"connected\"");
                       break;
                     default:
                       break;
                     }
                   });
    ///////////////////////////////////
  }

  void loop()
  {
    TimerManager::instance().update();
  }
};

Winder JarvisWinder;