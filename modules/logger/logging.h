#pragma once

/**
 * @brief Priority levels for logging
 */
enum loggingLevel_e{
    NONE=0, // Default, no output at all
    CRITICAL=1, // Module / System is in a critical state / failing
    ERROR=2, // An error has occured
    WARNING=3, // It is possible / likely that an error could occur
    INFO=4 // General verbose info for debugging purposes
};

/**
 * @brief Checks whether a message would be relevant enough to be logged given a current logging level
 * 
 * @param currentLevel current level for logging
 * @param messageLevel priority level of the message
 * @return true relevant, do log
 * @return false irrelevant, do not log
 */
bool isLogRelevant(loggingLevel_e currentLevel, loggingLevel_e messageLevel);

/**
 * @brief Checks whether a message would be relevant enough to be logged given a current logging level, and printf's it to stdout with the specific color if relevant
 * 
 * @param currentLevel current level for logging
 * @param messageLevel priority level of the message
 * @param message message to be printed
 * @param ... parameter for printf
 */
void logPrint(loggingLevel_e currentLevel, loggingLevel_e messageLevel, char* message, ...);