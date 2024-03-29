#ifndef XBOX_ONE_ADAPTER_H
#define XBOX_ONE_ADAPTER_H
#include <Arduino.h>
#include <GamepadAdapter.h>
#include <Gamepad.h>

#define CLASS_OF_DEVICE_XBOX_ONE            0x002508


#define XBOX_ONE_VENDOR_ID                  0x045e
#define XBOX_ONE_VENDOR_ID_1                0x1949
#define XBOX_ONE_PRODUCT_ID                 0x02e0
#define XBOX_ONE_PRODUCT_ID_1               0x0402

#define XBOX_RUMBLE_REPORT_ID               0x03

typedef enum {
    XB_BUTTON_HOME       = 0b00000001, 
    XBA_BUTTON_HOME      = 0b10000000, 
} HomeButtonState;

typedef enum {
    XB_BUTTON_B          = 0b00000001, 
    XB_BUTTON_A          = 0b00000010, 
    XB_BUTTON_Y          = 0b00000100, 
    XB_BUTTON_X          = 0b00001000,
    XB_BUTTON_SHOULDER_L = 0b00010000,
    XB_BUTTON_SHOULDER_R = 0b00100000,
    XB_BUTTON_MINUS      = 0b01000000, 
    XB_BUTTON_PLUS       = 0b10000000, 
} Buttons01State;

typedef enum {
    XB_THUMB_L           = 0b00000001, 
    XB_THUMB_R           = 0b00000010, 
} Thumbs01State;

struct XBox01Report {
    uint8_t x_msb;
    uint8_t x_lsb;
    uint8_t y_msb;
    uint8_t y_lsb;
    uint8_t rx_msb;
    uint8_t rx_lsb;
    uint8_t ry_msb;
    uint8_t ry_lsb;
    uint8_t tl_lsb;
    uint8_t tl_msb;
    uint8_t tr_lsb;
    uint8_t tr_msb;
    uint8_t hat;
    uint8_t buttons;
    uint8_t thumbs;
} __attribute__((packed));

typedef enum {
    XBA_BUTTON_B          = 0b00000001, 
    XBA_BUTTON_A          = 0b00000010, 
    XBA_BUTTON_Y          = 0b00001000, 
    XBA_BUTTON_X          = 0b00010000,
    XBA_BUTTON_SHOULDER_L = 0b01000000,
    XBA_BUTTON_SHOULDER_R = 0b10000000,
} Buttons07State1;

typedef enum {
    XBA_BUTTON_TRIGGER_L  = 0b00000001,
    XBA_BUTTON_TRIGGER_R  = 0b00000010,
    XBA_BUTTON_MINUS      = 0b00000100, 
    XBA_BUTTON_PLUS       = 0b00001000, 
    XBA_BUTTON_THUMB_L    = 0b00100000, 
    XBA_BUTTON_THUMB_R    = 0b01000000, 
    XBA_BUTTON_CIRCLE     = 0b10000000, 
} Buttons07State2;

struct XBox07Report {
    uint8_t x;
    uint8_t y;
    uint8_t rx;
    uint8_t ry;
    uint8_t hat;
    uint8_t buttons1;
    uint8_t buttons2;
    uint8_t tl;
    uint8_t tr;
} __attribute__((packed));


// Actuators for the force feedback (FF).
enum {
    FF_RIGHT = 1 << 0,
    FF_LEFT = 1 << 1,
    FF_TRIGGER_RIGHT = 1 << 2,
    FF_TRIGGER_LEFT = 1 << 3,
};


struct XBoxFFReport {
    // Force-feedback related
    uint8_t enable_actuators;    // LSB 0-3 for each actuator
    uint8_t force_left_trigger;  // HID descriptor says that it goes from 0-100
    uint8_t force_right_trigger;
    uint8_t force_left;
    uint8_t force_right;
    uint8_t duration;  // unknown unit, 255 is ~second
    uint8_t start_delay;
    uint8_t loop_count;  // how many times "duration" is repeated
} __attribute__((packed));



enum XBoxReportId {
    /* Input Reports */
    XBOX_INPUT_STANDARD = 0x01,
    XBOX_INPUT_HOME = 0x02,
    XBOX_INPUT_BATTERY = 0x04,
    XBOX_INPUT_ANDROID = 0x07,
};

enum XBoxAdapterState {
    XBOX_CONNECTING = 0,
    XBOX_SEND_BATTERY_REQUEST,
    XBOX_CONNECTED
};



class XBoxOneAdapter : public GamepadAdapter
{
    public :
        const char* getName() { return "X-Box One"; };

        bool match(uint16_t vendorId, uint16_t productId, uint32_t classOfDevice)
        {
            return (classOfDevice == CLASS_OF_DEVICE_XBOX_ONE && (vendorId == XBOX_ONE_VENDOR_ID || vendorId == XBOX_ONE_VENDOR_ID_1) && (productId == XBOX_ONE_PRODUCT_ID || productId == XBOX_ONE_PRODUCT_ID_1));
        };

        void connectionComplete(Gamepad* gamepad)
        {   // Set player led
            GamepadAdapter::connectionComplete(gamepad);
            // Request battery level
            gamepad->adapterState = XBOX_SEND_BATTERY_REQUEST;
        };


        bool parseDataPacket(Gamepad* gamepad, uint8_t * packet, uint16_t packetSize)
        {
            if(gamepad->adapterState == XBOX_SEND_BATTERY_REQUEST)
            { 
                requestBatteryReport(gamepad);
                gamepad->adapterState = XBOX_CONNECTED;
            }
            if(packetSize >= 3)
            {   // Check report type (0x30 for normal report 0x31 for extended report) => http://wiibrew.org/wiki/Wiimote#Data_Reporting
                switch(packet[1])
                {  
                    case XBOX_INPUT_STANDARD :
                        {   // XBox standard report
                            const struct XBox01Report* xbox01Report = (const struct XBox01Report*)&packet[2];
                            GamepadCommand* command = gamepad->getCommand();
                            // Preserve home state
                            bool home = command->buttons[GamepadCommand::SwitchButtons::SW_HOME];
                            command->clearCommand();
                            command->buttons[GamepadCommand::SwitchButtons::SW_HOME] = home;
                            command->buttons[GamepadCommand::SwitchButtons::SW_A] = (xbox01Report->buttons & XB_BUTTON_A);
                            command->buttons[GamepadCommand::SwitchButtons::SW_B] = (xbox01Report->buttons & XB_BUTTON_B);
                            command->buttons[GamepadCommand::SwitchButtons::SW_X] = (xbox01Report->buttons & XB_BUTTON_X);
                            command->buttons[GamepadCommand::SwitchButtons::SW_Y] = (xbox01Report->buttons & XB_BUTTON_Y);
                            command->buttons[GamepadCommand::SwitchButtons::SW_SHOULDER_LEFT]  = (xbox01Report->buttons & XB_BUTTON_SHOULDER_L);
                            command->buttons[GamepadCommand::SwitchButtons::SW_SHOULDER_RIGHT] = (xbox01Report->buttons & XB_BUTTON_SHOULDER_R);
                            command->buttons[GamepadCommand::SwitchButtons::SW_TRIGGER_LEFT]   = (xbox01Report->tl_lsb > 0);
                            command->buttons[GamepadCommand::SwitchButtons::SW_TRIGGER_RIGHT]  = (xbox01Report->tr_lsb > 0);
                            command->buttons[GamepadCommand::SwitchButtons::SW_PLUS]    = (xbox01Report->buttons & XB_BUTTON_PLUS);
                            command->buttons[GamepadCommand::SwitchButtons::SW_MINUS]   = (xbox01Report->buttons & XB_BUTTON_MINUS);
                            command->buttons[GamepadCommand::SwitchButtons::SW_JOY_LEFT_CLICK]  = (xbox01Report->thumbs & XB_THUMB_L);
                            command->buttons[GamepadCommand::SwitchButtons::SW_JOY_RIGHT_CLICK] = (xbox01Report->thumbs & XB_THUMB_R);
                            command->axes[GamepadCommand::AxesLeft::L_HORIZONTAL]   = UINT8_TO_INT(xbox01Report->x_lsb-1);
                            command->axes[GamepadCommand::AxesLeft::L_VERTICAL]     = UINT8_TO_INT(xbox01Report->y_lsb);
                            command->axes[GamepadCommand::AxesRight::R_HORIZONTAL]  = UINT8_TO_INT(xbox01Report->rx_lsb-1);
                            command->axes[GamepadCommand::AxesRight::R_VERTICAL]    = UINT8_TO_INT(xbox01Report->ry_lsb);
                            command->triggers[GamepadCommand::Triggers::LEFT]   = UINT10_TO_UINT(xbox01Report->tl_msb,xbox01Report->tl_lsb);
                            command->triggers[GamepadCommand::Triggers::RIGHT]  = UINT10_TO_UINT(xbox01Report->tr_msb,xbox01Report->tr_lsb);
                            command->hatToDpad(xbox01Report->hat-1);
                            command->setChanged();
                            return true;
                        }
                        break;
                    case XBOX_INPUT_ANDROID :
                        {   // XBox Android format report
                            const struct XBox07Report* xbox07Report = (const struct XBox07Report*)&packet[2];
                            GamepadCommand* command = gamepad->getCommand();
                            // Preserve home state
                            bool home = command->buttons[GamepadCommand::SwitchButtons::SW_HOME];
                            command->clearCommand();
                            command->buttons[GamepadCommand::SwitchButtons::SW_HOME] = home;
                            command->buttons[GamepadCommand::SwitchButtons::SW_A] = (xbox07Report->buttons1 & XBA_BUTTON_A);
                            command->buttons[GamepadCommand::SwitchButtons::SW_B] = (xbox07Report->buttons1 & XBA_BUTTON_B);
                            command->buttons[GamepadCommand::SwitchButtons::SW_X] = (xbox07Report->buttons1 & XBA_BUTTON_X);
                            command->buttons[GamepadCommand::SwitchButtons::SW_Y] = (xbox07Report->buttons1 & XBA_BUTTON_Y);
                            command->buttons[GamepadCommand::SwitchButtons::SW_SHOULDER_LEFT]  = (xbox07Report->buttons1 & XBA_BUTTON_SHOULDER_L);
                            command->buttons[GamepadCommand::SwitchButtons::SW_SHOULDER_RIGHT] = (xbox07Report->buttons1 & XBA_BUTTON_SHOULDER_R);
                            command->buttons[GamepadCommand::SwitchButtons::SW_TRIGGER_LEFT]   = (xbox07Report->buttons2 & XBA_BUTTON_TRIGGER_L);
                            command->buttons[GamepadCommand::SwitchButtons::SW_TRIGGER_RIGHT]  = (xbox07Report->buttons2 & XBA_BUTTON_TRIGGER_R);
                            command->buttons[GamepadCommand::SwitchButtons::SW_PLUS]    = (xbox07Report->buttons2 & XBA_BUTTON_PLUS);
                            command->buttons[GamepadCommand::SwitchButtons::SW_MINUS]   = (xbox07Report->buttons2 & XBA_BUTTON_MINUS);
                            command->buttons[GamepadCommand::SwitchButtons::SW_JOY_LEFT_CLICK]  = (xbox07Report->buttons2 & XBA_BUTTON_THUMB_L);
                            command->buttons[GamepadCommand::SwitchButtons::SW_JOY_RIGHT_CLICK] = (xbox07Report->buttons2 & XBA_BUTTON_THUMB_R);
                            command->buttons[GamepadCommand::SwitchButtons::SW_CAPTURE] = (xbox07Report->buttons2 & XBA_BUTTON_CIRCLE);
                            command->axes[GamepadCommand::AxesLeft::L_HORIZONTAL]   = UINT8_TO_INT(xbox07Report->x-1);
                            command->axes[GamepadCommand::AxesLeft::L_VERTICAL]     = UINT8_TO_INT(xbox07Report->y-1);
                            command->axes[GamepadCommand::AxesRight::R_HORIZONTAL]  = UINT8_TO_INT(xbox07Report->rx-1);
                            command->axes[GamepadCommand::AxesRight::R_VERTICAL]    = UINT8_TO_INT(xbox07Report->ry-1);
                            command->triggers[GamepadCommand::Triggers::LEFT]   = xbox07Report->tl;
                            command->triggers[GamepadCommand::Triggers::RIGHT]  = xbox07Report->tr;
                            command->hatToDpad(xbox07Report->hat);
                            command->setChanged();
                            return true;
                        }
                        break;
                    case XBOX_INPUT_HOME :
                        {   // Home button
                            GamepadCommand* command = gamepad->getCommand();
                            command->buttons[GamepadCommand::SwitchButtons::SW_HOME] = (packetSize == 3) ? (packet[2] & XB_BUTTON_HOME) : (packet[2] & XBA_BUTTON_HOME);
                            return true;
                        }
                        break;
                    case XBOX_INPUT_BATTERY :
                        {   // Battery
                            GamepadCommand* command = gamepad->getCommand();
                            command->battery = packet[2];
                            return true;
                        }
                        break;
                    default :
                        LOG_WARN("Switch Adapter packet : unsupported reportId 0x%02x\n",packet[0]);

                }
            }
            else
            {
                LOG_ERROR("Wrong packet size for XBox adapter : %d\n",packetSize);
                LOG_HEXDUMP(packet,packetSize);
            }
            return false;
        }

        void setRumble(Gamepad* gamepad, uint8_t left, uint8_t right)
        {
            // Range is 0-100, but use 1-100 since 0 won't turn rumble off...
            left = map(left, 0, 0xFF, 1, 100);
            right = map(right, 0, 0xFF, 1, 100);

            struct XBoxFFReport ff = {
                .enable_actuators = FF_RIGHT | FF_LEFT | FF_TRIGGER_LEFT | FF_TRIGGER_RIGHT,
                .force_left_trigger = left,
                .force_right_trigger = right,
                .force_left = left,
                .force_right = right,
                .duration = 0xFF,
                .start_delay = 0,
                .loop_count = 0xFF,
            };

            gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,OUTPUT_REPORT_HEADER,XBOX_RUMBLE_REPORT_ID,(const uint8_t*)&ff, sizeof(ff));
        }

        void requestBatteryReport(Gamepad* gamepad)
        {
            gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,INPUT_REPORT_REQUEST_HEADER,XBOX_INPUT_BATTERY);
        }

};

#endif 