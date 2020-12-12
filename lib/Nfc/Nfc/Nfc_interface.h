#ifndef NFC_INTERFACE_H
#define NFC_INTERFACE_H
#include "../Arduino/Arduino_types.h"
#include "NfcControl/NfcControl_interface.h"

// Wrapper class to separate dependency MFRC522 card reader from utilizing classes.
// Enabler for mocking MFRC522 behavior.
class Nfc_interface
{
public:
    virtual ~Nfc_interface(){};

public:
    // initializes the NFC reader (serial etc.)
    virtual void initNfc() = 0;
    // returns presence state of NFC tag
    virtual NfcControl_interface::eTagState getTagPresence(void) = 0;
    // Returns true on successful write of data to a sector->block of the tag
    virtual bool writeTag(byte blockAddress, byte *dataToWrite) = 0;
    // Returns true on successful read
    // of data to a sector->block of the tag
    virtual bool readTag(byte blockAddress, byte *readResult) = 0;
};
#endif // NFC_INTERFACE_H