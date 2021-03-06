#ifndef UNITTEST_POWERMANAGER_MOCKS_H
#define UNITTEST_POWERMANAGER_MOCKS_H

#include <gmock/gmock.h>
#include "../Arduino_HardwareAbstraction/Arduino_types.h"
#include "../PowerManager/PowerManager_interface.h"

class Mock_PowerManager : public PowerManager_interface
{
public:
    MOCK_METHOD(void, setPlayback, (bool isPlaying), (override));
    MOCK_METHOD(void, setDeleteMenu, (), (override));
    MOCK_METHOD(void, setLinkMenu, (), (override));
    MOCK_METHOD(void, requestShutdown, (), (override));
    MOCK_METHOD(void, requestKeepAlive, (), (override));
    MOCK_METHOD(bool, isShutdownRequested, (), (override));
    MOCK_METHOD(void, allowShutdown, (), (override));
    MOCK_METHOD(void, notify1sTimer, (), (override));
    MOCK_METHOD(void, service1msLed, (), (override));
};

#endif // UNITTEST_POWERMANAGER_MOCKS_H