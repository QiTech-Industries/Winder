#ifndef GLOBAL_H
#define GLOBAL_H
// TODO: Only used in examples/ -> remove?

// only print to Serial if DEBUG_WINDER flag is defined
#ifdef DEBUG_WINDER
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x...) Serial.printf(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x...)
#endif

// Built in Library inclusions
#include <Arduino.h> // Arduino standard library
#include <Preferences.h> // Store Backup of config to Non Volatile Storage
#include <Update.h> // Update Frimware and Interface
#include <HTTPClient.h> // Connect to update server URL
#include <DNSServer.h> // Redirect every AP Request to Webinterface
#include <ESPmDNS.h> // Make Interface available at winder.local
#include <WiFi.h> // Create Acces Point and Connect to Station

// External Library inclusions
#include <ArduinoJson.h> // Encode and Decode JSON messages 
#include <timer.h> // Timer to schedule Commands
#include <timerManager.h> // Allow classes to register timers
#include <ESPAsyncWebServer.h> // Serve Website and cretae Websocket
#include <LITTLEFS.h> // Store Webinterface in Filesystem
#include <TMCStepper.h> // Interface for TMC 2130 Stepper Driver
#include <FastAccelStepper.h> // Pulse generation for Stepper

// Custom Header file with globally available structs for data storage
#include <store.h>
#endif

// DESIGN GUIDE
// Alle Werte die surch den Nutzer verändert werden können, befinden sich in Config::soft, die restlichen in Config::hard
// mDNS does not work on Android!
// Why do we store configuration in JSON and not struct directly?
// - JSON stores values and! keys, allows changing struct on update while still beeing able to read old config
// - JSON is human readable