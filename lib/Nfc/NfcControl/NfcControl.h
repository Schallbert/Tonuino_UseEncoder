#ifndef NFCCONTROL_H
#define NFCCONTROL_H

#include "NfcControl_interface.h"
#include "Nfc/Nfc_interface.h"
#include "MessageHandler_interface.h"
#include "Folder.h"
#include "Tonuino_config.h"

// this object stores nfc tag data
class NfcControl : public NfcControl_interface
{
public:
    // Create NfcControl object with dependency-injected NfcReader object
    NfcControl(Nfc_interface &rNfc,
               MessageHander_interface &rMessageHandler) : m_rNfc(rNfc),
                                                           m_rMessageHandler(rMessageHandler){};
    ~NfcControl() = default;

public:
    // initializes communication with the tag reader.
    void init() override;
    // Returns tag state of presence to requesting entity.
    Message::eMessageContent getTagPresence() override;
    // takes a reference to an existing folder and copies the card's saved data into that object
    // [cookie, folderId, playMode, trackCount]
    bool readFolderFromTag(Folder &targetFolder) override;
    // writes an existing source folder's data to card (by reference)
    // writes cookie to card
    bool writeFolderToTag(const Folder &sourceFolder) override;
    // Sets tag contents the system writes to to 0
    bool eraseTag() override;

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
    Nfc_interface &m_rNfc; // NfcReader object to interact with
    MessageHander_interface &m_rMessageHandler;

    uint32_t m_ui32CardCookie{0};                    //Cookie read from card to compare against magic ID
    static const uint8_t blockAddressToReadWrite{4}; // sector 1 block 0 for Mini1k4k, page 4-7 for UltraLight
    uint8_t m_pBuffer[NFCTAG_MEMORY_TO_OCCUPY]{};    // Buffer to read/write from/to tag reader
    Folder m_oFolder{};                              //Uninitialized!
};

#endif //NFCCONTROL_H