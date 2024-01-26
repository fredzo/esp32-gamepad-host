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


struct SwitchSubcommandRequest {
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

struct SwitchSubcommandReplyReport {
    uint8_t report_id;
    uint8_t timer;
    uint8_t bat_con;
    struct switch_buttons_s status;
    uint8_t ack;
    uint8_t subcmd_id;
    uint8_t data[0];
} __attribute__((packed));

/* frequency/amplitude tables for rumble */
struct SwitchRumbleFreqData {
    uint16_t high;
    uint8_t low;
    uint16_t freq; /* Hz*/
};

struct SwitchRumbleAmpData {
    uint8_t high;
    uint16_t low;
    uint16_t amp;
};

/*
 * These tables are from
 * https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/blob/master/rumble_data_table.md
 */
static const struct SwitchRumbleFreqData rumble_freqs[] = {
    /* high, low, freq */
    {0x0000, 0x01, 41},   {0x0000, 0x02, 42},   {0x0000, 0x03, 43},   {0x0000, 0x04, 44},   {0x0000, 0x05, 45},
    {0x0000, 0x06, 46},   {0x0000, 0x07, 47},   {0x0000, 0x08, 48},   {0x0000, 0x09, 49},   {0x0000, 0x0A, 50},
    {0x0000, 0x0B, 51},   {0x0000, 0x0C, 52},   {0x0000, 0x0D, 53},   {0x0000, 0x0E, 54},   {0x0000, 0x0F, 55},
    {0x0000, 0x10, 57},   {0x0000, 0x11, 58},   {0x0000, 0x12, 59},   {0x0000, 0x13, 60},   {0x0000, 0x14, 62},
    {0x0000, 0x15, 63},   {0x0000, 0x16, 64},   {0x0000, 0x17, 66},   {0x0000, 0x18, 67},   {0x0000, 0x19, 69},
    {0x0000, 0x1A, 70},   {0x0000, 0x1B, 72},   {0x0000, 0x1C, 73},   {0x0000, 0x1D, 75},   {0x0000, 0x1e, 77},
    {0x0000, 0x1f, 78},   {0x0000, 0x20, 80},   {0x0400, 0x21, 82},   {0x0800, 0x22, 84},   {0x0c00, 0x23, 85},
    {0x1000, 0x24, 87},   {0x1400, 0x25, 89},   {0x1800, 0x26, 91},   {0x1c00, 0x27, 93},   {0x2000, 0x28, 95},
    {0x2400, 0x29, 97},   {0x2800, 0x2a, 99},   {0x2c00, 0x2b, 102},  {0x3000, 0x2c, 104},  {0x3400, 0x2d, 106},
    {0x3800, 0x2e, 108},  {0x3c00, 0x2f, 111},  {0x4000, 0x30, 113},  {0x4400, 0x31, 116},  {0x4800, 0x32, 118},
    {0x4c00, 0x33, 121},  {0x5000, 0x34, 123},  {0x5400, 0x35, 126},  {0x5800, 0x36, 129},  {0x5c00, 0x37, 132},
    {0x6000, 0x38, 135},  {0x6400, 0x39, 137},  {0x6800, 0x3a, 141},  {0x6c00, 0x3b, 144},  {0x7000, 0x3c, 147},
    {0x7400, 0x3d, 150},  {0x7800, 0x3e, 153},  {0x7c00, 0x3f, 157},  {0x8000, 0x40, 160},  {0x8400, 0x41, 164},
    {0x8800, 0x42, 167},  {0x8c00, 0x43, 171},  {0x9000, 0x44, 174},  {0x9400, 0x45, 178},  {0x9800, 0x46, 182},
    {0x9c00, 0x47, 186},  {0xa000, 0x48, 190},  {0xa400, 0x49, 194},  {0xa800, 0x4a, 199},  {0xac00, 0x4b, 203},
    {0xb000, 0x4c, 207},  {0xb400, 0x4d, 212},  {0xb800, 0x4e, 217},  {0xbc00, 0x4f, 221},  {0xc000, 0x50, 226},
    {0xc400, 0x51, 231},  {0xc800, 0x52, 236},  {0xcc00, 0x53, 241},  {0xd000, 0x54, 247},  {0xd400, 0x55, 252},
    {0xd800, 0x56, 258},  {0xdc00, 0x57, 263},  {0xe000, 0x58, 269},  {0xe400, 0x59, 275},  {0xe800, 0x5a, 281},
    {0xec00, 0x5b, 287},  {0xf000, 0x5c, 293},  {0xf400, 0x5d, 300},  {0xf800, 0x5e, 306},  {0xfc00, 0x5f, 313},
    {0x0001, 0x60, 320},  {0x0401, 0x61, 327},  {0x0801, 0x62, 334},  {0x0c01, 0x63, 341},  {0x1001, 0x64, 349},
    {0x1401, 0x65, 357},  {0x1801, 0x66, 364},  {0x1c01, 0x67, 372},  {0x2001, 0x68, 381},  {0x2401, 0x69, 389},
    {0x2801, 0x6a, 397},  {0x2c01, 0x6b, 406},  {0x3001, 0x6c, 415},  {0x3401, 0x6d, 424},  {0x3801, 0x6e, 433},
    {0x3c01, 0x6f, 443},  {0x4001, 0x70, 453},  {0x4401, 0x71, 462},  {0x4801, 0x72, 473},  {0x4c01, 0x73, 483},
    {0x5001, 0x74, 494},  {0x5401, 0x75, 504},  {0x5801, 0x76, 515},  {0x5c01, 0x77, 527},  {0x6001, 0x78, 538},
    {0x6401, 0x79, 550},  {0x6801, 0x7a, 562},  {0x6c01, 0x7b, 574},  {0x7001, 0x7c, 587},  {0x7401, 0x7d, 600},
    {0x7801, 0x7e, 613},  {0x7c01, 0x7f, 626},  {0x8001, 0x00, 640},  {0x8401, 0x00, 654},  {0x8801, 0x00, 668},
    {0x8c01, 0x00, 683},  {0x9001, 0x00, 698},  {0x9401, 0x00, 713},  {0x9801, 0x00, 729},  {0x9c01, 0x00, 745},
    {0xa001, 0x00, 761},  {0xa401, 0x00, 778},  {0xa801, 0x00, 795},  {0xac01, 0x00, 812},  {0xb001, 0x00, 830},
    {0xb401, 0x00, 848},  {0xb801, 0x00, 867},  {0xbc01, 0x00, 886},  {0xc001, 0x00, 905},  {0xc401, 0x00, 925},
    {0xc801, 0x00, 945},  {0xcc01, 0x00, 966},  {0xd001, 0x00, 987},  {0xd401, 0x00, 1009}, {0xd801, 0x00, 1031},
    {0xdc01, 0x00, 1053}, {0xe001, 0x00, 1076}, {0xe401, 0x00, 1100}, {0xe801, 0x00, 1124}, {0xec01, 0x00, 1149},
    {0xf001, 0x00, 1174}, {0xf401, 0x00, 1199}, {0xf801, 0x00, 1226}, {0xfc01, 0x00, 1253}};
#define TOTAL_RUMBLE_FREQS (sizeof(rumble_freqs) / sizeof(rumble_freqs[0]))

static const struct SwitchRumbleAmpData rumble_amps[] = {
    /* high, low, amp */
    {0x00, 0x0040, 0},   {0x02, 0x8040, 10},  {0x04, 0x0041, 12},  {0x06, 0x8041, 14},  {0x08, 0x0042, 17},
    {0x0a, 0x8042, 20},  {0x0c, 0x0043, 24},  {0x0e, 0x8043, 28},  {0x10, 0x0044, 33},  {0x12, 0x8044, 40},
    {0x14, 0x0045, 47},  {0x16, 0x8045, 56},  {0x18, 0x0046, 67},  {0x1a, 0x8046, 80},  {0x1c, 0x0047, 95},
    {0x1e, 0x8047, 112}, {0x20, 0x0048, 117}, {0x22, 0x8048, 123}, {0x24, 0x0049, 128}, {0x26, 0x8049, 134},
    {0x28, 0x004a, 140}, {0x2a, 0x804a, 146}, {0x2c, 0x004b, 152}, {0x2e, 0x804b, 159}, {0x30, 0x004c, 166},
    {0x32, 0x804c, 173}, {0x34, 0x004d, 181}, {0x36, 0x804d, 189}, {0x38, 0x004e, 198}, {0x3a, 0x804e, 206},
    {0x3c, 0x004f, 215}, {0x3e, 0x804f, 225}, {0x40, 0x0050, 230}, {0x42, 0x8050, 235}, {0x44, 0x0051, 240},
    {0x46, 0x8051, 245}, {0x48, 0x0052, 251}, {0x4a, 0x8052, 256}, {0x4c, 0x0053, 262}, {0x4e, 0x8053, 268},
    {0x50, 0x0054, 273}, {0x52, 0x8054, 279}, {0x54, 0x0055, 286}, {0x56, 0x8055, 292}, {0x58, 0x0056, 298},
    {0x5a, 0x8056, 305}, {0x5c, 0x0057, 311}, {0x5e, 0x8057, 318}, {0x60, 0x0058, 325}, {0x62, 0x8058, 332},
    {0x64, 0x0059, 340}, {0x66, 0x8059, 347}, {0x68, 0x005a, 355}, {0x6a, 0x805a, 362}, {0x6c, 0x005b, 370},
    {0x6e, 0x805b, 378}, {0x70, 0x005c, 387}, {0x72, 0x805c, 395}, {0x74, 0x005d, 404}, {0x76, 0x805d, 413},
    {0x78, 0x005e, 422}, {0x7a, 0x805e, 431}, {0x7c, 0x005f, 440}, {0x7e, 0x805f, 450}, {0x80, 0x0060, 460},
    {0x82, 0x8060, 470}, {0x84, 0x0061, 480}, {0x86, 0x8061, 491}, {0x88, 0x0062, 501}, {0x8a, 0x8062, 512},
    {0x8c, 0x0063, 524}, {0x8e, 0x8063, 535}, {0x90, 0x0064, 547}, {0x92, 0x8064, 559}, {0x94, 0x0065, 571},
    {0x96, 0x8065, 584}, {0x98, 0x0066, 596}, {0x9a, 0x8066, 609}, {0x9c, 0x0067, 623}, {0x9e, 0x8067, 636},
    {0xa0, 0x0068, 650}, {0xa2, 0x8068, 665}, {0xa4, 0x0069, 679}, {0xa6, 0x8069, 694}, {0xa8, 0x006a, 709},
    {0xaa, 0x806a, 725}, {0xac, 0x006b, 741}, {0xae, 0x806b, 757}, {0xb0, 0x006c, 773}, {0xb2, 0x806c, 790},
    {0xb4, 0x006d, 808}, {0xb6, 0x806d, 825}, {0xb8, 0x006e, 843}, {0xba, 0x806e, 862}, {0xbc, 0x006f, 881},
    {0xbe, 0x806f, 900}, {0xc0, 0x0070, 920}, {0xc2, 0x8070, 940}, {0xc4, 0x0071, 960}, {0xc6, 0x8071, 981},
    {0xc8, 0x0072, 1003}};
#define TOTAL_RUMBLE_AMPS (sizeof(rumble_amps) / sizeof(rumble_amps[0]))


enum {
    OUTPUT_RUMBLE_AND_SUBCMD = 0x01,
    OUTPUT_RUMBLE_ONLY = 0x10,
};

enum SwitchInputReportId {
    /* Input Reports */
    SWITCH_INPUT_SUBCMD_REPLY = 0x21,
    SWITCH_INPUT_IMU_DATA = 0x30,
    SWITCH_INPUT_MCU_DATA = 0x31,
    SWITCH_INPUT_BUTTON_EVENT = 0x3F,
};

enum SwitchSubcommand {
    SUBCMD_REQ_DEV_INFO = 0x02,
    SUBCMD_SET_REPORT_MODE = 0x03,
    SUBCMD_SPI_FLASH_READ = 0x10,
    SUBCMD_SET_PLAYER_LEDS = 0x30,
    SUBCMD_ENABLE_IMU = 0x40,
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


        // Process 0x21 input report: SWITCH_INPUT_SUBCMD_REPLY
        void processInputSubcommandReply(Gamepad* gamepad, const uint8_t* report, int len) {
            // Report has this format:
            // 21 D9 80 08 10 00 18 A8 78 F2 C7 70 0C 80 30 00 00 00 00 00 00 00 00 00
            // 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            // 00
            const struct SwitchSubcommandReplyReport* r = (const struct SwitchSubcommandReplyReport*)(report+1);
            if ((r->ack & 0b10000000) == 0) {
                LOG_ERROR("Switch adapter: Error, subcommand id=0x%02x was not successful.\n", r->subcmd_id);
            }
            switch (r->subcmd_id) 
            {
                case SUBCMD_SET_PLAYER_LEDS:
                    // Nothing to do
                    LOG_DEBUG("Switch adapter: Set player led command success.");
                    break;
                case SUBCMD_ENABLE_IMU:
                    // Nothing to do
                    LOG_DEBUG("Switch adapter: Enable IMU command success.");
                    break;
                case SUBCMD_SET_REPORT_MODE:
                case SUBCMD_REQ_DEV_INFO:
                case SUBCMD_SPI_FLASH_READ:
                default:
                    LOG_ERROR("Switch adapter: Error, unsupported subcmd_id=0x%02x in report 0x%02x\n", r->subcmd_id,r->report_id);
                    break;
            }

            // Update battery
            int battery = r->bat_con >> 5;
            gamepad->getCommand()->battery = battery;
            LOG_DEBUG("Switch adapter: battery level = %d.\n", battery);
        }

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
                switch(packet[1])
                {  
                    case SWITCH_INPUT_BUTTON_EVENT :
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
                        break;
                    case SWITCH_INPUT_SUBCMD_REPLY :
                        {
                            processInputSubcommandReply(gamepad,packet,packetSize);
                            return true;
                        }
                        break;
                    case SWITCH_INPUT_IMU_DATA :
                        {   // TODO

                        }
                        break;
                    case SWITCH_INPUT_MCU_DATA :
                    default :
                        LOG_WARN("Switch Adapter packet : unsupported reportId 0x%02x\n",packet[0]);

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

            // 1 == SET_LEDS subcmd len
            uint8_t report[sizeof(struct SwitchSubcommandRequest) + 1] = {0};

            struct SwitchSubcommandRequest* req = (struct SwitchSubcommandRequest*)&report[0];
            req->subcmd_id = SUBCMD_SET_PLAYER_LEDS;
            // LSB: turn on LEDs, MSB: flash LEDs
            // Official Switch doesn't honor the flash bit.
            // 8BitDo in Switch mode: LEDs are not working
            // White-label Switch clone: works Ok with flash LEDs
            req->data[0] = leds & 0x0f;
            sendSubCommand(gamepad, OUTPUT_RUMBLE_AND_SUBCMD, req, sizeof(report));
        }

        void setRumble(Gamepad* gamepad, uint8_t left, uint8_t right)
        {
            if(left == 0 && right == 0)
            {
                switchRumbleOff(gamepad);
            }
            else
            {
                struct SwitchSubcommandRequest req = { 0 };
                encodeRumble(req.rumble_left, left << 2, left, 500);
                encodeRumble(req.rumble_right, right << 2, right, 500);

                // Rumble request don't include the last byte of "SwitchSubcommandRequest": subcmd_id
                sendSubCommand(gamepad,OUTPUT_RUMBLE_ONLY, &req, sizeof(req) - 1);
            }
        }

        void encodeRumble(uint8_t* data, uint16_t freq_low, uint16_t freq_high, uint16_t amp) 
        {
            struct SwitchRumbleFreqData freq_data_low;
            struct SwitchRumbleFreqData freq_data_high;
            struct SwitchRumbleAmpData amp_data;

            freq_data_low = findRumbleFreq(freq_low);
            freq_data_high = findRumbleFreq(freq_high);
            amp_data = findRumbleAmp(amp);

            data[0] = (freq_data_high.high >> 8) & 0xFF;
            data[1] = (freq_data_high.high & 0xFF) + amp_data.high;
            data[2] = freq_data_low.low + ((amp_data.low >> 8) & 0xFF);
            data[3] = amp_data.low & 0xFF;
        }        

        struct SwitchRumbleFreqData findRumbleFreq(uint16_t freq) {
            unsigned int i = 0;
            if (freq > rumble_freqs[0].freq) {
                for (i = 1; i < TOTAL_RUMBLE_FREQS - 1; i++) {
                    if (freq > rumble_freqs[i - 1].freq && freq <= rumble_freqs[i].freq)
                        break;
                }
            }
            return rumble_freqs[i];
        }

        struct SwitchRumbleAmpData findRumbleAmp(uint16_t amp) {
            unsigned int i = 0;
            if (amp > rumble_amps[0].amp) {
                for (i = 1; i < TOTAL_RUMBLE_AMPS - 1; i++) {
                    if (amp > rumble_amps[i - 1].amp && amp <= rumble_amps[i].amp)
                        break;
                }
            }

            return rumble_amps[i];
        }

        void switchRumbleOff(Gamepad* gamepad) 
        {
            struct SwitchSubcommandRequest req = {0};

            uint8_t rumble_default[4] = {0x00, 0x01, 0x40, 0x40};
            memcpy(req.rumble_left, rumble_default, sizeof(req.rumble_left));
            memcpy(req.rumble_right, rumble_default, sizeof(req.rumble_left));

            // Rumble request don't include the last byte of "SwitchSubcommandRequest": subcmd_id
            sendSubCommand(gamepad,OUTPUT_RUMBLE_ONLY,(struct SwitchSubcommandRequest*)&req, sizeof(req) - 1);
        }

        void sendSubCommand(Gamepad* gamepad, uint8_t reportId ,struct SwitchSubcommandRequest* r, int len) 
        {   // TODO store in gamepad
            static uint8_t packet_num = 0;
            r->packet_num = packet_num++;
            if (packet_num > 0x0f)
               packet_num = 0;
            gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,OUTPUT_REPORT_HEADER,reportId,(const uint8_t*)r, len);
        }

};

#endif 