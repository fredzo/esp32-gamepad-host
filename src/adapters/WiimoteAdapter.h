#ifndef WIIMOTE_ADAPTER_H
#define WIIMOTE_ADAPTER_H
#include <Arduino.h>
#include <GamepadAdapter.h>
#include <Gamepad.h>

#define CLASS_OF_DEVICE_WIIMOTE             0x002504

#define WII_DATA_REPORTING_REQUEST          0x12
#define WII_DATA_REPORTING_MODE_EXTENDED    0x31

#define WII_RUMBLE_REQUEST                  0x10

typedef enum {
    BUTTON_Z          = 0x00020000, // nunchuk
    BUTTON_C          = 0x00010000, // nunchuk
    BUTTON_PLUS       = 0x00001000,
    BUTTON_UP         = 0x00000800, // vertical orientation
    BUTTON_DOWN       = 0x00000400,
    BUTTON_RIGHT      = 0x00000200,
    BUTTON_LEFT       = 0x00000100,
    BUTTON_HOME       = 0x00000080,
    BUTTON_MINUS      = 0x00000010,
    BUTTON_A          = 0x00000008,
    BUTTON_B          = 0x00000004,
    BUTTON_ONE        = 0x00000002,
    BUTTON_TWO        = 0x00000001,
    NO_BUTTON         = 0x00000000
} ButtonState;

enum WiimoteAdapterState {
    WII_CONNECTING = 0,
    WII_SEND_EXTENDED_REPORT_REQUEST,
    WII_CONNECTED
};



class WiimoteAdapter : public GamepadAdapter
{
    public :
        const char* getName() { return "Wiimote"; };

        bool match(uint16_t vendorId, uint16_t deviceId, uint32_t classOfDevice)
        {
            return classOfDevice == CLASS_OF_DEVICE_WIIMOTE;
        };

        bool isLowLevelSecurity(Gamepad * gamepad)
        {   // Wiimote needs low level security
            return true;
        }

        void connectionComplete(Gamepad* gamepad)
        {   // Set player led
            GamepadAdapter::connectionComplete(gamepad);
            // Request extended report if needed
            if(!config.filterAccel)
            {   // We need to delay sending the request
                gamepad->adapterState = WII_SEND_EXTENDED_REPORT_REQUEST;
            }
        };


        bool parseDataPacket(Gamepad* gamepad, uint8_t * packet, uint16_t packetSize)
        {
            if(gamepad->adapterState == WII_SEND_EXTENDED_REPORT_REQUEST)
            {   // We need to send an extended report request
                uint8_t payload[2];
                payload[0] = 0x00; // Non continous report
                payload[1] = WII_DATA_REPORTING_MODE_EXTENDED;
                gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,OUTPUT_REPORT_HEADER,WII_DATA_REPORTING_REQUEST,payload,2);
                gamepad->adapterState = WII_CONNECTED;
            }
            if(packetSize >= 4)
            {   // Check report type (0x30 for normal report 0x31 for extended report) => http://wiibrew.org/wiki/Wiimote#Data_Reporting
                if(packet[1] == 0x30 || packet[1] == 0x31)
                {   // Commont part
                    ButtonState buttonState = (ButtonState)((packet[2] << 8) | packet[3]);
                    GamepadCommand* command = gamepad->getCommand();
                    command->clearCommand();
                    command->buttons[GamepadCommand::WiiButtons::W_A] = (buttonState & BUTTON_A);
                    command->buttons[GamepadCommand::WiiButtons::W_B] = (buttonState & BUTTON_B);
                    command->buttons[GamepadCommand::WiiButtons::W_C] = (buttonState & BUTTON_C);
                    command->buttons[GamepadCommand::WiiButtons::W_Z] = (buttonState & BUTTON_Z);
                    command->buttons[GamepadCommand::WiiButtons::W_ONE]   = (buttonState & BUTTON_ONE);
                    command->buttons[GamepadCommand::WiiButtons::W_TWO]   = (buttonState & BUTTON_TWO);
                    command->buttons[GamepadCommand::WiiButtons::W_MINUS] = (buttonState & BUTTON_MINUS);
                    command->buttons[GamepadCommand::WiiButtons::W_PLUS]  = (buttonState & BUTTON_PLUS);
                    command->buttons[GamepadCommand::WiiButtons::W_HOME]  = (buttonState & BUTTON_HOME);
                    command->buttons[GamepadCommand::WiiButtons::W_DPAD_LEFT]  = (buttonState & BUTTON_UP);   // Wiimote horizontal
                    command->buttons[GamepadCommand::WiiButtons::W_DPAD_RIGHT] = (buttonState & BUTTON_DOWN); // Wiimote horizontal
                    command->buttons[GamepadCommand::WiiButtons::W_DPAD_UP]    = (buttonState & BUTTON_RIGHT);// Wiimote horizontal
                    command->buttons[GamepadCommand::WiiButtons::W_DPAD_DOWN]  = (buttonState & BUTTON_LEFT); // Wiimote horizontal
                    if(packet[1] == 0x31 && packetSize >= 7)
                    {   // Extended part (accelerometer)
                        command->accel[GamepadCommand::AXES::X] = packet[4];
                        command->accel[GamepadCommand::AXES::Y] = packet[5];
                        command->accel[GamepadCommand::AXES::Z] = packet[6];
                    }
                    command->setChanged();
                    return true;
                }
            }
            else
            {
                LOG_ERROR("Wrong packet size for Wiimote : %d\n",packetSize);
                LOG_HEXDUMP(packet,packetSize);
            }
            return false;
        }

        void setPlayer(Gamepad* gamepad, uint8_t playerNumber)
        {   // Send report to light the player led on the wiimote
            playerNumber = playerNumber % 4;
            uint8_t leds = 0b0001 << playerNumber;
            uint8_t payload[1];
            payload[0] = (uint8_t)(leds << 4);
            gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,OUTPUT_REPORT_HEADER,0x11,payload,1);
        }

        void setRumble(Gamepad* gamepad, uint8_t left, uint8_t right)
        {
            bool rumble = ((left > 0) || (right > 0));
            LOG_DEBUG("Set rumble wiimpte to %d\n",rumble);
            uint8_t payload[1];
            payload[0] = rumble ? 0x01 : 0x00;
            gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,OUTPUT_REPORT_HEADER,WII_RUMBLE_REQUEST,payload,1);
        }
};

#endif 