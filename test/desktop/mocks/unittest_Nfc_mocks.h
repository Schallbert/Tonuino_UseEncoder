#ifndef UNITTEST_NFCTAG_MOCKS_H
#define UNITTEST_NFCTAG_MOCKS_H

#include "gmock/gmock.h"
#include "../Arduino_HardwareAbstraction/Arduino_types.h"
#include "../Nfc/Nfc/Nfc_interface.h"
#include "../Nfc/NfcTag/NfcTag_interface.h"
#include "../Nfc/MFRC522/MFRC522_interface.h"
#include "../Nfc/NfcControl/NfcControl.h"

bool resultArrayByteCompare(const byte *compareSrc, byte *compareTgt, uint8_t size);

// FAKES
// Fake buffer data for NFC tag read
static const byte fakeBufferData[NFCTAG_BYTES_TO_WRITE + 2]{
    (byte)(TAG_MAGIC_COOKIE >> 24),          // 0
    (byte)((TAG_MAGIC_COOKIE >> 16) & 0xFF), // 1
    (byte)((TAG_MAGIC_COOKIE >> 8) & 0xFF),  // 2
    (byte)(TAG_MAGIC_COOKIE & 0xFF),         // 3
    (byte)1,                                 // 4 FolderId
    (byte)Folder::ALBUM,                     // 5 ePlayMode
    0x13, 0x37};                             // last 2 bytes are fake "checksum ;)"

class Fake_Nfc : public Nfc_interface
{
public:
    virtual ~Fake_Nfc(){}; // MUST BE DEFINED; ELSE VTABLE INCLUDE ERRORS
    void initNfc() override;
    void nfcWakeup() override;
    void nfcSleep() override;
    Message::eMessageContent getTagPresence() override;
    // returns true, simulating successful write
    bool writeTag(byte blockAddress, byte *dataToWrite) override;
    // copies fakeBufferData to "readResult", simulating read from NFC tag
    bool readTag(byte blockAddress, byte *readResult) override;
};

class Fake_MFRC522_MifareMini1k4k : public MFRC522_interface
{
public:
    void init() override { return; }
    void softPowerDown() override { return; }
    void softPowerUp() override { return; }
    bool getTagUid() override { return false; }
    bool tagLogin(byte blockAddress) override
    {
        return false;
    };
    void tagHalt() override { return; };
    void tagLogoff() override { return; };
    eTagType getTagType() override { return MFRC522_interface::PICC_TYPE_UNKNOWN; };
    bool tagRead(byte blockAddress, byte *buffer, byte bufferSize) override;
    bool tagWrite(byte blockAddress, byte *buffer, byte bufferSize) override { return true; };
    bool setTagActive() override { return false; };
    bool isTagPresent() override { return true; };
};

class Fake_MFRC522_MifareUltralight : public MFRC522_interface
{
public:
    void init() override { return; }
    void softPowerDown() override { return; }
    void softPowerUp() override { return; }
    bool getTagUid() override { return false; }
    bool tagLogin(byte blockAddress) override
    {
        return false;
    };
    void tagHalt() override { return; };
    void tagLogoff() override { return; };
    eTagType getTagType() override { return MFRC522_interface::PICC_TYPE_UNKNOWN; };
    bool tagRead(byte blockAddress, byte *buffer, byte bufferSize) override;
    bool tagWrite(byte blockAddress, byte *buffer, byte bufferSize) override { return true; };
    bool setTagActive() override { return false; };
    bool isTagPresent() override { return true; };
};

// MOCKS
// Mocks the general NFC interface
class Mock_Nfc : public Nfc_interface
{
public:
    // Method:  output name   input   overrides functionality of interface
    // IF THE METHOD IS INPUTS VOID; JUST WRITE () NEVER WRITE (void) !!!
    MOCK_METHOD(void, initNfc, (), (override));
    MOCK_METHOD(void, nfcWakeup, (), (override));
    MOCK_METHOD(void, nfcSleep, (), (override));
    MOCK_METHOD(Message::eMessageContent, getTagPresence, (), (override));
    MOCK_METHOD(bool, writeTag, (byte blockAddress, byte *dataToWrite), (override));
    MOCK_METHOD(bool, readTag, (byte blockAddress, byte *readResult), (override));

    void DelegateToFake()
    {
        ON_CALL(*this, readTag).WillByDefault([this](byte blockAddress, byte *readResult) {
            return m_FakeRead.readTag(blockAddress, readResult);
        });
    }

private:
    Fake_Nfc m_FakeRead{};
};

// Mocks connection between Nfc top level and downstream MFCR interface
class Mock_NfcTag : public NfcTag_interface
{
public:
    MOCK_METHOD(bool, readTag, (byte blockAddress, byte *readResult), (override));
    MOCK_METHOD(bool, writeTag, (byte blockAddress, byte *dataToWrite), (override));
};

// Mocks out actual library Hardware access
class Mock_MFRC522 : public MFRC522_interface
{
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, softPowerDown, (), (override));
    MOCK_METHOD(void, softPowerUp, (), (override));
    MOCK_METHOD(void, tagHalt, (), (override));
    MOCK_METHOD(void, tagLogoff, (), (override));
    MOCK_METHOD(MFRC522_interface::eTagType, getTagType, (), (override));
    MOCK_METHOD(bool, setTagActive, (), (override));
    MOCK_METHOD(bool, getTagUid, (), (override));
    MOCK_METHOD(bool, isTagPresent, (), (override));
    MOCK_METHOD(bool, tagLogin, (byte blockAddress), (override));
    MOCK_METHOD(bool, tagRead, (byte blockAddress, byte *buffer, byte bufferSize), (override));
    MOCK_METHOD(bool, tagWrite, (byte blockAddress, byte *buffer, byte bufferSize), (override));

    void DelegateToFakeMini1k4k()
    {
        ON_CALL(*this, tagRead).WillByDefault([this](byte blockAddress, byte *buffer, byte bufferSize) {
            return m_FakeMini1k4k.tagRead(blockAddress, buffer, bufferSize);
        });
    }

    void DelegateToFakeUltralight()
    {
        ON_CALL(*this, tagRead).WillByDefault([this](byte blockAddress, byte *buffer, byte bufferSize) {
            return m_FakeUltralight.tagRead(blockAddress, buffer, bufferSize);
        });
    }

private:
    Fake_MFRC522_MifareMini1k4k m_FakeMini1k4k{};
    Fake_MFRC522_MifareUltralight m_FakeUltralight{};
};

// MATCHERS
MATCHER_P2(arrayByteCompare, target, size, "Compares array bites and throws errors for each byte that does not match.")
{
    for (int i = 0; i < size; ++i)
    {
        if (arg[i] != target[i])
        {
            EXPECT_EQ("", "Error: " + std::to_string(i) + ". element not matching:");
            EXPECT_EQ(arg[i], target[i]);
        }
    }
    return true;
}

#endif // UNITTEST_NFCTAG_MOCKS_H