#include <Winder.h>

// TODO: Replace with proper debugging/logging
#ifdef DEBUG_WINDER
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
#endif

#include <timerManager.h> // Allow classes to register timers

#include <WinderWifi.h>
#include <Webserver.h>
#include <Stepper.h>
#include <QiUpdater.h>

#include "logger/logging.h"

// TODO: Move global into class as attributes?
QiUpdater updater;
WinderWifi wifi; // TODO: rename as it can be confused with the WiFi singleton of the arduino library
Webserver server;
Stepper ferrari, spool, puller;
Timer timer, report;
float_t spoolSpeedToFerrariSpeed; // Ratio between spool speed and ferrari speed (spoolMotorRotationsPerLayer / ferrariMotorRotationsPerLayer)
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
    power(true); // Always reset Motors on mode change
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
  // TODO: Comment
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
                   Serial.println("Callback"); // TODO: Debug/Logging
                   isMode(POWER);
                 }});
  }

  // TODO: Comment
  if (startPos)
    soft.ferrari_min = position;
  else
    soft.ferrari_max = position;
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
  spool.add({.rps = 3, .mm = 0, ADJUST});
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

void change()
{
  ferrari.on();
  spool.on();
  ferrari.add({.rps = 2, .mm = soft.ferrari_max, POSITION});
  mode = CHANGING;
};

void reportStatus()
{
  if (connection == CONNECTING)
    return;

  stepperStatus_s fS = ferrari.getStatus();
  stepperStatus_s sS = spool.getStatus();
  stepperStatus_s pS = puller.getStatus();

  // TODO:
  //soft.store(); //must be called async or esp might crash on watchdog trigger
  //Serial.println(fS.error);
  //Serial.println(fS.rpm);

  // Assemble report message
  StaticJsonDocument<512> doc;
  String json;
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
  doc["w"] = 0; // TODO sS.rotations;
  doc["l"] = 0; // TODO fS.rotations * hard.motors.ferrari.mm_per_rotation / 1000;
  if (!fS.error.isEmpty())
    doc["e"] = fS.error;
  else if (!sS.error.isEmpty())
    doc["e"] = sS.error;
  else if (!pS.error.isEmpty())
    doc["e"] = pS.error;

  // Serialise and check message
  if (serializeJson(doc, json) == 0)
  {
    Serial.println("[Winder] JSON serialization failed!"); // TODO: Debug/Logging
    // TODO: return if json failed?
  }

  // Send message
  server.emit("stats", json);
}

void Winder::setup(config_s conf){
  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // TODO
  printBanner();

  // Load config from main.cpp // TODO: main.cpp? conf is a function-parameter?
  hard = conf.hard;
  soft = conf.soft;

  // Initialize stepper motors
  spool.init(hard.motors.spool);
  spool.setId(1);
  spool.setDebuggingLevel(INFO);
  ferrari.init(hard.motors.ferrari);
  ferrari.setId(2);
  ferrari.setDebuggingLevel(INFO);
  puller.init(hard.motors.puller);
  puller.setId(3);
  puller.setDebuggingLevel(INFO);

  // Load config from SPIFFS and BLYNK
  if (soft.load())
  {
      power(true);
      //ferrari.add({.rps = -2, .mm = 1, ROTATE});
      //spool.calibrateStall(1);
      //puller.add({.rps = 3, .mm = 0, ADJUST});
      //spool.add({.rps = -2, .mm = 1, ROTATE});
      //spool.calibrateStall(1);
  }
  else
  {
      power(false);
  }
  DEBUG_PRINTLN(soft.asJSON()); // TODO: Debug/Logging

  // Create HTTP and WS server
  server.create(hard.server.port, hard.server.default_path, soft.wifi.mdns_name, soft.wifi.friendly_name);
  server.createSocket("/ws");

  // Connect to Wifi and create AP
  if (strlen(soft.wifi.ssid))
  {
      Serial.println("Connect to Wifi"); // TODO: Debug/Logging
      wifi.connect(soft.wifi.ssid, soft.wifi.password);
  }
  if (soft.wifi.ap_enabled || !strlen(soft.wifi.ssid))
  {
      Serial.println("Create Wifi-AP"); // TODO: Debug/Logging
      wifi.createAP(soft.wifi.ap_ssid, soft.wifi.ap_password);
      server.createCaptive(WiFi.softAPIP());
  }

  // Setup Timers
  updater.setInterval(30000);
  report.setInterval(1000);
  report.setCallback(reportStatus);
  report.start();

  // Handle incoming socket events
  // TODO: Move to server-creation, maybe even to a separate function?
  // TODO: Specify and document the list of commands
  server.on("connect", [](JsonObject data)
              { // Connect to a wifi with a given ssid and password
              wifi.connect(data["name"], data["password"]);
              return String();
              });
  server.on("scan", [](JsonObject data)
              { // Return list of available wifis
              return wifi.scan();
              });
  server.on("power", [](JsonObject data)
              { // Put machine into active mode, powering on the motors
              power(true);
              return String();
              });
  server.on("standby", [](JsonObject data)
              { // Put machine into standby, powering off the motors
              power(false);
              return String();
              });
  server.on("calibrate", [](JsonObject data)
              { // TODO: Calibtrate the start- or end-position of the ferrari
              uint16_t position = data["position"].as<uint16_t>();
              bool startPos = data["startPos"].as<bool>();
              calibrate(position, startPos);
              return String();
              });
  server.on("wind", [](JsonObject data)
              { // Start winding at a defined speed
              float mpm = data["mpm"].as<float>(); // Speed in meters per minute (based on diameter of the puller-role)
              wind(mpm);
              return String();
              });
  server.on("unwind", [](JsonObject data)
              { // Start unwinding at a defined speed
              float mpm = data["mpm"].as<float>(); // Speed in meters per minute (based on diameter of the puller-role)
              unwind(mpm);
              return String();
              });
  server.on("pull", [](JsonObject data)
              { // Start pulling at a defined speed
              float mpm = data["mpm"].as<float>(); // Speed in meters per minute (based on diameter of the puller-role)
              pull(mpm);
              return String();
              });
  server.on("change", [](JsonObject data)
              { // TODO: Comment, also very bad command-identifier
              change();
              return String();
              });
  server.on("speed", [](JsonObject data)
              { // Adjust speed while pulling/unwinding/winding
              float mpm = data["mpm"].as<float>(); // Speed in meters per minute (based on diameter of the puller-role)
              float speed = mpm * 1000 / hard.motors.puller.mm_per_rotation / 60;
              if (mode == WINDING || mode == PULLING || mode == CHANGING)
                  puller.setSpeed(speed);
              else if (mode == UNWINDING)
                  puller.setSpeed(-speed);
              return String();
              });
  server.on("modify", [](JsonObject data) 
              { // TODO: "modify" is a really unintuitive command-identifier
              soft.wifi.ap_enabled = data["ap_enabled"];
              if (data["mdns_name"] != "")
              {
                  strcpy(soft.wifi.mdns_name, data["mdns_name"]);
              }
              if (data["ap_ssid"] != "")
              {
                  strcpy(soft.wifi.ap_ssid, data["ap_ssid"]);
              }
              return String("\"stored\"");
              });
  server.on("config", [](JsonObject data)
              { // Return current configuration
              return soft.asJSON();
              });

  // Respond to Wifi connection changes
  wifi.conChange([](){
      switch (connection){
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
}

void Winder::loop(){
    TimerManager::instance().update();
}