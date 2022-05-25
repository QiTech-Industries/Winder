#include <global.h>

namespace Interrupt {
volatile int count;
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
int totalInterrupts;
std::vector<void (*)(void)> callbacks;
std::vector<int> frequences;

void IRAM_ATTR onTime() {
    portENTER_CRITICAL_ISR(&timerMux);  // lock count variable to prevent override from update
    count++;
    portEXIT_CRITICAL_ISR(&timerMux);  // Comment out enter / exit to deactivate the critical section
}

void setupInterrupt() {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTime, true);
    timerAlarmWrite(timer, 12, true);
    timerAlarmEnable(timer);
}

void update() {
    if (count == 0) return;  // No interrupts to handle

    portENTER_CRITICAL(&timerMux);  // lock count variable to prevent override from interrupt
    count--;
    portEXIT_CRITICAL(&timerMux);

    totalInterrupts++;

    // Loop through array of all frequences and call timer if due
    for (std::vector<int>::size_type i = 0; i != frequences.size(); i++) {
        if (totalInterrupts % frequences[i] == 0) {
            (*callbacks[i])();
            ;
        }
    }
}

void addCallback(void (*function)(void), int frequency) {
    callbacks.push_back(function);
    frequences.push_back(frequency);
}
};  // namespace Interrupt
