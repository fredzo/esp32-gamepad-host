#include <GamepadAdapter.h>

const GamepadColor GamepadAdapter::PLAYER_COLORS[] = { 
    Gamepad::PURPLE,
    Gamepad::CYAN,
    Gamepad::RED,
    Gamepad::GREEN,
    Gamepad::BLUE,
    Gamepad::YELLOW
    };

void GamepadAdapter::setPlayer(Gamepad* gamepad, uint8_t playerNumber)
{
    gamepad->setLed(PLAYER_COLORS[playerNumber]);
}

void GamepadAdapter::connectionComplete(Gamepad* gamepad)
{   // On connection complete, set player led
    gamepad->setPlayer(((gamepad->index) % MAX_PLAYERS));
}
