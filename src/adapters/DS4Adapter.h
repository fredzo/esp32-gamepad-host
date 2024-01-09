#ifndef DS4_ADAPTER_H
#define DS4_ADAPTER_H
#include <Arduino.h>
#include <GamepadAdapter.h>
#include <Gamepad.h>

#define CLASS_OF_DEVICE_DS4        0x002508

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
  uint8_t LeftX;
  uint8_t LeftY;
  uint8_t RightX;
  uint8_t RightY;
  PS4Buttons Buttons;
  uint8_t LT;
  uint8_t RT;
} __attribute__((packed)) DS4Data;

class DS4Adapter : public GamepadAdapter
{
    public :
        const char* getName() { return "Dualshock 4"; };

        bool match(uint16_t vendorId, uint16_t deviceId, uint32_t classOfDevice)
        {
            return classOfDevice == CLASS_OF_DEVICE_DS4;
        };


        virtual void parseDataPacket(Gamepad* gamepad, uint8_t * packet, uint16_t packetSize)
        {
            if(packetSize >= 2)
            {
                uint8_t reportId = packet[1];
                if(reportId == 0x01)
                {
                    DS4Data ds4Data;
                    memcpy(&ds4Data, packet+2, 7);
                    GamepadCommand* command = gamepad->getCommand();
                    command->clearCommand();
                    command->axes[GamepadCommand::AxesLeft::L_HORIZONTAL]   = ds4Data.LeftX;
                    command->axes[GamepadCommand::AxesLeft::L_VERTICAL]     = ds4Data.LeftY;
                    command->axes[GamepadCommand::AxesRight::R_HORIZONTAL]  = ds4Data.RightX;
                    command->axes[GamepadCommand::AxesRight::R_VERTICAL]    = ds4Data.RightY;
                    command->buttons[GamepadCommand::SonyButtons::S_CIRCLE]     = ds4Data.Buttons.circle;
                    command->buttons[GamepadCommand::SonyButtons::S_CROSS]      = ds4Data.Buttons.cross;
                    command->buttons[GamepadCommand::SonyButtons::S_SQUARE]     = ds4Data.Buttons.square;
                    command->buttons[GamepadCommand::SonyButtons::S_TRIANGLE]   = ds4Data.Buttons.triangle;
                    command->buttons[GamepadCommand::SonyButtons::S_SHARE]      = ds4Data.Buttons.share;
                    command->buttons[GamepadCommand::SonyButtons::S_OPTIONS]    = ds4Data.Buttons.options;
                    command->buttons[GamepadCommand::SonyButtons::S_HOME]       = ds4Data.Buttons.ps;
                    command->buttons[GamepadCommand::SonyButtons::S_SHOULDER_LEFT]  = ds4Data.Buttons.l1;
                    command->buttons[GamepadCommand::SonyButtons::S_SHOULDER_RIGHT] = ds4Data.Buttons.r1;
                    command->buttons[GamepadCommand::SonyButtons::S_TRIGGER_LEFT]   = ds4Data.Buttons.l2;
                    command->buttons[GamepadCommand::SonyButtons::S_TRIGGER_RIGHT]  = ds4Data.Buttons.r2;
                    command->hatToDpad(ds4Data.Buttons.dpad);
                    command->triggers[GamepadCommand::Triggers::LEFT]   = ds4Data.LT;
                    command->triggers[GamepadCommand::Triggers::RIGHT]  = ds4Data.RT;
                    LOG_INFO("Left / Right triggers : %u, %u\n",ds4Data.LT,ds4Data.RT);
                }
                else
                {   // TODO handle 0x11 reports
                    LOG_ERROR("Unknown report id %02x for packet :\n",reportId);
                    LOG_HEXDUMP(packet,packetSize);
                }
            }
        }
};

#endif 