#ifndef USERINPUT_H
#define USERINPUT_H

#ifndef ARDUINO_H
#include "Arduino.h"
#endif

#include "ClickEncoder.h"

class UserInput
{
    //Interface class of UserInput implementation
    /*  UserInput gets the user's requests for device control,
        be it via buttons or a click encoder, and relays the requests
        to the main program.
        This interface is set up to make the main program independent of
        the physical implementation of the user input.
    */
public:
    enum UserRequest_e
    {
        NoAction,
        PlayPause,
        NextTrack,
        PrevTrack,
        IncVolume,
        DecVolume,
        DelCard,
        Help,
        Abort,
    };

protected:
    //UserInput();
    //~UserInput();
    bool userInputLocked = false;

public:
    // methods
    virtual void set_input_pins(uint8_t, uint8_t, uint8_t) = 0; //Must have same signature as derived methods
    virtual void override_parameters() = 0; //overrides standard header-defined parameters
    virtual void init(void) = 0;
    virtual UserRequest_e get_user_request(bool) = 0;
    virtual void userinput_service_isr(void) = 0;

}; // UserInput

class UserInput_ClickEncoder : public UserInput
{
    //Interface implementation for clickEncoder
    //  NoAction = 0,
    //  PlayPause = Click
    //  NextTrack = Turn Right with card
    //  PrevTrack = Turn Left with card
    //  IncVolume = Pressed/Held Turn Right
    //  DecVolume = Pressed/Held Turn Left
    //  DelCard =   Doubleclick without card
    //  ToggleLockInput = Doubleclick with card
    //  Help = Held without card

    // Name friend classes so constructors can be called
    friend class UserInput_Factory;

public:
    UserInput_ClickEncoder();
    ~UserInput_ClickEncoder();

    // pinA, pinB, pinSwitch are the pins of the encoder that are connected to the uC.
    void set_input_pins(uint8_t pinA, uint8_t pinB, uint8_t pinSwitch);
    // encStepsPerNotch is the resolution of the encoder (datasheet).
    // switchActiveState is the uC input level the switch shall be detected as pressed.
    // doubleClickTime is the time interval [ms] in which two clicks must be detected to count as doubleclick event.
    void override_parameters();
    void init();
    void userinput_service_isr();
    UserRequest_e get_user_request(bool cardDetected);

private:
    int16_t get_encoder_diff();

private:
    //OBJECTS
    // have to use pointer as object would have to be initialized 
    // using its constructor while not all constructor's init variables are defined yet
    ClickEncoder* encoder; 
    // INPUT PINS
    uint8_t pinA = 0;
    uint8_t pinB = 0;
    uint8_t pinSwitch = 0;
    // PARAMETERS AND DEFAULT VALUES
    bool switchActiveState = false;
    uint16_t doubleClickTime = 400;
    uint8_t encStepsPerNotch = 4;
    // Business logic variables
    volatile int16_t encoderPosition;
    int16_t encoderDiff;
    uint8_t buttonState;

}; // UserInput_ClickEncoder

class UserInput_3Buttons : public UserInput
{
    //Interface implementation for 3 buttons
    //  NoAction = 0,
    //  PlayPause = plpsButton Click
    //  NextTrack = nextButton Click with card
    //  PrevTrack = prevButton Click with card
    //  IncVolume = nextButton Held
    //  DecVolume = prevButton Held
    //  DelCard =   plpsButton Doubleclick without card
    //  toggleLockInput = plpsButton Doubleclick with card
    //  Help = plpsButton Held without card

    // Name friend classes so constructors can be called
    friend class UserInput_Factory;

public:
    /*
            First call set_input_pins to get arduino pins attached to the encoder
            Second call set_parameters if you want encoder parameters deviatin from standards
            Then call init to finally instantiate the encoder.
        */
    //UserInput_3Buttons();
    //~UserInput_3Buttons();

    // pinPrev, pinePlayPause, pinNext are the pins of the buttons that are connected to the uC.
    void set_input_pins(uint8_t pinPlayPauseAbort, uint8_t pinPrev, uint8_t pinNext);
    void override_parameters();
    void init();
    // Service routine to update button status (use 1ms task)
    void userinput_service_isr(void);
    UserRequest_e get_user_request(bool cardDetected);

private:
    class DigitalButton_SupportsLongPress : public DigitalButton
    {
    public:
        //DigitalButton_SupportsLongPress(int8_t pinId, bool active, uint16_t longPressTime, uint16_t longPressRepeatInterval);
        DigitalButton_SupportsLongPress(int8_t pinId, bool active); //params to be set via set_parameters of parent class or uses default
        ~DigitalButton_SupportsLongPress();

    public:
        void service(void);
        void set_long_press_active(bool longPressActive);
        bool handle_repeat_long_pressed(void);

    private:
        void increment_ctr_long_pressed(void);

    private:
        uint16_t longPressTime;           //mSec
        uint16_t longPressRepeatInterval; //mSec
        volatile uint16_t longPressCount = 0;
        bool longPressActive = false;
        ClickEncoder::Button buttonState; //enum to hold buttonState
    };                                    //  DigitalButton_SupportsLongPress

private:
    //OBJECTS
    DigitalButton_SupportsLongPress* prevButton;
    DigitalButton_SupportsLongPress* plpsButton;
    DigitalButton_SupportsLongPress* nextButton;
    // INPUT PINS
    uint8_t pinPrev;
    uint8_t pinPlayPauseAbort;
    uint8_t pinNext;
    // PARAMETERS
    bool switchActiveState;
    uint16_t doubleClickTime;         //mSec
    uint16_t longPressTime;           //mSec
    uint16_t longPressRepeatInterval; //mSec
};                                    // UserInput_3Buttons

class UserInput_Factory
{
    /*  This is the factory class, designed to create the right UserInput
        object based on input parameter, handing a pointer to therequested object back
        to the caller. 
        */

public:
    enum UserInputType_e
    {
        Undefined = 0,
        Encoder,
        ThreeButtons
    };

    static UserInput *getInstance(UserInputType_e typeKey);

};     // UserInput_Factory
#endif // USERINPUT_H