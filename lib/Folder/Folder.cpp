#include "Arduino_config.h"

#include "Folder.h"

//Folders
Folder::Folder(uint8_t ui8FolderId,
               ePlayMode ePlayMode) : m_ui8FolderId(ui8FolderId),
                                      m_ePlayMode(ePlayMode) {}

// Copy Constructor
Folder::Folder(const Folder &cpySrcFolder) : m_ui8FolderId(cpySrcFolder.m_ui8FolderId),
                                             m_ePlayMode(cpySrcFolder.m_ePlayMode),
                                             m_ui8TrackCount(cpySrcFolder.m_ui8TrackCount),
                                             m_pArduinoHal(cpySrcFolder.m_pArduinoHal),
                                             m_pMessageHandler(cpySrcFolder.m_pMessageHandler)
{
    if (cpySrcFolder.m_TrackQueue[1] != 0)
    {
        deep_copy_queue(cpySrcFolder.m_TrackQueue);
        m_ui8CurrentQueueEntry = cpySrcFolder.m_ui8CurrentQueueEntry;
    }
}

// overload assignment operator
Folder &Folder::operator=(const Folder &cpySrcFolder)
{
    if (this == &cpySrcFolder)
    {
        return *this;
    }
    m_ui8FolderId = cpySrcFolder.m_ui8FolderId;
    m_ePlayMode = cpySrcFolder.m_ePlayMode;
    m_ui8TrackCount = cpySrcFolder.m_ui8TrackCount;
    m_pArduinoHal = cpySrcFolder.m_pArduinoHal;
    m_pMessageHandler = cpySrcFolder.m_pMessageHandler;
    if (cpySrcFolder.m_TrackQueue[1] != 0)
    {
        deep_copy_queue(cpySrcFolder.m_TrackQueue);
        m_ui8CurrentQueueEntry = cpySrcFolder.m_ui8CurrentQueueEntry;
    }
    return *this;
}

void Folder::deep_copy_queue(const uint8_t *pTrackQueue)
{
    // Deep copy queue for copy constructor/assignment operator
    for (uint8_t i = 1; i <= m_ui8TrackCount; ++i)
    {
        m_TrackQueue[i] = pTrackQueue[i];
    }
}

bool Folder::isValid()
{
    if (isInitiated() && m_ui8TrackCount)
    {
        if (isTrackQueueSet())
        {
            return true;
        }
        else if (isDependencySet())
        {
            setup_track_queue();
            return true;
        }
    }
    return false;
}

bool Folder::isTrackQueueSet()
{
    return (m_TrackQueue[1] != 0); // first track is track 1
}

bool Folder::isInitiated() const
{
    return (m_ui8FolderId && (m_ePlayMode != Folder::UNDEFINED));
}

bool Folder::isDependencySet()
{
    return ((m_pArduinoHal != nullptr) && (m_pMessageHandler != nullptr));
}

void Folder::setup_track_queue()
{
    Message message{Message::PLAYLIST, static_cast<uint8_t>(m_ePlayMode)};
    m_pMessageHandler->printMessage(message.getContent());
    m_ui8CurrentQueueEntry = 1;

    switch (m_ePlayMode)
    {
    case ePlayMode::RANDOM:
        shuffle_queue();
        break;
    case ePlayMode::SAVEPROGRESS:
        init_sorted_queue();
        m_ui8CurrentQueueEntry = m_pArduinoHal->getEeprom().eeprom_read(m_ui8FolderId);
        if (m_ui8CurrentQueueEntry > m_ui8TrackCount || m_ui8CurrentQueueEntry == 0)
        {
            //  m_pArduinoHal.getEeprom() has never been written, contains some unknown value
            m_ui8CurrentQueueEntry = 1; // set to first track
            m_pArduinoHal->getEeprom().eeprom_write(m_ui8FolderId, m_ui8CurrentQueueEntry);
        }
        break;
    case ePlayMode::ALBUM:
        init_sorted_queue();
        break;
    case ePlayMode::ONELARGETRACK:
        init_sorted_queue();
        break;
    default:
        break;
    }
}

uint8_t Folder::getCurrentTrack()
{
    if (isValid())
    {
        saveProgressIfRequired();
        return m_TrackQueue[m_ui8CurrentQueueEntry];
    }
    else
    {
        return 0;
    }
}

void Folder::saveProgressIfRequired()
{
    if (m_ePlayMode == ePlayMode::SAVEPROGRESS)
    {
        m_pArduinoHal->getEeprom().eeprom_write(m_ui8FolderId, m_TrackQueue[m_ui8CurrentQueueEntry]);
    }
}

void Folder::setupDependencies(Arduino_DIcontainer_interface *pArduinoHal,
                               MessageHander_interface *pMessageHandler)
{
    m_pArduinoHal = pArduinoHal;
    m_pMessageHandler = pMessageHandler;
    isValid(); // Call to setup play queue in case dependencies are correctly linked
}

bool Folder::setTrackCount(uint8_t trackCount)
{
    bool countValid{true};
    if (trackCount > MAXTRACKSPERFOLDER)
    {
        countValid = false;
        trackCount = MAXTRACKSPERFOLDER;
    }

    m_ui8TrackCount = trackCount;
    isValid(); // Call to setup play queue in case dependencies are correctly linked
    return countValid;
}

uint8_t Folder::getNextTrack()
{
    if (!isValid())
    {
        return 0; //Error: folder not initialized
    }

    if (m_ui8CurrentQueueEntry < m_ui8TrackCount)
    {
        ++m_ui8CurrentQueueEntry;
    }
    else
    {
        m_ui8CurrentQueueEntry = 1; // Reset queue pointer to first track [1]
    }
    return getCurrentTrack();
}

uint8_t Folder::getPrevTrack()
{
    if (!isValid())
    {
        return 0; //Error: folder not initialized
    }
    if (m_ui8CurrentQueueEntry > 1)
    {
        --m_ui8CurrentQueueEntry;
    }
    else
    {
        // Reset queue pointer to last track [m_ui8TrackCount]
        m_ui8CurrentQueueEntry = m_ui8TrackCount;
    }
    return getCurrentTrack();
}

void Folder::shuffle_queue()
{
    uint8_t i = 1; // start at queue[1], queue[0] is always 0!
    uint8_t j = 1;
    uint8_t rnd = 0;
    Arduino_interface_random &rRnd = m_pArduinoHal->getRandom();
    rRnd.random_generateSeed(PINANALOG_RNDMGEN);
    bool alreadyInQueue = false;
    // Fill queue with non-repeating, random contents.
    while (i <= m_ui8TrackCount)
    {
        // Calculate pseudo random number
        // Number between 1 and m_ui8TrackCount is acceptable
        while (true)
        {
            rnd = rRnd.random_generateUi8();
            if ((rnd > 0) && (rnd <= m_ui8TrackCount))
            {
                break;
            }
        }
        j = 1;
        alreadyInQueue = false;
        while (j < i)
        {
            if (m_TrackQueue[j] == rnd)
            {
                alreadyInQueue = true; // Random number already used
                break;
            }
            ++j;
        }
        if (!alreadyInQueue)
        {
            m_TrackQueue[i] = rnd;
            ++i;
        }
    }
    m_ui8CurrentQueueEntry = 1; //reset m_ui8TrackCounter
}

void Folder::init_sorted_queue()
{
    m_ui8CurrentQueueEntry = 0; // Init: No track played yet.
    while (m_ui8CurrentQueueEntry <= m_ui8TrackCount)
    {
        // go through list and init with standard tracks 1-n; m_pTrackQueue[0] = 0, [1] = 1, etc.
        // 0th track does not exist!
        ++m_ui8CurrentQueueEntry;
        m_TrackQueue[m_ui8CurrentQueueEntry] = m_ui8CurrentQueueEntry;
    }
    m_ui8CurrentQueueEntry = 1; //reset m_ui8TrackCounter
}

uint8_t Folder::getFolderId() const
{
    return m_ui8FolderId;
}

Folder::ePlayMode Folder::getPlayMode() const
{
    return m_ePlayMode;
}

uint8_t Folder::getTrackCount() const
{
    return m_ui8TrackCount;
}