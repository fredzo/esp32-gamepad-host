#include <GamepadAdapter.h>

void GamepadAdapter::setPlayer(Gamepad* gamepad, uint8_t playerNumber)
{
    gamepad->setLed(Gamepad::PLAYER_COLORS[playerNumber]);
}

void GamepadAdapter::connectionComplete(Gamepad* gamepad)
{   // On connection complete, set player led
    gamepad->setPlayer(((gamepad->index) % MAX_PLAYERS));
}
