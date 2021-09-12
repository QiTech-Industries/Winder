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

void printBanner() { DEBUG_PRINTLN("\n   ___                  _       _    _ _           _           \n  |_  |                (_)     | |  | (_)         | |          \n    | | __ _ _ ____   ___ ___  | |  | |_ _ __   __| | ___ _ __ \n    | |/ _` | '__\\ \\ / / / __| | |/\\| | | '_ \\ / _` |/ _ \\ '__|\n/\\__/ / (_| | |   \\ V /| \\__ \\ \\  /\\  / | | | | (_| |  __/ |   \n\\____/ \\__,_|_|    \\_/ |_|___/  \\/  \\/|_|_| |_|\\__,_|\\___|_|   \n"); }

void unwind(float mpm = 1)
{
  float speed = mpm * 1000 / hard.motors.puller.mm_per_rotation / 60;
  if (mode == UNWINDING)
  {
    puller.setSpeed(speed);
    return;
  }

  mode = UNWINDING;
  puller.add({.rps = -speed, .mm = 0, ROTATE});
}

void power()
{
  if (mode == STANDBY)
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

void home()
{
  mode = HOMING;
  ferrari.add({.rps = -1, .mm = 0, HOME, []
               {
                 DEBUG_PRINTLN("HOMING FINISHED");
                 mode = STANDBY;
               }});
}

void calibrate(uint16_t position, bool startPos = true)
{
  if (mode == HOMING || position == 0)
    return;

  if (startPos)
  {
    soft.ferrari_min = position;
  }
  else
  {
    soft.ferrari_max = position;
  }
  soft.store();

  if (mode == CALIBRATING)
  {
    ferrari.position(2, position);
    return;
  }
  mode = CALIBRATING;
  ferrari.add({2, position, POSITION});
}

void updateFerrariSpeed()
{
  //SPEED CALCULATIONS
  float_t spoolWidth = (soft.ferrari_max - soft.ferrari_min);
  // tested: when winded filament width = 2.1mm theory 1.75mm
  float_t spoolMotorRotationsPerLayer = spoolWidth / 1.9 / hard.motors.spool.gear_ratio;
  float_t ferrariMotorRotationsPerLayer = spoolWidth / hard.motors.ferrari.mm_per_rotation;
  float_t spoolSpeedToFerrariSpeed = spoolMotorRotationsPerLayer / ferrariMotorRotationsPerLayer;

  ferrari.setSpeedUs(spool.getSpeedUs() * spoolSpeedToFerrariSpeed);
  //
};

void wind(float mpm, bool start = false)
{
  if (mode == HOMING)
    return;
  if (!start)
  {
    float speed = mpm * 1000 / hard.motors.puller.mm_per_rotation / 60;
    if (mode != WINDING)
    {
      mode = WINDING;
      puller.add({.rps = speed, .mm = 0, ROTATE});
    }
    puller.setSpeed(speed);
    return;
  }
  mode = WINDING;

  // START RUNNING the motors
  ferrari.add({.rps = 1, .mm = soft.ferrari_max, POSITION, []
               {
                 ferrari.add({.rps = 1, .mm = soft.ferrari_min, POSITION});
                 ferrari.add({.rps = 1, .mm = soft.ferrari_max, POSITION});
                 ferrari.loop(2);
                 spool.add({.rps = 0.25, .mm = 0, ADJUST});

                 //SETUP TIMER for Ferrari Speed update
                 timer.setCallback(updateFerrariSpeed);
                 timer.setInterval(100);
                 timer.start();
                 //
               }});
  //
}

void reportStatus()
{
  wifi.updateStatus();

  StaticJsonDocument<512> doc;
  String json;

  auto fS = ferrari.getStatus();
  auto sS = spool.getStatus();
  auto pS = puller.getStatus();

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
  if (!fS.error.isEmpty())
    doc["e"] = fS.error;
  else if (!sS.error.isEmpty())
    doc["e"] = sS.error;
  else if (!pS.error.isEmpty())
    doc["e"] = pS.error;

  if (serializeJson(doc, json) == 0)
  {
    Serial.println("JSON serialization failed!");
  }

  server.emit("stats", json);
}

class Winder
{
public:
  void setup(config_s conf)
  {
    printBanner();

    // Load config from correct locations
    ///////////////////////////////////
    hard = conf.hard;
    soft = conf.soft;
    soft.loadBlynkCredentials();
    soft.load();
    ///////////////////////////////////

    // Create HTTP and WS server
    ///////////////////////////////////
    server.create(hard.server.port, hard.server.default_path, soft.wifi.mdns_name, soft.wifi.friendly_name);
    server.createSocket("/ws");
    ///////////////////////////////////

    // Connect to Wifi and create AP
    ///////////////////////////////////
    if (soft.wifi.ap_enabled)
    {
      wifi.createAP(soft.wifi.ap_ssid, soft.wifi.ap_password);
      server.createCaptive(WiFi.softAPIP());
    }
    if (strlen(soft.wifi.ssid))
    {
      wifi.connect(soft.wifi.ssid, soft.wifi.password);
    }
    ///////////////////////////////////

    // Initialize Stepper Motors
    ///////////////////////////////////
    ferrari.init(hard.motors.ferrari);
    spool.init(hard.motors.spool);
    puller.init(hard.motors.puller);
    ///////////////////////////////////

    // Setup Timers
    ///////////////////////////////////
    updater.setInterval(10000);
    report.setInterval(1000);
    report.setCallback(reportStatus);
    report.start();
    ///////////////////////////////////

    // Handle incoming socket events
    ///////////////////////////////////
    server.on("ws_connect", []()
              {
                DEBUG_PRINTLN("Client connected to Websocket.");
                return String();
              });
    server.on("ws_disconnect", []()
              {
                DEBUG_PRINTLN("Client disconnected from Websocket.");
                return String();
              });
    server.on("connect", [](JsonObject data)
              {
                wifi.connect(data["name"], data["password"]);
                connection = CONNECTING;
                return String();
              });
    server.on("scan", [](JsonObject data)
              { return wifi.scan(); });
    server.on("power", [](JsonObject data)
              {
                power();
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
                bool start = data["start"].as<bool>();
                wind(mpm, start);
                return String();
              });
    server.on("unwind", [](JsonObject data)
              {
                float mpm = data["mpm"].as<float>();
                unwind(mpm);
                return String();
              });
    server.on("config", [](JsonObject data)
              { return soft.asJSON(); });
  }
  ///////////////////////////////////

  void loop()
  {
    TimerManager::instance().update();
  }
};

Winder JarvisWinder;