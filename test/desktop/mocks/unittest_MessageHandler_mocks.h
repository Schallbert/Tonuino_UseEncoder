#ifndef MESSAGEHANDLER_MOCKS_H
#define MESSAGEHANDLER_MOCKS_H

#include <gmock/gmock.h>

#include "../MessageHandler/MessageHandler_interface.h"

class Mock_MessageHandler : public MessageHander_interface
{
public:
    MOCK_METHOD(void, printMessage, (const Message &message), (override));

    // Helper method for debugging purposes
    void printMessageIdToConsole()
    {
        ON_CALL(*this, printMessage).WillByDefault([this](const Message &message) {
            std::cout << "printMessage GroupId: 0x"
                      << std::hex << +message.getGroupId()
                      << ", Contents: 0x"
                      << std::hex << +message.getContents()
                      << std::endl;
        });
    }
};

class Mock_MessageToString : public MessageToString_interface
{
public:
    MOCK_METHOD(char *, getStringFromMessage, (const Message &message), (override));
};

MATCHER_P(identicalMessage, comp, "")
{
    return arg == comp;
}

#endif // MESSAGEHANDLER_MOCKS_H