#include <Gamepad.h>


Gamepad::Command Gamepad::NO_COMMAND = Gamepad::Command();

bool Gamepad::parseDataPacket(uint8_t * packet, uint16_t packetSize)
{
    if(packetSize < MAX_BT_DATA_SIZE && (memcmp(lastPacket,packet,packetSize)!=0))
    {
        LOG_INFO("Parsing Gamepad Data: ");
        LOG_HEXDUMP(packet,packetSize);
        // TODO : parse
        // Store packet content to last packet
        memcpy(lastPacket,packet,packetSize);
        return true;
    }
    else
    {
        return false;
    }
}

bool Gamepad::Command::hasCommand()
{
    return (a || b || up || down || left || right || plus || minus || menu || trig || home );
}

Gamepad::Command Gamepad::getCommand()
{
#ifdef BLUETOOTH
    if (wiimote.available() > 0) 
    {
        ButtonState  button  = wiimote.getButtonState();
        bool a     = (button & BUTTON_A);
        bool b     = (button & BUTTON_B);
        bool c     = (button & BUTTON_C);
        bool z     = (button & BUTTON_Z);
        bool b1     = (button & BUTTON_ONE);
        bool b2     = (button & BUTTON_TWO);
        bool minus = (button & BUTTON_MINUS);
        bool plus  = (button & BUTTON_PLUS);
        bool home  = (button & BUTTON_HOME);
        bool left  = (button & BUTTON_LEFT);
        bool right = (button & BUTTON_RIGHT);
        bool up    = (button & BUTTON_UP);
        bool down  = (button & BUTTON_DOWN);
        if(a || b || c || z || b1 || b2 || minus || plus || home || left || right || up || down)
        {
            currentCommand = Command();
            currentCommand.a = b1 || c;
            currentCommand.b = b2 || z;
            // Change for horizontal orientation
            currentCommand.left = up;
            currentCommand.right = down;
            currentCommand.up = right;
            currentCommand.down = left;
            currentCommand.plus = plus;
            currentCommand.minus = minus;
            currentCommand.menu = a;
            currentCommand.trig = b;
            currentCommand.home = home;
            num_updates++;
            if (logging)
            {
                AccelState   accel   = wiimote.getAccelState();
                NunchukState nunchuk = wiimote.getNunchukState();

                char ca     = (button & BUTTON_A)     ? 'A' : '.';
                char cb     = (button & BUTTON_B)     ? 'B' : '.';
                char cc     = (button & BUTTON_C)     ? 'C' : '.';
                char cz     = (button & BUTTON_Z)     ? 'Z' : '.';
                char c1     = (button & BUTTON_ONE)   ? '1' : '.';
                char c2     = (button & BUTTON_TWO)   ? '2' : '.';
                char cminus = (button & BUTTON_MINUS) ? '-' : '.';
                char cplus  = (button & BUTTON_PLUS)  ? '+' : '.';
                char chome  = (button & BUTTON_HOME)  ? 'H' : '.';
                char cleft  = (button & BUTTON_LEFT)  ? '<' : '.';
                char cright = (button & BUTTON_RIGHT) ? '>' : '.';
                char cup    = (button & BUTTON_UP)    ? '^' : '.';
                char cdown  = (button & BUTTON_DOWN)  ? 'v' : '.';
        
                Serial.printf("button: %05x = ", (int)button);
                Serial.print(ca);
                Serial.print(cb);
                Serial.print(cc);
                Serial.print(cz);
                Serial.print(c1);
                Serial.print(c2);
                Serial.print(cminus);
                Serial.print(chome);
                Serial.print(cplus);
                Serial.print(cleft);
                Serial.print(cright);
                Serial.print(cup);
                Serial.print(cdown);
                Serial.printf(", wiimote.axis: %3d/%3d/%3d", accel.xAxis, accel.yAxis, accel.zAxis);
                Serial.printf(", nunchuk.axis: %3d/%3d/%3d", nunchuk.xAxis, nunchuk.yAxis, nunchuk.zAxis);
                Serial.printf(", nunchuk.stick: %3d/%3d\n", nunchuk.xStick, nunchuk.yStick);
            }
        }
        else
        {
            currentCommand = NO_COMMAND;
        }
    }
/*
    if (! logging)
    {
        long ms = millis();
        if (ms - last_ms >= 1000)
        {
            Serial.printf("Run %d times per second with %d updates\n", num_run, num_updates);
            num_run = num_updates = 0;
            last_ms += 1000;
        }
    }*/
#endif

    return currentCommand;
}
