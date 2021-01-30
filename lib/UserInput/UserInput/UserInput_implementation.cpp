#include "Tonuino_config.h"
#include "Arduino_config.h"

#include "UserInput_implementation.h"

void UserInput_ClickEncoder::userinputServiceIsr()
{
    m_rEncoder.service();
}

Message::eMessageContent UserInput_ClickEncoder::getUserRequest()
{
    Message::eMessageContent result{Message::INNONE};
    //Poll for current encoder position and button state
    UserInput_ClickEncoder::userinputRefresh();

    if (buttonState == ClickEncoder_interface::DoubleClicked)
    {
        if (UserInput_interface::userInputLocked)
        {
            UserInput_interface::userInputLocked = false;
            VoicePrompt locked{VoicePrompt::MSG_BUTTONFREE, true};
            m_rPrompt.playPrompt(locked);
        }
        else
        {
            UserInput_interface::userInputLocked = true;
            VoicePrompt unlocked{VoicePrompt::MSG_BUTTONLOCK, true};
            m_rPrompt.playPrompt(unlocked);
        }
    }

    if (UserInput_interface::userInputLocked)
    {
        result = Message::INLOCK;
    }
    else if (buttonState == ClickEncoder_interface::Clicked)
    {
        result = Message::INPLPS;
    }
    else if (encoderDiff > 0) // TODO: Check in INTEGRATION TEST if this really is a good idea. Alternative: take position.
    {
        //encoder turned right
        if (buttonState == ClickEncoder_interface::Pressed ||
            buttonState == ClickEncoder_interface::Held)
        {
            result = Message::INNEXTLP;
        }
        else
        {
            result = Message::INNEXT;
        }
    }
    else if (encoderDiff < 0)
    {
        //encoder turned left
        if (buttonState == ClickEncoder_interface::Pressed ||
            buttonState == ClickEncoder_interface::Held)
        {
            result = Message::INPREVLP;
        }
        else
        {
            result = Message::INPREV;
        }
    }
    else if (buttonState == ClickEncoder_interface::Held) // Button not turned
    {
        result = Message::INPLPSLP;
    }
    else
    {
        result = Message::INNONE;
    }

    m_rMessageHandler.printMessage(Message{result});
    return result;
}

void UserInput_ClickEncoder::userinputRefresh()
{

    //Get values from encoder
    encoderDiff = m_rEncoder.getValue(); // diff to last "getValue" call
    encoderPosition += encoderDiff;
    buttonState = m_rEncoder.getButton();
}

// USERINPUT___CLICKENCODER ---------------------------------------------------------------

// USERINPUT___3BUTTONS     ---------------------------------------------------------------
//UserInput_3Buttons::UserInput_3Buttons(){}

void UserInput_3Buttons::userinputServiceIsr()
{
    m_PlpsButton.service();
    m_NextButton.service();
    m_PrevButton.service();
}

Message::eMessageContent UserInput_3Buttons::getUserRequest()
{
    Message::eMessageContent result{Message::INNONE};
    //Get current button's states
    UserInput_3Buttons::userinputRefresh();

    if (buttonStates.plpsButton == ClickEncoder_interface::DoubleClicked)
    {
        if (UserInput_interface::userInputLocked)
        {
            UserInput_interface::userInputLocked = false;
            VoicePrompt locked{VoicePrompt::MSG_BUTTONFREE, true};
            m_rPrompt.playPrompt(locked);
        }
        else
        {
            UserInput_interface::userInputLocked = true;
            VoicePrompt unlocked{VoicePrompt::MSG_BUTTONLOCK, true};
            m_rPrompt.playPrompt(unlocked);
        }
    }

    if (UserInput_interface::userInputLocked)
    {
        result = Message::INLOCK;
    }
    else if (buttonStates.plpsButton == ClickEncoder_interface::Clicked)
    {
        result = Message::INPLPS;
    }
    else if (buttonStates.plpsButton == ClickEncoder_interface::Held ||
             buttonStates.plpsButton == ClickEncoder_interface::LongPressRepeat)
    {
        result = Message::INPLPSLP;
    }
    else if (buttonStates.nextButton == ClickEncoder_interface::Clicked)
    {
        result = Message::INNEXT;
    }
    else if (buttonStates.nextButton == ClickEncoder_interface::LongPressRepeat)
    {
        result = Message::INNEXTLP;
    }
    else if (buttonStates.prevButton == ClickEncoder_interface::Clicked)
    {
        result = Message::INPREV;
    }
    else if (buttonStates.prevButton == ClickEncoder_interface::LongPressRepeat)
    {
        result = Message::INPREVLP;
    }
    else
    {
        result = Message::INNONE;
    }

    m_rMessageHandler.printMessage(Message{result});
    return result;
}

void UserInput_3Buttons::userinputRefresh()
{
    buttonStates.plpsButton = m_PlpsButton.getButton();
    buttonStates.nextButton = m_NextButton.getButton();
    buttonStates.prevButton = m_PrevButton.getButton();
}

// USERINPUT___3BUTTONS     ---------------------------------------------------------------