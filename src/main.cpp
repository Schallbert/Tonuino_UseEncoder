#ifndef UNIT_TEST

// Includes -------------------------
#include <avr/sleep.h>

// project includes -------
#include <TimerOne.h>
#include <System.h>

void timer1_task_1ms();

System* pSys{nullptr};
TimerOne* pTimer1{nullptr};

void setup()
{
    pSys = new System();
    pTimer1 = new TimerOne();
    //Init Timer1 for timer tasks
    pTimer1->initialize(TIMERONE_TASK_INTERVAL_USEC);
    pTimer1->attachInterrupt(timer1_task_1ms); // only allowed for "free" functions, NO METHODS :/
    pTimer1->start();
}

// TODO:::::::
// - Refactor pseudo timer in Mp3PlayerControl DONE
// - Rethink folder creation (trackCount renewal) DONE
// - Use class forward declaration for quicker compiles
// - Option: Card stays on system or can be removed while playing? Config?
// - Unit tests for UserInput
// - general refactoring
// - solve naming differences (underscores) of HAL with namespaces
// - rename NfcTag to more fitting name like mp3Ctrl
// - Integration testing on device
// - Lib cleanup in solution (lib manager) DONE
// - Solve build warnings (clickEncoder) DONE

void loop()
{
    //LowPower.sleep(100);
    // SLEEP for 100ms to reduce power consumption?
    if (!pSys->loop()) // shutdown requested
    {
        pTimer1->detachInterrupt();
        delete pTimer1;
        delete pSys; // shutdown system
        return; // leave loop
    }
}

void timer1_task_1ms()
{
    pSys->timer1_task_1ms();
}
#endif //UNIT_TEST
