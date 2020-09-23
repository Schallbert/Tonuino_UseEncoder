#include "NfcTagControl.h"

NfcTagControl::NfcTagControl(MFRC522_interface *pMfrc522)
{
    m_pMfrc522 = pMfrc522; // make internal variable point to external object
    m_pMfrc522->initReader();
    m_pBuffer = new uint8_t[MFRC522_interface::NFCTAG_MEMORY_TO_OCCUPY]();
}

NfcTagControl::~NfcTagControl()
{
    delete[] m_pBuffer;
}

bool NfcTagControl::is_card_present()
{
    return m_pMfrc522->isCardPresent();
}
bool NfcTagControl::is_new_card_present()
{
    return m_pMfrc522->isNewCardPresent();
}

bool NfcTagControl::write_folder_to_card(const Folder &sourceFolder)
{
    m_oFolder = sourceFolder; // Copy source folder to member object
    if (!m_oFolder.is_valid())
    {
        return false;
    }
    folder_to_buffer(); // Set buffer according to local folder data
    // Get card online and authenticate
    return m_pMfrc522->writeCard(blockAddressToReadWrite, m_pBuffer);
}

bool NfcTagControl::erase_card()
{
    for (int i = 0; i < MFRC522_interface::NFCTAG_MEMORY_TO_OCCUPY; ++i) // 7-15: Empty
    {
        m_pBuffer[i] = 0x00;
    }
    return m_pMfrc522->writeCard(blockAddressToReadWrite, m_pBuffer);
}

bool NfcTagControl::read_folder_from_card(Folder &targetFolder)
{
    if (m_pMfrc522->readCard(blockAddressToReadWrite, m_pBuffer))
    {
        buffer_to_folder();
        if (m_oFolder.is_initiated())
        {
            targetFolder = m_oFolder; // Copy member object to target folder
            return true;
        }
    }
    return false; //unknown or corrupted card
}

void NfcTagControl::folder_to_buffer()
{
    m_pBuffer[0] = (byte)(cui32MagicCookie >> 24);                       // 0
    m_pBuffer[1] = (byte)((cui32MagicCookie >> 16) & 0xFF);              // 1
    m_pBuffer[2] = (byte)((cui32MagicCookie >> 8) & 0xFF);               // 2
    m_pBuffer[3] = (byte)(cui32MagicCookie & 0xFF);                      // 3: magic cookie to identify our nfc tags
    m_pBuffer[4] = (byte)m_oFolder.get_folder_id();                      // 4: folder picked by the user
    m_pBuffer[5] = (byte)m_oFolder.get_play_mode();                      // 5: playback mode picked by the user
    m_pBuffer[6] = (byte)m_oFolder.get_track_count();                    // 6: track count of that m_oFolder
    for (int i = 7; i < MFRC522_interface::NFCTAG_MEMORY_TO_OCCUPY; ++i) // 7-15: Empty
    {
        m_pBuffer[i] = 0x00;
    }
}

void NfcTagControl::buffer_to_folder()
{
    // Transfer m_pBuffer from card read to nfcTag's variables
    m_ui32CardCookie = ((uint32_t)m_pBuffer[0] << 24) |
                       ((uint32_t)m_pBuffer[1] << 16) |
                       ((uint32_t)m_pBuffer[2] << 8) |
                       (uint32_t)m_pBuffer[3];
    uint8_t folderId = (uint8_t)m_pBuffer[4];
    Folder::ePlayMode playMode = (Folder::ePlayMode)m_pBuffer[5];
    uint8_t trackCount = (uint8_t)m_pBuffer[6];
    // Set m_oFolder object.
    m_oFolder = Folder(folderId, playMode, trackCount);
}

bool NfcTagControl::is_known_card()
{
    if (m_ui32CardCookie != cui32MagicCookie)
    {
        // Card has never been written with Magic Cookie:
        // This card can be read but is unknown!
        return false;
    }
    return true;
}