#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mocks/unittest_ArduinoIf_mocks.h"
#include "mocks/unittest_DfMiniMp3_mocks.h"

#include "Tonuino_config.h"
#include "SimpleTimer/SimpleTimer.h"
#include "../MessageHandler/MessageHandler_implementation.h"

using ::testing::_;
using ::testing::InvokeWithoutArgs;
using ::testing::NiceMock;
using ::testing::Return;

// Fixture
class MessageHandlerTest : public ::testing::Test
{
protected:
    NiceMock<Mock_serial> m_serialMock{};
    NiceMock<Mock_DfMiniMp3> m_dfMiniMp3Mock{};
    SimpleTimer m_messageTimer{};

    MessageHandler m_MessageHandler{MessageHandler(m_serialMock,
                                               m_dfMiniMp3Mock,
                                               m_messageTimer)};
};

TEST_F(MessageHandlerTest, PrintMessage_nullptr_WontPrint)
{
    EXPECT_CALL(m_serialMock, com_println(_)).Times(0);
    m_MessageHandler.printMessage(nullptr);
}

TEST_F(MessageHandlerTest, PrintMessage_emptyString_WontPrint)
{
    EXPECT_CALL(m_serialMock, com_println(_)).Times(0);
    m_MessageHandler.printMessage("");
}

TEST_F(MessageHandlerTest, PrintMessage_Valid_WillPrint)
{
    EXPECT_CALL(m_serialMock, com_println(_));
    m_MessageHandler.printMessage("Test");
}

// PLAY PROMPT ////////////////////////////////////////////////////////////
TEST_F(MessageHandlerTest, PromptMessage_Undefined_WillNotPrompt)
{
    VoicePrompt undefined;
    EXPECT_CALL(m_dfMiniMp3Mock, playMp3FolderTrack(_)).Times(0);
    m_MessageHandler.promptMessage(undefined);
}

TEST_F(MessageHandlerTest, PromptMessage_noSkipNotPlaying_Timeout)
{
    VoicePrompt prompt;
    prompt.allowSkip = false;
    prompt.promptId = MSG_HELP;

    ON_CALL(m_dfMiniMp3Mock, isPlaying()).WillByDefault(Return(false));                                                              // not playing
    ON_CALL(m_dfMiniMp3Mock, loop()).WillByDefault(InvokeWithoutArgs(&m_messageTimer, &SimpleTimer::timerTick));

    EXPECT_CALL(m_dfMiniMp3Mock, loop()).Times(WAIT_DFMINI_READY); // timeout kicks in. to wait system calls MP3's loop
    m_MessageHandler.promptMessage(prompt);
}

TEST_F(MessageHandlerTest, PromptMessage_noSkipNotFinishing_Timeout)
{
    VoicePrompt prompt;
    prompt.allowSkip = false;
    prompt.promptId = MSG_ABORTED;

    ON_CALL(m_dfMiniMp3Mock, isPlaying()).WillByDefault(Return(true));                                                             // not playing
    ON_CALL(m_dfMiniMp3Mock, loop()).WillByDefault(InvokeWithoutArgs(&m_messageTimer, &SimpleTimer::timerTick));

    EXPECT_CALL(m_dfMiniMp3Mock, loop()).Times(TIMEOUT_PROMPT_PLAYED); // timeout kicks in. to wait system calls MP3's loop
    m_MessageHandler.promptMessage(prompt);
}

TEST_F(MessageHandlerTest, PromptMessage_noSkipPlaying_onlyStartTimeout)
{
    VoicePrompt prompt;
    prompt.allowSkip = false;
    prompt.promptId = MSG_STARTUP;
    // timeout not elapsing
    EXPECT_CALL(m_dfMiniMp3Mock, isPlaying())
        .Times(3)
        .WillOnce(Return(true)) // Called by WaitForPromptToStart()
        .WillOnce(Return(true)) // All following(s) called by WaitForPromptToFinish();
        .WillRepeatedly(Return(false)); // Finishing before timeout
    EXPECT_CALL(m_dfMiniMp3Mock, loop()).Times(1); //called once before isplaying returns true
    m_MessageHandler.promptMessage(prompt);
}

TEST_F(MessageHandlerTest, PromptMessage_playStarts_willCallPrompt)
{
    VoicePrompt prompt;
    prompt.allowSkip = true;
    prompt.promptId = MSG_HELP;

    ON_CALL(m_dfMiniMp3Mock, isPlaying()).WillByDefault(Return(true));   
    EXPECT_CALL(m_dfMiniMp3Mock, playMp3FolderTrack(_));
    m_MessageHandler.promptMessage(prompt);
}

TEST_F(MessageHandlerTest, PromptMessage_callTwice_wontPlayAgain)
{
    VoicePrompt prompt;
    prompt.allowSkip = true;
    prompt.promptId = MSG_ABORTED;

    ON_CALL(m_dfMiniMp3Mock, isPlaying()).WillByDefault(Return(true));   
    EXPECT_CALL(m_dfMiniMp3Mock, playMp3FolderTrack(_)).Times(1);
    m_MessageHandler.promptMessage(prompt);
    m_MessageHandler.promptMessage(prompt);
}