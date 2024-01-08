#include <GamepadCommand.h>

GamepadCommand::GamepadCommand()
{
    clearCommand();
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
}

String GamepadCommand::toString()
{
    char buffer[128];
    //              Command[ ^ v < > A B X Y l r L R s S H][rr,ll](xx,yy)(xx,yy)
    sprintf(buffer,"Command[%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c][%d,%d](%d,%d)(%d,%d)"
    //              Command[ ^ v < > A B X Y l r L R s S H][rr,ll](xx,yy)(xx,yy)
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
                          ,buttons[N_START]?    's' : ' '
                          ,buttons[N_SELECT]?   'S' : ' '
                          ,buttons[N_HOME]?     'H' : ' '
                          ,triggers[LEFT]
                          ,triggers[RIGHT]
                          ,axes[L_HORIZONTAL]
                          ,axes[L_VERTICAL]
                          ,axes[R_HORIZONTAL]
                          ,axes[R_VERTICAL]);
    return String(buffer);
}
