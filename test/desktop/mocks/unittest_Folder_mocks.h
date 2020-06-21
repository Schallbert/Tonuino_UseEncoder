#include <Arduino.h>
#include <EEPROM_interface.h>

class Mock_Eeprom : public EEPROM_interface
{
public:
    MOCK_METHOD(uint8_t, read, (uint8_t memId), (override));
    MOCK_METHOD(void, write, (uint8_t memId, uint8_t contents), (override));
};