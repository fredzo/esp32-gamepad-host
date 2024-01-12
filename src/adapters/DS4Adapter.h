#ifndef DS4_ADAPTER_H
#define DS4_ADAPTER_H
#include <Arduino.h>
#include <GamepadAdapter.h>
#include <Gamepad.h>

#define CLASS_OF_DEVICE_DS4             0x002508

#define DS4_FEATURE_REPORT_CALIBRATION  0x02

// From https://github.com/StryderUK/BluetoothHID/blob/main/examples/DualShock4/DS4Controller.h
union PS4Buttons {
  struct {
    uint8_t dpad : 4;
    uint8_t square : 1;
    uint8_t cross : 1;
    uint8_t circle : 1;
    uint8_t triangle : 1;

    uint8_t l1 : 1;
    uint8_t r1 : 1;
    uint8_t l2 : 1;
    uint8_t r2 : 1;
    uint8_t share : 1;
    uint8_t options : 1;
    uint8_t l3 : 1;
    uint8_t r3 : 1;

    uint8_t ps : 1;
    uint8_t touchpad : 1;
    uint8_t reportCounter : 6;
  } __attribute__((packed));
  uint32_t val : 24;
} __attribute__((packed));

typedef struct {
  uint8_t leftX;
  uint8_t leftY;
  uint8_t rightX;
  uint8_t rightY;
  PS4Buttons buttons;
  uint8_t leftTrigger;
  uint8_t rightTrigger;
} __attribute__((packed)) DS4Data;

typedef struct {
    uint8_t contact;
    uint8_t x_lo;
    uint8_t x_hi : 4, y_lo : 4;
    uint8_t y_hi;
} __attribute((packed)) PS4TouchPoint;

typedef struct {
  uint8_t packetCounter;
  PS4TouchPoint finger1Data;
  PS4TouchPoint finger2Data;
} __attribute__((packed)) PS4TrackpadData;


typedef struct {
  uint8_t dummy0;   // always 0xC0?
  uint8_t reportId; // Always 0x00
  uint8_t leftX;
  uint8_t leftY;
  uint8_t rightX;
  uint8_t rightY;
  PS4Buttons buttons;
  uint8_t leftTrigger;
  uint8_t rightTrigger;
  uint16_t timestamp;
  uint8_t temperature;
  int16_t angularVelocityX;
  int16_t angularVelocityY;
  int16_t angularVelocityZ;
  int16_t accelerationX;
  int16_t accelerationY;
  int16_t accelerationZ;
  uint32_t dummy1;
  uint8_t dummy2;
  uint8_t status;
  uint16_t dummy3;
  uint8_t trackpadPacketCount;
  PS4TrackpadData trackpadData[4];
  uint16_t dummy4;
  uint32_t crc32;
} __attribute__((packed)) DS4DataExt;

// From https://github.com/ricardoquesada/bluepad32/blob/main/src/components/bluepad32/uni_hid_parser_ds4.c
typedef struct __attribute((packed)) {
    uint8_t unk0[2];
    uint8_t flags;
    uint8_t unk1[2];
    uint8_t motorRight;
    uint8_t motorLeft;
    uint8_t ledRed;
    uint8_t ledGreen;
    uint8_t ledBlue;
    uint8_t flashLed1;  // time to flash bright (255 = 2.5 seconds)
    uint8_t flashLed2;  // time to flash dark (255 = 2.5 seconds)
    uint8_t unk2[61];
    uint32_t crc32;
} DS4OutputReport;

// When sending the FF report, which "features" should be set.
enum {
    DS4_FF_FLAG_RUMBLE = 1 << 0,
    DS4_FF_FLAG_LED_COLOR = 1 << 1,
    DS4_FF_FLAG_LED_BLINK = 1 << 2,
    DS4_FF_FLAG_BLINK_COLOR_RUMBLE = DS4_FF_FLAG_RUMBLE | DS4_FF_FLAG_LED_COLOR | DS4_FF_FLAG_LED_BLINK,
};

enum DS4AdapterState {
    CONNECTING = 0,
    SEND_PLAYER_LED,
    CONNECTED
};

class DS4Adapter : public GamepadAdapter
{
    private :
        DS4DataExt mask;

    public :
        DS4Adapter() {
            mask.dummy0 = 0x00;
            mask.reportId = 0x00;
            mask.leftX = 0xFF;
            mask.leftY = 0xFF;
            mask.rightX = 0xFF;
            mask.rightY = 0xFF;
            mask.buttons.val = 0x03FFFF;
            mask.leftTrigger = 0xFF;
            mask.rightTrigger =0xFF;
            mask.timestamp = 0x0000;
            mask.temperature = 0x00;
            mask.angularVelocityX = 0;//0xFFFF;
            mask.angularVelocityY = 0;//0xFFFF;
            mask.angularVelocityZ = 0;//0xFFFF;
            mask.accelerationX = 0;//0xFFFF;
            mask.accelerationY = 0;//0xFFFF;
            mask.accelerationZ = 0;//0xFFFF;
            mask.dummy1 = 0x00000000;
            mask.dummy2 = 0x00000000;
            mask.status = 0xFF;
            mask.dummy3 = 0x0000;
            mask.trackpadPacketCount = 0x00;
            PS4TrackpadData trackpadData[4] = {0,0,0,0};
            mask.dummy4 = 0x00;
            mask.crc32 = 0x00;
            mask.trackpadData[0].finger1Data.contact = 0xFF;
            mask.trackpadData[0].finger1Data.x_lo = 0xFF;
            mask.trackpadData[0].finger1Data.x_hi = 0xF;
            mask.trackpadData[0].finger1Data.y_lo = 0xF;
            mask.trackpadData[0].finger1Data.y_hi = 0xFF;
        }

        const char* getName() { return "Dualshock 4"; };
        
        bool match(uint16_t vendorId, uint16_t deviceId, uint32_t classOfDevice)
        {
            return classOfDevice == CLASS_OF_DEVICE_DS4;
        };

        void connectionComplete(Gamepad* gamepad)
        {   // Request extended report (by sending a calibration feature report request)
            gamepad->sendReport(Gamepad::ReportType::R_CONTROL,FEATURE_REPORT_REQUEST_HEADER,DS4_FEATURE_REPORT_CALIBRATION);
            /*// We need to delay sending of player led
            gamepad->adapterState = SEND_PLAYER_LED;*/
            GamepadAdapter::connectionComplete(gamepad);
        };

        void parseButtons(GamepadCommand* command , PS4Buttons* buttons)
        {
            command->buttons[GamepadCommand::SonyButtons::S_CIRCLE]     = buttons->circle;
            command->buttons[GamepadCommand::SonyButtons::S_CROSS]      = buttons->cross;
            command->buttons[GamepadCommand::SonyButtons::S_SQUARE]     = buttons->square;
            command->buttons[GamepadCommand::SonyButtons::S_TRIANGLE]   = buttons->triangle;
            command->buttons[GamepadCommand::SonyButtons::S_SHARE]      = buttons->share;
            command->buttons[GamepadCommand::SonyButtons::S_OPTIONS]    = buttons->options;
            command->buttons[GamepadCommand::SonyButtons::S_HOME]       = buttons->ps;
            command->buttons[GamepadCommand::SonyButtons::S_SHOULDER_LEFT]  = buttons->l1;
            command->buttons[GamepadCommand::SonyButtons::S_SHOULDER_RIGHT] = buttons->r1;
            command->buttons[GamepadCommand::SonyButtons::S_TRIGGER_LEFT]   = buttons->l2;
            command->buttons[GamepadCommand::SonyButtons::S_TRIGGER_RIGHT]  = buttons->r2;
            command->buttons[GamepadCommand::SonyButtons::S_TOUCH]          = buttons->touchpad;
            command->hatToDpad(buttons->dpad);
        };

        bool parseDataPacket(Gamepad* gamepad, uint8_t * packet, uint16_t packetSize)
        {
            /*if(gamepad->adapterState == SEND_PLAYER_LED)
            {   // First data packet => we can send player led
                GamepadAdapter::connectionComplete(gamepad);
                gamepad->adapterState = CONNECTED;
            }*/
            bool changed = false;
            if(packetSize >= 2)
            {
                uint8_t reportId = packet[1];
                if(reportId == 0x01)
                {
                    DS4Data ds4Data;
                    memcpy(&ds4Data, packet+2, sizeof(DS4Data));
                    GamepadCommand* command = gamepad->getCommand();
                    command->clearCommand();
                    command->axes[GamepadCommand::AxesLeft::L_HORIZONTAL]   = UINT8_TO_INT(ds4Data.leftX);
                    command->axes[GamepadCommand::AxesLeft::L_VERTICAL]     = UINT8_TO_INT(ds4Data.leftY);
                    command->axes[GamepadCommand::AxesRight::R_HORIZONTAL]  = UINT8_TO_INT(ds4Data.rightX);
                    command->axes[GamepadCommand::AxesRight::R_VERTICAL]    = UINT8_TO_INT(ds4Data.rightY);
                    parseButtons(command,&(ds4Data.buttons));
                    command->triggers[GamepadCommand::Triggers::LEFT]   = ds4Data.leftTrigger;
                    command->triggers[GamepadCommand::Triggers::RIGHT]  = ds4Data.rightTrigger;
                    //LOG_INFO("Left / Right triggers : %u, %u\n",ds4Data.leftTrigger,ds4Data.rightTrigger);
                    command->setChanged();
                    changed = true;
                }
                else if(reportId == 0x11)
                {   // Extended report => filter to ignore non relevant data
                    changed = packetChanged((gamepad->lastPacket)+2,packet+2,(uint8_t*)(&mask),44 /*sizeof(DS4DataExt)*/);
                    if(changed)
                    {
                        DS4DataExt ds4Data;
                        memcpy(&ds4Data, packet+2, sizeof(DS4DataExt));
                        GamepadCommand* command = gamepad->getCommand();
                        command->clearCommand();
                        command->axes[GamepadCommand::AxesLeft::L_HORIZONTAL]   = UINT8_TO_INT(ds4Data.leftX);
                        command->axes[GamepadCommand::AxesLeft::L_VERTICAL]     = UINT8_TO_INT(ds4Data.leftY);
                        command->axes[GamepadCommand::AxesRight::R_HORIZONTAL]  = UINT8_TO_INT(ds4Data.rightX);
                        command->axes[GamepadCommand::AxesRight::R_VERTICAL]    = UINT8_TO_INT(ds4Data.rightY);
                        parseButtons(command,&(ds4Data.buttons));
                        command->triggers[GamepadCommand::Triggers::LEFT]   = ds4Data.leftTrigger;
                        command->triggers[GamepadCommand::Triggers::RIGHT]  = ds4Data.rightTrigger;
                        command->battery = ds4Data.status & 0x0F;
                        command->gyro[GamepadCommand::AXES::X] = ds4Data.angularVelocityX;
                        command->gyro[GamepadCommand::AXES::Y] = ds4Data.angularVelocityY;
                        command->gyro[GamepadCommand::AXES::Z] = ds4Data.angularVelocityZ;
                        command->accel[GamepadCommand::AXES::X] = ds4Data.accelerationX;
                        command->accel[GamepadCommand::AXES::Y] = ds4Data.accelerationY;
                        command->accel[GamepadCommand::AXES::Z] = ds4Data.accelerationZ;
                        // Read touchpad
                        command->touch[GamepadCommand::AXES::X] = (ds4Data.trackpadData[0].finger1Data.x_hi << 8) + ds4Data.trackpadData[0].finger1Data.x_lo;
                        command->touch[GamepadCommand::AXES::Y] = (ds4Data.trackpadData[0].finger1Data.y_hi << 4) + ds4Data.trackpadData[0].finger1Data.y_lo;
                        //LOG_INFO("Left / Right triggers : %u, %u\n",ds4Data.leftTrigger,ds4Data.rightTrigger);
                        command->setChanged();
                    }
                }
                else
                {
                    LOG_ERROR("Unknown report id %02x for packet :\n",reportId);
                    LOG_HEXDUMP(packet,packetSize);
                }
            }
            return changed;
        };

        void setRumble(Gamepad* gamepad, uint8_t left, uint8_t right)
        {
            DS4OutputReport report;
            report.flags = DS4_FF_FLAG_BLINK_COLOR_RUMBLE; // Rumble only mode not supported
            report.motorLeft = left;
            report.motorRight = right;
            report.ledRed = gamepad->color.red;
            report.ledGreen = gamepad->color.green;
            report.ledBlue = gamepad->color.blue;
            gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,OUTPUT_REPORT_HEADER,0x11,(uint8_t*)&report,sizeof(DS4OutputReport));
        };

        void setLed(Gamepad* gamepad, GamepadColor color)
        {
            DS4OutputReport report;
            report.flags = DS4_FF_FLAG_BLINK_COLOR_RUMBLE; // Color only mode not supported
            report.motorLeft = gamepad->rumbleLeft;
            report.motorRight = gamepad->rumbleRight;
            report.ledRed = color.red;
            report.ledGreen = color.green;
            report.ledBlue = color.blue;
            gamepad->sendReport(Gamepad::ReportType::R_INTERRUPT,OUTPUT_REPORT_HEADER,0x11,(uint8_t*)&report,sizeof(DS4OutputReport));
        };
};

#endif 