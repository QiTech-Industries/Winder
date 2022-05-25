/**
 * @brief State / Operation-mode of a winder-machine
 *
 */
enum winderOperation_e {
    OPERATE_OFF,         // Motors are unpowered, no operations
    OPERATE_STANDBY,     // Motors are powered, no operations
    OPERATE_PULLING,     // Puller-motor is active
    OPERATE_WINDING,     // Winding filament on spools with adjusted pulling-strength
    OPERATE_UNWINDING,   // Unwinding filament from the spool
    OPERATE_CALIBRATING  // Interactive calibration of the ferrari end-positions // TODO: Unused due to redesign, remove?
};

/**
 * @brief Converts an operation-identifier into its text-representation
 *
 * @param winderOperation operation to be converted
 * @return "error" on error, representation otherwise
 */
const char* winderOperationToString(winderOperation_e winderOperation);
