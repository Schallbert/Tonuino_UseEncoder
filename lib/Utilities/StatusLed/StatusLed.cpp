#include "Arduino_interface.h"

#include "StatusLed.h"

StatusLed::StatusLed(Arduino_interface_pins &rPins,
                     uint8_t ledPinId,
                     bool pinActiveState,
                     uint16_t msFlashSlow,
                     uint16_t msFlashQuick) : m_rPinControl(rPins),
                                              m_ui8LedPinId(ledPinId),
                                              m_bPinAciveState(pinActiveState),
                                              m_ui16MsFlashSlow(msFlashSlow),
                                              m_ui16MsFlashQuick(msFlashQuick)
{
    m_rPinControl.pin_mode(m_ui8LedPinId, OUTPUT);
    this->perform = &StatusLed::led_off; //Set default behavior of function pointer: LED off
    led_off();                           //init state is off
}

void StatusLed::led_service()
{
    (this->*perform)(); // Call function that pointer points to.
    // this-> is needed to bind to current instance's member function
    ++m_ui16TickInternal;
}

void StatusLed::set_led_behavior(eLedState ledState)
{
    // Set function pointer.
    // This is to keep execution times in interrupt routine as low as possible
    switch (ledState)
    {
    case solid:
        perform = &StatusLed::led_solid;
        break;
    case flash_slow:
        perform = &StatusLed::led_flash_slow;
        break;
    case flash_quick:
        perform = &StatusLed::led_flash_quick;
        break;
    case dim:
        perform = &StatusLed::led_dim;
        break;
    default:
        perform = &StatusLed::led_off;
        break;
    }
}

void StatusLed::led_off()
{
    m_rPinControl.digital_write(m_ui8LedPinId, !m_bPinAciveState);
}

void StatusLed::led_solid()
{
    m_rPinControl.digital_write(m_ui8LedPinId, m_bPinAciveState);
}

void StatusLed::led_flash_slow()
{
    led_flash(m_ui16MsFlashSlow);
}

void StatusLed::led_flash_quick()
{
    led_flash(m_ui16MsFlashQuick);
}

void StatusLed::led_flash(uint16_t msFlashInterval)
{
    if (m_ui16TickInternal >= msFlashInterval)
    {
        m_ui16TickInternal = 0;
        m_rPinControl.digital_write(m_ui8LedPinId, !(m_rPinControl.digital_read(m_ui8LedPinId)));
    }
}

void StatusLed::led_dim()
{
    // Sets LED to 12.5% brightness @ 125 Hz
    // takes pinActiveState into account
    const uint8_t DIMTOEIGTH{0x07};
    bool dim = (m_ui16TickInternal & DIMTOEIGTH) == DIMTOEIGTH;
    dim ^= (!m_bPinAciveState); // Will invert value according to PinActiveState
    m_rPinControl.digital_write(m_ui8LedPinId, dim); 
}