#include "VoiceMenu.h"

VoiceMenu::~VoiceMenu()
{
    if (m_pMenuInstance)
    {
        delete m_pMenuInstance;
    }
}

void VoiceMenu::setUserInput(UserInput_interface::eUserRequest input)
{
    m_userInput = input;
}

void VoiceMenu::setTagState(NfcControl_interface::eTagState input)
{
    m_tagState = input;
}

void VoiceMenu::loop()
{
    if (isActive())
    {
        dispatchInputs();
        m_pMenuInstance->setTagState(m_tagState);
        m_pMenuInstance->handlePlayback();
        checkTimerElapsed();
        checkLeaveMenu();        
    }
    else
    {
        checkEnterLinkMenu();
        checkEnterDeleteMenu();
    }
}

bool VoiceMenu::isActive()
{
    bool result{false};
    if (m_pMenuInstance != nullptr)
    {
        result = m_pMenuInstance->isActive();
    }
    return result;
}

void VoiceMenu::checkEnterLinkMenu()
{
    if (m_tagState == NfcControl_interface::NEW_UNKNOWN_TAG)
    {
        m_pMenuInstance = Menu_factory::getInstance(Menu_factory::LINK_MENU,
                                                    m_pNfcControl,
                                                    m_pMp3Play,
                                                    m_pMessageHandler,
                                                    m_pPowerManager);
        m_pMenuInstance->confirm();
        m_pMenuTimer->start(MENU_TIMEOUT_SECS);
    }
}

void VoiceMenu::checkEnterDeleteMenu()
{
    if ((m_tagState == NfcControl_interface::ACTIVE_KNOWN_TAG) &&
        (m_userInput == UserInput_interface::PP_LONGPRESS))
    {
        m_pMenuInstance = Menu_factory::getInstance(Menu_factory::DELETE_MENU,
                                                    m_pNfcControl,
                                                    m_pMp3Play,
                                                    m_pMessageHandler,
                                                    m_pPowerManager);
        m_pMenuInstance->confirm();
        m_pMenuTimer->start(MENU_TIMEOUT_SECS);
    }
}

void VoiceMenu::checkLeaveMenu()
{
    // Could be completed or aborted
    if (isComplete() || !isActive())
    {
        delete m_pMenuInstance;
        m_pMenuInstance = nullptr;
    }
}

bool VoiceMenu::isComplete()
{
    bool result{false};
    if (m_pMenuInstance != nullptr)
    {
        result = m_pMenuInstance->isComplete();
    }
    return result;
}

void VoiceMenu::dispatchInputs()
{
    typedef VoiceMenu VM;
    static const dispatcher dispatchTable[UserInput_interface::NUMBER_OF_REQUESTS] =
        {
            //NOAC,     PL_PS,     PP_LP,     NEXT_,     PREV_,     INC_V,     DEC_V,
            &VM::none, &VM::conf, &VM::abrt, &VM::next, &VM::prev, &VM::none, &VM::none};
    dispatcher dispatchExecutor = dispatchTable[m_userInput];
    (this->*dispatchExecutor)();
}

void VoiceMenu::checkTimerElapsed()
{
    if (m_pMenuTimer->isElapsed())
    {
        abrt();
    }
}

