#ifndef GAMEPAD_COMMAND_H
#define GAMEPAD_COMMAND_H
#include <Arduino.h>

// https://w3c.github.io/gamepad/#dfn-standard-gamepad
// Standard w3c gamepad mapping => 17 buttons and 4 axes + 2 triggers

#define BUTTONS_NUMBER  17
#define AXES_NUMBER     4
#define TRIGGERS_NUMBER 2

class GamepadCommand {
    public :
        enum NintendoButtons { N_A = 0    , N_B     , N_X     , N_Y       , N_SHOULDER_LEFT, N_SHOULDER_RIGHT, N_TRIGGER_LEFT, N_TRIGGER_RIGHT, N_SELECT, N_START  , N_JOY_LEFT_CLICK, N_JOY_RIGHT_CLICK, N_DPAD_UP, N_DPAD_DOWN, N_DPAD_LEFT, N_DPAD_RIGHT, N_HOME };
        enum SonyButtons     { S_CROSS = 0, S_CIRCLE, S_SQUARE, S_TRIANGLE, S_SHOULDER_LEFT, S_SHOULDER_RIGHT, S_TRIGGER_LEFT, S_TRIGGER_RIGHT, S_SHARE , S_OPTIONS, S_JOY_LEFT_CLICK, S_JOY_RIGHT_CLICK, S_DPAD_UP, S_DPAD_DOWN, S_DPAD_LEFT, S_DPAD_RIGHT, S_HOME };
        enum WiiButtons { W_A = 0, W_B, W_ONE, W_TWO, W_C, W_Z, W_MINUS, W_PLUS , W_SELECT, W_START, W_JOY_LEFT_CLICK, W_JOY_RIGHT_CLICK, W_DPAD_UP, W_DPAD_DOWN, W_DPAD_LEFT, W_DPAD_RIGHT, W_HOME };
        enum AxesLeft { L_HORIZONTAL = 0, L_VERTICAL = 1 };
        enum AxesRight { R_HORIZONTAL = 2, R_VERTICAL = 3 };
        enum Triggers { LEFT = 0, RIGHT = 1 };
        GamepadCommand();
        bool hasCommand();
        void clearCommand();
        bool buttons[BUTTONS_NUMBER];
        int32_t axes[AXES_NUMBER];
        uint16_t triggers[TRIGGERS_NUMBER];
        void hatToDpad(uint8_t hat);
        String toString();
};
#endif 