#include "System.h"

InputManager::eCardState InputManager::getCardState_fromReader()
{
    bool cardPresent = m_nfcTagReader->is_card_present();
    m_userInput.set_card_detected(cardPresent); // TODO: needed?

    if (cardPresent)
    {
        if (!m_nfcTagReader->is_new_card_present())
        {
            return ACTIVE_KNOWN_CARD;
        }
        else // New card detected: runs once as new card is automatically set to ActiveCard
        {
            Folder dummyFolder;
            if (m_nfcTagReader->read_folder_from_card(dummyFolder))
            {
                return NEW_KNOWN_CARD;
            }
            else // New card but folder cannot be read
            {
                if (!m_nfcTagReader->is_known_card())
                {
                    return UNKNOWN_CARD_MENU;
                }
            }
        }
    }
    else // No card present
    {
        return NO_CARD;
    }
}

UserInput::UserRequest_e InputManager::getUserInput()
{
    return m_userInput.get_user_request();
}

void OutputManager::setInputStates(InputManager::eCardState cardState, UserInput::UserRequest_e userInput)
{
    // set to input values, modify if currently in menu
    m_eCardState = cardState;
    m_eUserInput = userInput;

    // lock state in menu until complete
    if (m_bDeleteMenu)
    {
        if (cardState == InputManager::NEW_KNOWN_CARD) // wait for new card
        {
            m_bDeleteReady = true;
        }
        m_eCardState = InputManager::DELETE_CARD_MENU;
    }

    if (m_bLinkMenu)
    {
        m_eCardState = InputManager::UNKNOWN_CARD_MENU; // keeps in link menu
    }

    if (cardState == InputManager::UNKNOWN_CARD_MENU)
    {
        link(); // runs card link method on UNKNOWN_CARD detected
    }
}

bool OutputManager::runDispatcher()
{
    // Check for index out of bounds
    if ((InputManager::NO_CARD > m_eCardState) ||
        (m_eCardState >= InputManager::NUMBER_OF_CARD_STATES) ||
        (UserInput::NO_ACTION > m_eUserInput) ||
        (m_eUserInput >= UserInput::NUMBER_OF_REQUESTS))
    {
        return false;
    }
    // to not clutter dispatcher
    handleErrors(); // TODO: refactor?

    // initialize 2D-array of function pointers to address state-event transitions
    // dispatch table contains function pointers
    // cardStates = ROWS, userInput = COLUMNS
    static const dispatcher dispatchTable[InputManager::NUMBER_OF_CARD_STATES]
                                         [UserInput::NUMBER_OF_REQUESTS - 1] =
                                             {
                                                 //NOAC, PL_PS, PP_LP, NEXT_, PREV_, INC_V, DEC_V, ERROR
                                                 {&none, &plPs, &help, &next, &prev, &incV, &decV}, // NO_CARD
                                                 {&none, &plPs, &delt, &next, &prev, &incV, &decV}, // ACTIVE_KNOWN_CARD,
                                                 {&none, &plPs, &none, &next, &prev, &incV, &decV}, // NEW_KNOWN_CARD,
                                                 {&none, &linC, &abrt, &linN, &linP, &none, &none}, // UNKNOWN_CARD_MENU,
                                                 {&none, &delC, &abrt, &none, &none, &none, &none}, // DELETE_CARD_MENU,
                                             };
    dispatcher dispatchExecutor = dispatchTable[m_eCardState][m_eUserInput];
    (this->*dispatchExecutor)();
    // TODO: optimize dispatcher (link, erro!)
    return true;
}

// TODO: Add Debug output?
void OutputManager::handleErrors()
{
    // TODO: refactor?
    if (m_eUserInput == UserInput::ERROR)
    {
        m_mp3->play_specific_file(MSG_ERROR_USERINPUT);
        m_mp3->dont_skip_current_track();
        m_eUserInput = UserInput::NO_ACTION;
    }
}

void OutputManager::read()
{
    if (m_nfcTagReader->read_folder_from_card(m_currentFolder))
    {
        m_currentFolder.setup_dependencies(&eeprom, init_random_generator()); // TODO: SOLVE maybe on top level?
        m_mp3->play_folder(&m_currentFolder);
    }
    else
    {
        // TODO: Add Debug Output?
        m_mp3->play_specific_file(MSG_ERROR_CARDREAD);
        m_mp3->dont_skip_current_track();
        m_eCardState = InputManager::NO_CARD;
    }
}

void OutputManager::delt()
{
    m_mp3->play_specific_file(MSG_DELETETAG);
    m_mp3->dont_skip_current_track();
    m_bDeleteMenu = true; // keep in delete menu
}

void OutputManager::delC()
{
    if (m_bDeleteReady)
    {
        // Do delete the card.
        m_mp3->play_specific_file(MSG_CONFIRMED);
        // TODO: How to call InputManager's delete method?
        m_bDeleteReady = false;
        m_bDeleteMenu = false;
        m_nfcTagReader->erase_card();
    }
    else
    {
        m_mp3->play_specific_file(MSG_DELETETAG);
    }
}

void OutputManager::abrt()
{
    m_bDeleteMenu = false;
    m_bDeleteReady = false;
    m_bLinkMenu = false;
    m_mp3->play_specific_file(MSG_ABORTEED);
}



void OutputManager::linC()
{
    linkMenu.select_confirm();
}

void OutputManager::linN()
{
    linkMenu.select_next();
}

void OutputManager::linP()
{
    linkMenu.select_prev();
}

LinkMenu::LinkMenu(Mp3PlayerControl *mp3)
{
    m_mp3 = mp3;
}

void LinkMenu::init_link()
{
    m_ui8Option = 0;
    m_ui8OptionRange = MAXFOLDERCOUNT;
    m_bLinkState = false;
    //mp3.loop();
    if (m_mp3->is_playing())
    {
        m_mp3->play_pause();
    }
    m_mp3->play_specific_file(MSG_UNKNOWNTAG);
}


bool LinkMenu::select_confirm()
{
    static uint8_t folderId = 0;
    if (!m_bLinkState)
    {
        // Confirms folder selection
        folderId = m_ui8Option;
        m_ui8Option = 0;
        m_ui8OptionRange = static_cast<uint8_t>(Folder::ePlayMode::ENUM_COUNT);
        m_bLinkState = true;
        m_mp3->play_specific_file(MSG_TAGLINKED); // Tell user to select playMode
    }
    else
    {
        // Confirms playmode selection.
        Folder::ePlayMode playMode = static_cast<Folder::ePlayMode>(m_ui8Option);
        uint8_t trackCount = m_mp3->get_trackCount_of_folder(folderId);
        // Create new folder object and copy to main's folder object
        Folder tempFolder = Folder(folderId, playMode, trackCount);
        if (tempFolder.is_initiated())
        {
            m_linkedFolder = tempFolder; // Success! copy temporary folder to new folder.
            return true;
        }
    }
    return false;
}

void LinkMenu::select_next()
{
    // conditionally increment option (rollover)
    ++m_ui8Option;
    if (m_ui8Option > m_ui8OptionRange)
    {
        m_ui8Option = 1;
    }

    play_voice_prompt();
}

void LinkMenu::select_prev()
{
     // conditionally decrement option (rollover)
    --m_ui8Option;
    if (m_ui8Option > m_ui8OptionRange)
    {
        m_ui8Option = m_ui8OptionRange;
    }

    play_voice_prompt();
}

Folder LinkMenu::get_folder()
{
    return m_linkedFolder;
}

void LinkMenu::play_voice_prompt()
{
    //mp3.loop(); // TODO: Check if needed
    if (!m_bLinkState)
    {
        // play folderId of current choice, e.g. "one".
        m_mp3->play_specific_file(static_cast<uint16_t>(m_ui8Option));
        m_mp3->dont_skip_current_track();

        // play preview of selected folder contents
        Folder previewFolder = Folder(m_ui8Option, Folder::ONELARGETRACK, 1);
        // TODO: SOLVE SOMEWHERE ELSE?!
        previewFolder.setup_dependencies(&eeprom, 0);
        m_mp3->play_folder(&previewFolder);
    }
    else
    {
        // Prompt selected playMode
        m_mp3->play_specific_file(static_cast<uint16_t>(MSG_TAGLINKED + m_ui8Option));
        m_mp3->dont_skip_current_track();
    }
}


void OutputManager::handleInputs(InputManager::eCardState cardState,
                                 UserInput::UserRequest_e userInput)
{
    switch (cardState)
    {
        mp3.loop();
    case InputManager::NO_CARD:
        if (!m_deleteCardRequested)
        {
            // Set LED&keepAlive playback
            // Allow certain mp3 commands
            handle_playPause = &(Mp3PlayerControl::play_pause);
        }
        else
        {
            // Set LED&keepAlive delete
            // Allow abort
            // Start timeout
        }
        break;
    case InputManager::ACTIVE_KNOWN_CARD:
        // Set LED&keepAlive playback
        // Allow all mp3 commands
        break;
    case InputManager::NEW_KNOWN_CARD:
        if (!m_deleteCardRequested)
        {
            // read card
            // link folder
            // play card
            // allow all mp3 commands
        }
        else
        {
            // ask for confirmation
            // allow abort
            // delete card
            //deleteCardRequested = false;
        }

        break;
    case InputManager::UNKNOWN_CARD:
        // set LED&keepAlive newCard
        // play voice menu
        // link folder to card
        break;

    default:
        // play error message
        // CARD_ERROR

        break;
    }
}

bool System::delete_link_tagToFolder()
{
    bool newCardPresent = false;
    handleKeepAliveLed.DELETE_CARD();
    UserInput::UserRequest_e userAction = UserInput::NO_ACTION;
    while (!newCardPresent) // TODO: ADD TIMEOUT!
    {
        // Wait for new card to be detected, meanwhile allow to abort
        aUserInput->set_card_detected(newCardPresent);
        userAction = aUserInput->get_user_request();
        handleMp3.DELETE_CARD(userAction);
        if (userAction == UserInput::Abort)
        {
            return; // deletion aborted
        }
        else if (userAction == UerInput::PLAY_PAUSE)
        {
            nfcTagReader.delete_card();
#if DEBUGSERIAL
            usbSerial.com_println("Card deleted");
#endif
            return;
        }
        newCardPresent = nfcTagReader.is_new_card_present();
    }
}

void HandleMp3::newKnownCardPresent(Folder &currentFolder)
{
    mp3.play_folder(currentFolder); //Folder setup and ready to play
}

void HandleMp3::newUnknownCardPresent()
{
    mp3.play_specific_file(MSG_UNKNOWNTAG);
    mp3.dont_skip_current_track();
}

void HandleMp3::DELETE_CARD(UserRequest_e userAction)
{
    switch (userAction)
    {
    case UserInput::NO_ACTION:
        break;
    case UserInput::DelCard:
        mp3.play_specific_file(MSG_DELETETAG); // prompt to have tag placed that shall be deleted.
        mp3.dont_skip_current_track();
        break;
    case UserINput::PLAY_PAUSE:
        mp3.play_specific_file(MSG_CONFIRMED); // TODO voice prompt
        mp3.dont_skip_current_track();
        break;
    }
}


void HandleKeepAliveLed::playback()
{
    static bool wasPlaying = true; // init true to make check on startup
    if (wasPlaying == isPlaying)
    {
        return; // no status change.
    }

    if (isPlaying)
    {
        aKeepAlive.set_idle_timer(false);
        aLed.set_led_behavior(StatusLed::solid);
    }
    else
    {
        aKeepAlive.set_idle_timer(true);
        aLed.set_led_behavior(StatusLed::flash_slow);
    }
    wasPlaying = isPlaying;
}

void HandleKeepAliveAndLed::newCard()
{
    aKeepAlive.set_idle_timer(false);
    aLed.set_led_behavior(StatusLed::dim);
}

void HandleKeepAliveAndLed::deleteCard()
{
    aKeepAlive.set_idle_timer(false);
    aLed.set_led_behavior(StatusLed::flash_quick);
}




// dependencies: setup_folder(), mp3 (pausing), nfctagreader (write to card), delay
void setup_card()
{
#if DEBUGSERIAL
    usbSerial.com_println("setup_card()");
#endif
    if (setup_folder(currentFolder))
    {
        // Configuring card was successful.
        if (mp3.is_playing())
        {
            mp3.play_pause();
        }
        if (nfcTagReader.write_folder_to_card(currentFolder))
        {
            mp3.play_specific_file(MSG_TAGCONFSUCCESS); //WRITE_CARD
        }
        else
        {
            mp3.play_specific_file(MSG_ERROR); //WRITE_CARD
        }
    }
    delayControl.delay_ms(WAIT_DFMINI_READY);
}
void System::handleCardState()
{
    bool cardPresent = nfcTagReader.is_card_present();
    aUserInput->set_card_detected(cardPresent);

    handleKeepAliveLed.playback() if (cardPresent)
    {
        if (!nfcTagReader.is_new_card_present())
        {
            return ACTIVE_KNOWN_CARD;
            if (deletionRequested)
            {
                delete_link_tagToFolder(); // card cannot be fully read: clear
                deletionRequested = false;
            }
            else
            {
                // Active Card present Accept user input for playback control.
                handleMp3.activeKnownCardPresent(userAction); // Autoplay and DFmini loop
            }
        }
        else // New card detected: runs once as new card is automatically set to ActiveCard
        {

            handleKeepAliveLed.newCard();
            if (nfcTagReader.read_folder_from_card(currentFolder))
            {
                return NEW_KNOWN_CARD;
                currentFolder.setup_dependencies(&eeprom, init_random_generator());
                handleMp3.newKnownCardPresent(currentFolder);
            }
            else
            {
                if (!nfcTagReader.is_known_card())
                {
                    return UNKNOWN_CARD;
                    handleMp3.newUnknownCardPresent();
                    establish_link_tagToFolder(); // unknown card: configure
                }
                else
                {
                    return CARD_ERROR;
                    handleMp3.CARD_ERROR();
                    delete_link_tagToFolder(); // card cannot be fully read: clear
                }
            }
        }
    }
    else
    {
        NO_CARD;
    }
}