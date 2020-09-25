#ifndef NFCTAG_H
#define NFCTAG_H

#include "MFRC522_interface.h"
#include "Folder.h"


// this object stores nfc tag data
class NfcTagControl
{
public:
    // Create NfcTagControl object with dependency-injected NfcReader object
    NfcTagControl(MFRC522_interface *pMfrc522);
    ~NfcTagControl();

public:
    // Returns tag state of presence to requesting entity.
    MFRC522_interface::eTagState get_tag_presence();
    // takes a reference to an existing folder and copies the card's saved data into that object
    // [cookie, folderId, playMode, trackCount]
    bool read_folder_from_card(Folder &targetFolder);
    // writes an existing source folder's data to card (by reference)
    // writes cookie to card
    bool write_folder_to_card(const Folder &sourceFolder);
    // Sets tag contents the system writes to to 0
    bool erase_card();
    // Gets notification message from card reader
    #if DEBUGSERIAL
    const char * get_nfcReader_notification() 
    { return m_pMfrc522->stringFromMFRC522Notify(checkMFRC522Notification()); };
    #endif 

private:
    // Handle Read: converts bytestream from NFC tag to folder/ cookie data
    // Converts 16Byte package
    void buffer_to_folder();
    // Prepare Write: copies folder and cookie information to buffer for card
    // Converts 16Byte package
    void folder_to_buffer();
    // Returns true if the current card is known to the system
    // if it has the "magic cookie" equal to system's
    bool is_known_card();

public:
    static const uint32_t cui32MagicCookie{0x1337b437}; // Magic Id to tag all cards
private:
    uint32_t m_ui32CardCookie{0};                    //Cookie read from card to compare against magic ID
    static const uint8_t blockAddressToReadWrite{4}; // sector 1 block 0 for Mini1k4k, page 4-7 for UltraLight
    MFRC522_interface *m_pMfrc522{nullptr};          // NfcReader object to interact with
    uint8_t *m_pBuffer{nullptr};                     // Buffer to read/write from/to tag reader
    Folder m_oFolder{};                              //Uninitialized!
};

#endif //NFCTAG_H