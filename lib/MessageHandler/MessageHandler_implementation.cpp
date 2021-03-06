#include "Arduino_DIcontainer_interface.h"

#include "MessageHandler_implementation.h"
#include "../Arduino/messages.h"

void MessageHandler::printMessage(const Message::eMessageContent message)
{
#if DEBUGSERIAL
    if (isNewMessage(message))
    {
        const char *buffer = m_rMessages.getStringFromMessage(message);
        if (buffer)
        {
            m_rSerial.com_println(buffer);
        }
    }
#endif
}

bool MessageHandler::isNewMessage(const Message::eMessageContent msg)
{
    Message message{Message(msg)};
    bool status{false};
    if (m_lastMessage[message.getGroupIdInt()] != message.getContentInt())
    {
        status = true;
        m_lastMessage[message.getGroupIdInt()] = message.getContentInt();
    }
    return status;
}