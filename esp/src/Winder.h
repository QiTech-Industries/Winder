#pragma once
#include <store.h>

/**
 * @brief Manages the complete winding machine and the operation of all its components, including the steppers and wifi/webfrontend
 */
class Winder{
  public:
  /**
   * @brief TODO: Comment
   * 
   * TODO: rename to init() ?
   * 
   * @param conf Initial configuration
   */
  void setup(config_s conf);

  /**
   * @brief Called repeatedly, handles timers and thus states and changes
   */
  void loop(); // TODO: Rename to handle()?
};












// TODO: Refactor code to be more object-oriented

/**
 * @brief Send machine status to web ui
 */
void reportStatus();

/**
 * @brief TODO: Comment
 */
void change(); // TODO: Rename, could mean about anything

/**
 * @brief Starts winding the spool at a defined speed
 * 
 * @param mpm Windingspeed in meters per minute (based on diameter of the puller-role)
 */
void wind(float mpm);

/**
 * @brief Sets the speed of the ferrari in relation to the speed of the spool, to ensure synchronised winding
 */
void updateFerrariSpeed();

/**
 * @brief Sets either the left(min) or the right(max) calibration position of the ferrari
 * 
 * @param position position to be saved
 * @param startPos true = left(min) calibration, false = right(max) calibration
 */
void calibrate(uint16_t position, bool startPos);

/**
 * @brief Starts unwinding the spool at a defined speed
 * 
 * @param mpm Unwindingspeed in meters per minute (based on diameter of the puller-role)
 */
void unwind(float mpm);

/**
 * @brief Checks whether a mode is active and starts it if not yet active
 * 
 * @param nextMode Mode to be started
 * @return true Mode already active
 * @return false Mode was not active and is now started
 */
bool isMode(mode_e nextMode); // TODO: functions starting with "is" should not have active side effects

/**
 * @brief Activates/Deactivates the components of the machine
 * 
 * TODO: Rename?
 * 
 * @param on true = activate, false = deactivate (aka standby)
 */
void power(bool on);

/**
 * @brief Nonfunctional output of a logo on the debugging console
 */
void printBanner();