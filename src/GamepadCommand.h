#ifndef GAMEPAD_COMMAND_H
#define GAMEPAD_COMMAND_H
#include <Arduino.h>

class GamepadCommand {
    public :
        GamepadCommand() {};       // Wiimote in horizontal orientation
        bool a = false;     // Mapped to "1" button
        bool b = false;     // Mapped to "2" button
        bool up = false;    // Reversed for horizontale position
        bool down = false;  // Reversed for horizontale position
        bool left = false;  // Reversed for horizontale position
        bool right = false; // Reversed for horizontale position
        bool plus = false;  
        bool minus = false;
        bool menu = false;  // Maped to wiimote "A" button
        bool trig = false;  // Maped to wiimote trigger button
        bool home = false;  // Mapped to home button
        bool hasCommand() { return (a || b || up || down || left || right || plus || minus || menu || trig || home ); };

    static GamepadCommand NO_COMMAND;
};
        
GamepadCommand GamepadCommand::NO_COMMAND = GamepadCommand();

#endif 