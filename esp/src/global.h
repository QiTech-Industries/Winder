#ifndef GLOBAL_H
#define GLOBAL_H

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
// Alle Funktionen, die im Setup aufgerufen werden müssen, weil sie sonst einen Fehler zurückgeben, werden init() genannt
// Structuren werden in PascalCase benannt, ihre Deklerationen in camelCase
// zum befüllen der Strukturen werden aggregate initializer verwendet, außer es handelt sich um char bei denen dies nicht funktioniert
// Alle includes werden in global.h gelistet
// defines werden aufgrund ihrer unberechenbarkeit in keiner .cpp datei verwendet
// Alle Klassen müssen ohne Parameter initialisierbar sein
// Alle Werte die surch den Nutzer verändert werden können, befinden sich in Config::soft, die restlichen in Config::hard
// Alle Member eines structs werden in snake_case geschrieben (Ausnahme: die Blynk Struktur)
// mDNS does not work on Android!
// why do we store configuration in JSON and not struct directly?
// - JSON stores values and! keys, allows changing struct on update while still beeing able to read old config
// - JSON is human readable
// *_s is used for structs *_t is used for types as suffix
// guard statements (if with return) should be prefered over if...else