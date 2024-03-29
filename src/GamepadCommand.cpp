#include <GamepadCommand.h>
#include <Esp32GamepadHostConfig.h>
#include <Gamepad.h>

GamepadCommand::GamepadCommand(Gamepad * gamepad)
{
    this->gamepad = gamepad;
    clearCommand();
}

Gamepad* GamepadCommand::getGamepad()
{
    return gamepad;
}

bool GamepadCommand::hasChanged()
{
    bool result = changed;
    changed = false;
    return result;
}

bool GamepadCommand::hasCommand()
{
    for(int i = 0 ; i < BUTTONS_NUMBER ; i++)
    {
        if(buttons[i]) return true;
    }
    for(int i = 0 ; i < AXES_NUMBER ; i++)
    {
        if(axes[i] != 0) return true;
    }
    for(int i = 0 ; i < TRIGGERS_NUMBER ; i++)
    {
        if(triggers[i] != 0) return true;
    }
    return false;
}

void GamepadCommand::clearCommand()
{
    for(int i = 0 ; i < BUTTONS_NUMBER ; i++)
    {
        buttons[i] = false;
    }
    for(int i = 0 ; i < AXES_NUMBER ; i++)
    {
        axes[i] = 0;
    }
    for(int i = 0 ; i < TRIGGERS_NUMBER ; i++)
    {
        triggers[i] = 0;
    }
    for(int i = 0 ; i < GYRO_NUMBER ; i++)
    {
        gyro[i] = 0;
    }
    for(int i = 0 ; i < ACCEL_NUMBER ; i++)
    {
        accel[i] = 0;
    }
    for(int i = 0 ; i < TOUCH_NUMBER ; i++)
    {
        touch[i] = 0;
    }
}

void GamepadCommand::hatToDpad(uint8_t hat)
{
    switch (hat) 
    {
        case 0xff:
        case 0x08:
            // joy.up = joy.down = joy.left = joy.right = 0;
            break;
        case 0:
            buttons[S_DPAD_UP] = true;
            break;
        case 1:
            buttons[S_DPAD_UP] = true;
            buttons[S_DPAD_RIGHT] = true;
            break;
        case 2:
            buttons[S_DPAD_RIGHT] = true;
            break;
        case 3:
            buttons[S_DPAD_DOWN] = true;
            buttons[S_DPAD_RIGHT] = true;
            break;
        case 4:
            buttons[S_DPAD_DOWN] = true;
            break;
        case 5:
            buttons[S_DPAD_DOWN] = true;
            buttons[S_DPAD_LEFT] = true;
            break;
        case 6:
            buttons[S_DPAD_LEFT] = true;
            break;
        case 7:
            buttons[S_DPAD_UP] = true;
            buttons[S_DPAD_LEFT] = true;
            break;
        default:
            LOG_ERROR("Error parsing hat value: 0x%02x\n", hat);
            break;
    }
}

String GamepadCommand::toString()
{
    char buffer[128]; //                                    triggers joy L  joy R       touch    gyro     accel
    //         gamepad[ ^ v < > A B X Y l r L R G D s S - + H T][rr,ll](xx,yy)(xx,yy)|bat|[xx,yy](xx,yy,zz)(xx,yy,zz)
    sprintf(buffer,"%s[%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c][%u,%u](%d,%d)(%d,%d)|%d|[%d,%d](%d,%d,%d)(%d,%d,%d)"
    //         gamepad[ ^ v < > A B X Y l r L R G D s S - + H T][rr,ll](xx,yy)(xx,yy)|bat|[xx,yy](xx,yy,zz)(xx,yy,zz)
                          ,gamepad->getName().c_str()
                          ,buttons[N_DPAD_UP]?   '^' : ' '
                          ,buttons[N_DPAD_DOWN]? 'v' : ' '
                          ,buttons[N_DPAD_LEFT]? '<': ' '
                          ,buttons[N_DPAD_RIGHT]?'>' : ' '
                          ,buttons[N_A]?'A' : ' '
                          ,buttons[N_B]?'B' : ' '
                          ,buttons[N_X]?'X' : ' '
                          ,buttons[N_Y]?'Y' : ' '
                          ,buttons[N_SHOULDER_LEFT]?    'l' : ' '
                          ,buttons[N_SHOULDER_RIGHT]?   'r' : ' '
                          ,buttons[N_TRIGGER_LEFT]?     'L' : ' '
                          ,buttons[N_TRIGGER_RIGHT]?    'R' : ' '
                          ,buttons[N_JOY_LEFT_CLICK]?   'G' : ' '
                          ,buttons[N_JOY_RIGHT_CLICK]?  'D' : ' '
                          ,buttons[N_START]?    's' : ' '
                          ,buttons[N_SELECT]?   'S' : ' '
                          ,buttons[N_MINUS]?     '-' : ' '
                          ,buttons[N_PLUS]?     '+' : ' '
                          ,buttons[N_HOME]?     'H' : ' '
                          ,buttons[N_TOUCH]?    'T' : ' '
                          ,triggers[LEFT]
                          ,triggers[RIGHT]
                          ,axes[L_HORIZONTAL]
                          ,axes[L_VERTICAL]
                          ,axes[R_HORIZONTAL]
                          ,axes[R_VERTICAL]
                          ,battery
                          ,touch[X]
                          ,touch[Y]
                          ,gyro[X]
                          ,gyro[Y]
                          ,gyro[Z]
                          ,accel[X]
                          ,accel[Y]
                          ,accel[Z]
                          );
    return String(buffer);
}
