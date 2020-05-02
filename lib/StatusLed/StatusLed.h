#ifndef STATUSLED_H
    #define STATUSLED_H

#ifndef ARDUINO_H
    #include "Arduino.h"
#endif

class StatusLed
{ 
    public:
        
        enum eLedState 
        {
            off = 0, // Off
            solid = 1, // Solid on full brightness
            flash_slow = 2, // flashes at a period of 2* flashSlowTime
            flash_quick = 3, // flashes at a period of 2* flashQuickTime
            dim = 4 // Solid dimmed to 12.5%
        };
        
        // Constructor
        StatusLed(uint8_t ledPin, uint16_t flashSlowTime = 500, uint16_t flashQuickTime = 100, bool active = HIGH);
        
    public:
        // Methods
        void led_service(); // ISR routine, to be hung to a 1ms timer interrupt. Controls LED.
        void set_led_behavior(eLedState ledState); // Defines mode in which LED should illuminate
        
    private:
        // Methods
        void led_off(); 
        void led_solid();
        void led_flash_slow();
        void led_flash_quick();
        void led_dim();
        
    private:
        // Variables
        void (StatusLed::*perform)(); //class method pointer
        bool ledBehaviorSet;
        bool active;
        uint8_t ledPin;
        uint16_t msCount;
        uint16_t flashSlowTime; 
        uint16_t flashQuickTime;         
};
#endif // STATUSLED_H