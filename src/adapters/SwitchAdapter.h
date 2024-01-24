#ifndef SWITCH_ADAPTER_H
#define SWITCH_ADAPTER_H
#include <Arduino.h>
#include <GamepadAdapter.h>
#include <Gamepad.h>

#define CLASS_OF_DEVICE_SWITCH              0x002508


#define SWITCH_VENDOR_ID                    0x057e
#define SWITCH_PRODUCT_ID                   0x2006
#define SWITCH_PRODUCT_ID_1                 0x2007
#define SWITCH_PRODUCT_ID_2                 0x2009
#define SWITCH_PRODUCT_ID_3                 0x2017

#define WII_DATA_REPORTING_REQUEST          0x12
#define WII_DATA_REPORTING_MODE_EXTENDED    0x31

#define WII_RUMBLE_REQUEST                  0x10

// From https://github.com/ricardoquesada/bluepad32/blob/af7f6570c4279f268e583e3750c16d2bc80ad6bc/src/components/bluepad32/uni_hid_parser_switch.c

typedef enum {
    SW_BUTTON_A          = 0b00000001, 
    SW_BUTTON_B          = 0b00000010, 
    SW_BUTTON_X          = 0b00000100,
    SW_BUTTON_Y          = 0b00001000, 
    SW_BUTTON_SHOULDER_L = 0b00010000,
    SW_BUTTON_SHOULDER_R = 0b00100000,
    SW_BUTTON_TRIGGER_L  = 0b01000000,
    SW_BUTTON_TRIGGER_R  = 0b10000000,
} Buttons3FMainState;

typedef enum {
    SW_BUTTON_MINUS      = 0b00000001, 
    SW_BUTTON_PLUS       = 0b00000010, 
    SW_BUTTON_THUMB_L    = 0b00000100,
    SW_BUTTON_THUMB_R    = 0b00001000, 
    SW_BUTTON_HOME       = 0b00010000,
    SW_BUTTON_CAPTURE    = 0b00100000,
} Buttons3FAuxState;


struct switch_subcmd_request {
    // Report related
    uint8_t transaction_type;  // type of transaction
    uint8_t report_id;         // must be 0x01 for subcommand, 0x10 for rumble only

    // Data related
    uint8_t packet_num;  // increment by 1 for each packet sent. It loops in 0x0 -
                         // 0xF range.
    uint8_t rumble_left[4];
    uint8_t rumble_right[4];
    uint8_t subcmd_id;  // Not used by rumble, request
    uint8_t data[0];    // length depends on the subcommand
} __attribute__((packed));

struct Switch3FReport {
    uint8_t buttons_main;
    uint8_t buttons_aux;
    uint8_t hat;
    uint8_t x_lsb;
    uint8_t x_msb;
    uint8_t y_lsb;
    uint8_t y_msb;
    uint8_t rx_lsb;
    uint8_t rx_msb;
    uint8_t ry_lsb;
    uint8_t ry_msb;
} __attribute__((packed));

struct switch_imu_data_s {
    int16_t accel[3];  // x, y, z
    int16_t gyro[3];   // x, y, z
} __attribute__((packed));

struct switch_buttons_s {
    uint8_t buttons_right;
    uint8_t buttons_misc;
    uint8_t buttons_left;
    uint8_t stick_left[3];
    uint8_t stick_right[3];
    uint8_t vibrator_report;
} __attribute__((packed));

struct switch_report_30_s {
    struct switch_buttons_s buttons;
    struct switch_imu_data_s imu[3];  // contains 3 samples differenciated by 5ms (?) each
} __attribute__((packed));

struct switch_report_21_s {
    uint8_t report_id;
    uint8_t timer;
    uint8_t bat_con;
    struct switch_buttons_s status;
    uint8_t ack;
    uint8_t subcmd_id;
    uint8_t data[0];
} __attribute__((packed));

/* frequency/amplitude tables for rumble */
struct switch_rumble_freq_data {
    uint16_t high;
    uint8_t low;
    uint16_t freq; /* Hz*/
};

struct switch_rumble_amp_data {
    uint8_t high;
    uint16_t low;
    uint16_t amp;
};



enum SwitchAdapterState {
    SWITCH_CONNECTING = 0,
    SWITCH_SEND_EXTENDED_REPORT_REQUEST,
    SWITCH_CONNECTED
};



class SwitchAdapter : public GamepadAdapter
{
    public :
        const char* getName() { return "Switch Joycon"; };

        bool match(uint16_t vendorId, uint16_t productId, uint32_t classOfDevice)
        {
            return (classOfDevice == CLASS_OF_DEVICE_SWITCH && vendorId == SWITCH_VENDOR_ID && (productId == SWITCH_PRODUCT_ID || productId == SWITCH_PRODUCT_ID_1 || productId == SWITCH_PRODUCT_ID_2 || productId == SWITCH_PRODUCT_ID_3));
        };

        void connectionComplete(Gamepad* gamepad)
        {   // Set player led
            GamepadAdapter::connectionComplete(gamepad);
            // Request extended report if needed
            /*if(!config.filterAccel)
            {   // We need to delay sending the request
                gamepad->adapterState = WII_SEND_EXTENDED_REPORT_REQUEST;
            }*/
        };


        bool parseDataPacket(Gamepad* gamepad, uint8_t * packet, uint16_t packetSize)
        {
            /*if(gamepad->adapterState == WII_SEND_EXTENDED_REPORT_REQUEST)
            {   // We need to send an extended report request
                uint8_t payload[2];
                payload[0] = 0x00; // Non continous report
                payload[1] = WII_DATA_REPORTING_MODE_EXTENDED;
                gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,OUTPUT_REPORT_HEADER,WII_DATA_REPORTING_REQUEST,payload,2);
                gamepad->adapterState = WII_CONNECTED;
            }*/
            if(packetSize >= 4)
            {   // Check report type (0x30 for normal report 0x31 for extended report) => http://wiibrew.org/wiki/Wiimote#Data_Reporting
                if(packet[1] == 0x3F)
                {   // Normal report
                    const struct Switch3FReport* switch3fReport = (const struct Switch3FReport*)&packet[2];
                    GamepadCommand* command = gamepad->getCommand();
                    command->clearCommand();
                    command->buttons[GamepadCommand::SwitchButtons::SW_A] = (switch3fReport->buttons_main & SW_BUTTON_A);
                    command->buttons[GamepadCommand::SwitchButtons::SW_B] = (switch3fReport->buttons_main & SW_BUTTON_B);
                    command->buttons[GamepadCommand::SwitchButtons::SW_X] = (switch3fReport->buttons_main & SW_BUTTON_X);
                    command->buttons[GamepadCommand::SwitchButtons::SW_Y] = (switch3fReport->buttons_main & SW_BUTTON_Y);
                    command->buttons[GamepadCommand::SwitchButtons::SW_SHOULDER_LEFT]  = (switch3fReport->buttons_main & SW_BUTTON_SHOULDER_L);
                    command->buttons[GamepadCommand::SwitchButtons::SW_SHOULDER_RIGHT] = (switch3fReport->buttons_main & SW_BUTTON_SHOULDER_R);
                    command->buttons[GamepadCommand::SwitchButtons::SW_TRIGGER_LEFT]   = (switch3fReport->buttons_main & SW_BUTTON_TRIGGER_L);
                    command->buttons[GamepadCommand::SwitchButtons::SW_TRIGGER_RIGHT]  = (switch3fReport->buttons_main & SW_BUTTON_TRIGGER_R);
                    command->buttons[GamepadCommand::SwitchButtons::SW_CAPTURE] = (switch3fReport->buttons_aux & SW_BUTTON_CAPTURE);
                    command->buttons[GamepadCommand::SwitchButtons::SW_HOME]    = (switch3fReport->buttons_aux & SW_BUTTON_HOME);
                    command->buttons[GamepadCommand::SwitchButtons::SW_PLUS]    = (switch3fReport->buttons_aux & SW_BUTTON_PLUS);
                    command->buttons[GamepadCommand::SwitchButtons::SW_MINUS]   = (switch3fReport->buttons_aux & SW_BUTTON_MINUS);
                    command->buttons[GamepadCommand::SwitchButtons::SW_JOY_LEFT_CLICK]  = (switch3fReport->buttons_aux & SW_BUTTON_THUMB_L);
                    command->buttons[GamepadCommand::SwitchButtons::SW_JOY_RIGHT_CLICK] = (switch3fReport->buttons_aux & SW_BUTTON_THUMB_R);
                    command->axes[GamepadCommand::AxesLeft::L_HORIZONTAL]   = UINT16_TO_INT(switch3fReport->x_msb,switch3fReport->x_lsb);
                    command->axes[GamepadCommand::AxesLeft::L_VERTICAL]     = UINT16_TO_INT(switch3fReport->y_msb,switch3fReport->y_lsb);
                    command->axes[GamepadCommand::AxesRight::R_HORIZONTAL]  = UINT16_TO_INT(switch3fReport->rx_msb,switch3fReport->rx_lsb);
                    command->axes[GamepadCommand::AxesRight::R_VERTICAL]    = UINT16_TO_INT(switch3fReport->ry_msb,switch3fReport->ry_lsb);
                    command->hatToDpad(switch3fReport->hat);
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
        { /*  // Send report to light the player led on the wiimote
            playerNumber = playerNumber % 4;
            uint8_t leds = 0b0001 << playerNumber;
            uint8_t payload[1];
            payload[0] = (uint8_t)(leds << 4);
            gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,OUTPUT_REPORT_HEADER,0x11,payload,1);*/
        }

        void setRumble(Gamepad* gamepad, uint8_t left, uint8_t right)
        {/*
            bool rumble = ((left > 0) || (right > 0));
            LOG_DEBUG("Set rumble wiimpte to %d\n",rumble);
            uint8_t payload[1];
            payload[0] = rumble ? 0x01 : 0x00;
            gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,OUTPUT_REPORT_HEADER,WII_RUMBLE_REQUEST,payload,1);*/
        }
};

#endif 