#ifndef NFCTAG_MIFAREULTRALIGHT_H
#define NFCTAG_MIFAREULTRALIGHT_H

#include "../NfcTag/NfcTag_interface/NfcTag_interface.h"
#include "../MFRC522/MFRC522_implementation.h"

class NfcTag_MifareUltralight : public NfcTag_interface
{
public:
    NfcTag_MifareUltralight(MFRC522_interface *pMfrc522) : m_pMfrc522(pMfrc522) {}

public:
    bool readTag(byte blockAddress, byte *readResult) override;
    bool writeTag(byte blockAddress, byte *dataToWrite) override;

private:
    void checkAndRectifyBlockAddress(byte &blockAddress) override;

private:
    MFRC522_interface *m_pMfrc522{nullptr};
    static const byte MIFARE_UL_BLOCK_SIZE{4};
    static const byte ULTRALIGHTSTARTPAGE{4};
    static const byte ULTRALIGHTSTOPPAGE{11};
};

#endif // NFCTAG_MIFAREULTRALIGHT_H