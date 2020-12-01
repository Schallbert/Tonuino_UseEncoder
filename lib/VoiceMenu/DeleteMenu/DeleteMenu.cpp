#include "DeleteMenu.h"

void DeleteMenu::confirm()
{
    m_menuState.confirm();
}

void DeleteMenu::abort()
{
    m_menuState.abort();
}

void DeleteMenu::selectNext()
{
    return;
}

void DeleteMenu::selectPrev()
{
    return;
}

void DeleteMenu::setStatusLed()
{
    if(isActive())
    {
        m_pPowerManager->set_delMenu();
    }    
}

void DeleteMenu::handlePlayback()
{
    handleTagStateChanges();
    playPrompt();
    playPreview();
}

void DeleteMenu::handleTagStateChanges()
{
    // tag to delete detected
    if (m_pNfcControl->getTagPresence() == Nfc_interface::NEW_REGISTERED_TAG)
    {
        m_menuState.setTagToDeleteDetected();
    }
}

bool DeleteMenu::isActive()
{
    return m_menuState.isActive();
}

bool DeleteMenu::isComplete()
{
    return m_menuState.isComplete();
}

void DeleteMenu::playPrompt()
{
    m_prompt.promptId = m_menuState.getMenuStateMessage();
    m_prompt.allowSkip = true;
    m_pMp3Play->playPrompt(m_prompt);
}

void DeleteMenu::playPreview()
{
    if (m_menuState.isPendingConfirmDelete())
    {
        Folder preview;
        if (m_pNfcControl->readFolderFromTag(preview))
        {
            m_pMp3Play->playFolder(preview);
        }
    }
}
