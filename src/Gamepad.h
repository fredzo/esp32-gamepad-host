#ifndef GAMEPAD_H
#define GAMEPAD_H
#include <Arduino.h>

class Gamepad
{
    public :
        class Command {
            public :
                Command() {};       // Wiimote in horizontal orientation
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
                bool hasCommand();
        };
        static Command NO_COMMAND;

        void init();
        void processTasks();
        Command getCommand();

    private :
        bool logging = false;
        long last_ms = 0;
        int num_run = 0, num_updates = 0;
        Command  currentCommand = NO_COMMAND;


};

#endif 