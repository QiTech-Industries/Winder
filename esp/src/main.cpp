#include "Winder.h"

void setup()
{
  Serial.begin(115200);
  Config conf = {
      .hard = {
          .motors = {
              .puller = {

                  .max_current = 700,
                  .microsteps = 16,
                  .gear_ratio = 1,
                  .steps_per_rotation = 200,
                  .mm_per_rotation = 108,
                  .pins = {
                      .en = 12,
                      .dir = 27,
                      .step = 25,
                      .cs = 2,
                  }

              },
              .ferrari = {

                  .max_current = 700, .microsteps = 16,
                  .gear_ratio = 1,
                  .steps_per_rotation = 200,
                  .mm_per_rotation = 8,
                  .pins = {
                      .en = 12,
                      .dir = 14,
                      .step = 17,
                      .cs = 13,
                  }

              },
              .spool = {

                  .max_current = 700, .microsteps = 16,
                  .gear_ratio = 1/13.734,
                  .steps_per_rotation = 200,
                  .mm_per_rotation = 550, // 310 if spool is not filles up
                  .pins = {
                      .en = 12,
                      .dir = 16,
                      .step = 26,
                      .cs = 5,
                  }

              },
          },
          .server = {.port = 80, "index.html", "/ws"}},
      .soft = {
          .wifi = {
              "FRITZ!Box 7412",
              "81969407061093121957",
              "Jarvis Winder",
              "",
              "Jarvis Winder",
              "winder",
              true,
          },
          .ferrari_min = 68,
          .ferrari_max = 130,
      }};
  JarvisWinder.setup(conf);
}

void loop()
{
  JarvisWinder.loop();
}