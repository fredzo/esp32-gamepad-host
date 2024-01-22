#include <Gamepad.h>
#include <GamepadAdapter.h>
#include <Esp32GamepadHostConfig.h>
#include <BluetoothManager.h>

const GamepadColor Gamepad::PURPLE = GamepadColor(0xFF,0x00,0xFF);
const GamepadColor Gamepad::CYAN   = GamepadColor(0x00,0xFF,0xFF);
const GamepadColor Gamepad::RED    = GamepadColor(0xFF,0x00,0x00);
const GamepadColor Gamepad::GREEN  = GamepadColor(0x00,0xFF,0x00);
const GamepadColor Gamepad::BLUE   = GamepadColor(0x00,0x00,0xFF);
const GamepadColor Gamepad::YELLOW = GamepadColor(0xFF,0xFF,0x00);
const GamepadColor Gamepad::WHITE  = GamepadColor(0xFF,0xFF,0xFF);

const GamepadColor Gamepad::PLAYER_COLORS[] = { 
    Gamepad::PURPLE,
    Gamepad::CYAN,
    Gamepad::RED,
    Gamepad::GREEN,
    Gamepad::BLUE,
    Gamepad::YELLOW
};

hsv gamepadColor2hsv(GamepadColor in)
{
    rgb rgbIn;
    rgbIn.r = ((double)in.red)/0xFF;
    rgbIn.g = ((double)in.green)/0xFF;
    rgbIn.b = ((double)in.blue)/0xFF;
    return rgb2hsv(rgbIn);
}

GamepadColor hsv2GamepadColor(hsv in)
{
    rgb rgbResult = hsv2rgb(in);
    GamepadColor result(rgbResult.r * 0xFF, rgbResult.g * 0xFF, rgbResult.b * 0xFF);
    return result;
}

// For color fading : HSV -> RGB and RGB -> HSV conversion (from https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both)
hsv rgb2hsv(rgb in)
{
    hsv         out;
    double      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;                                // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                  // s
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN;                            // its now undefined
        return out;
    }
    if( in.r >= max )                           // > is bogus, just keeps compilor happy
        out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

    out.h *= 60.0;                              // degrees

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}

rgb hsv2rgb(hsv in)
{
    double      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}

void Gamepad::setAdapter(GamepadAdapter * adapter)
{
    this->adapter = adapter;
    updateName();
}

bool Gamepad::isLowLevelSecurity()
{
    if(adapter)
    {
        return adapter->isLowLevelSecurity(this);
    }
    return false;
}

bool Gamepad::parseDataPacket(uint8_t * packet, uint16_t packetSize)
{
    bool parsed = false;
    if(packetSize < MAX_BT_DATA_SIZE && (memcmp(lastPacket,packet,packetSize)!=0))
    {
        if(adapter)
        {
            if(adapter->parseDataPacket(this,packet,packetSize))
            {
                LOG_INFO("%s\n",currentCommand->toString().c_str());
                parsed = true;
            }
        }
        else
        {
            LOG_INFO("No adapter for gamepad %s and packet:\n", toString().c_str());
            LOG_HEXDUMP(packet,packetSize);
        }
        // Store packet content to last packet
        memcpy(lastPacket,packet,packetSize);
    }
    return parsed;
}

// Called on connection complete
void Gamepad::connectionComplete()
{
    state = Gamepad::State::CONNECTED;
    if(adapter)
    {
        adapter->connectionComplete(this);
    }
}

// Called on connection loss
void Gamepad::connectionLost()
{
    state = Gamepad::State::DISCONNECTED;
}

void Gamepad::setRumble(uint8_t left, uint8_t right, uint16_t duration)
{
    this->rumbleLeft = left;
    this->rumbleRight = right;
    LOG_INFO("Setting rumble to (0x%02X,0x%02X) with duration %dms for gamepad %s.\n",left,right,duration,toString().c_str());
    if(adapter)
    {
        adapter->setRumble(this,left,right);
        if(duration > 0)
        {
            rumbleTimer = true;
            rumbleEndTime = millis() + duration;
        }
    }
    else
    {
        LOG_ERROR("No adapter, setRumple() impossible for gamepad %s.\n", toString().c_str());
    }
}

void Gamepad::setLed(GamepadColor color, uint16_t fadeTime)
{
    LOG_INFO("Setting led color to (0x%02X,0x%02X,0x%02X) with fade time %dms for gamepad for gamepad %s.\n",color.red,color.green,color.blue,fadeTime,toString().c_str());
    if(adapter)
    {
        if(this->color != color)
        {
            if(fadeTime > 0)
            {
                fadingTimer = true;
                fadeStartTime = millis();
                fadingStepDuration = fadeTime / FADE_STEPS;
                fromColor = gamepadColor2hsv(this->color);
                toColor = gamepadColor2hsv(color);
                toColorRgb = color;
                fadeNextStepTime = fadeStartTime + fadingStepDuration;
            }
            else
            {
                adapter->setLed(this,color);
                this->color = color;
            }
        }
    }
    else
    {
        LOG_ERROR("No adapter, setLed() impossible for gamepad %s.\n", toString().c_str());
    }
}

void Gamepad::processTasks()
{
    if(rumbleTimer)
    {
        unsigned long now = millis();
        if(now >= rumbleEndTime)
        {   // Stop rumble
            rumbleTimer = false;
            this->rumbleLeft = 0;
            this->rumbleRight = 0;
            if(adapter) adapter->setRumble(this,0,0);
        }
    }
    if(fadingTimer)
    {
        unsigned long now = millis();
        if(now >= fadeNextStepTime)
        {
            fadeNextStepTime = now + fadingStepDuration;
            hsv currentColor;
            int currentStep = (now - fadeStartTime)/fadingStepDuration;
            if(currentStep>=FADE_STEPS)
            {
                fadingTimer = false;
                this->color = toColorRgb;
            }
            else
            {
                currentColor.h = fromColor.h < toColor.h ? fromColor.h+((toColor.h-fromColor.h)/FADE_STEPS)*currentStep : fromColor.h - ((fromColor.h-toColor.h)/FADE_STEPS)*currentStep;
                currentColor.s = fromColor.s < toColor.s ? fromColor.s+((toColor.s-fromColor.s)/FADE_STEPS)*currentStep : fromColor.s - ((fromColor.s-toColor.s)/FADE_STEPS)*currentStep;
                currentColor.v = fromColor.v < toColor.v ? fromColor.v+((toColor.v-fromColor.v)/FADE_STEPS)*currentStep : fromColor.v - ((fromColor.v-toColor.v)/FADE_STEPS)*currentStep;
                this->color = hsv2GamepadColor(currentColor);
            }
            if(adapter) adapter->setLed(this,this->color);
        }
    }
}

void Gamepad::setPlayer(uint8_t playerNumber)
{
    if(adapter)
    {
        adapter->setPlayer(this,playerNumber);
    }
    else
    {
        LOG_ERROR("No adapter, setPlayer() impossible for gamepad %s.\n", toString().c_str());
    }
}


void Gamepad::sendReport(ReportType type, uint8_t header, uint8_t reportId, const uint8_t * reportData, uint8_t reportLength)
{
    if(state != Gamepad::State::CONNECTED)
    {
        LOG_ERROR("ERROR : Invalid device state for gamepad %s.\n", toString().c_str());
        return;
    }
    if(reportLength > MAX_BT_DATA_SIZE)
    {
        LOG_ERROR("ERROR : Invalid report length %d, max length is %d for gamepad %s.\n", reportLength, MAX_BT_DATA_SIZE, toString().c_str());
        return;
    }
    // Prevent concurrent access to report information
    // This critical section only prevents the bluetooth task (on core 0) from consuming a report while the main loop
    // (on core 1) is modifiying it. It will not prevent from overwriting an unsent report. The sent report will
    // always be the last created (no queue implementation).
    xSemaphoreTake( reportAccessMutex, portMAX_DELAY );
    report.reportType = type;
    report.reportHeader = header;
    report.reportId = reportId;
    report.reportCid =  (type == Gamepad::ReportType::R_CONTROL) ? l2capHidControlCid : l2capHidInterruptCid;
    if(reportData && (reportLength > 0)) memcpy(report.reportData, reportData, reportLength);
    report.reportLength = reportLength;
    //LOG_DEBUG("Request send for cid  %d.\n", report.reportCid);
    bluetoothManagerSendReport(this,&(report.reportCid));
    xSemaphoreGive(reportAccessMutex);
}

GamepadCommand* Gamepad::getCommand()
{
    return currentCommand;
}

void Gamepad::updateName()
{
    if(adapter)
    {
        name = adapter->getName();
    }
    else
    {
        name = UNDEFINED_ADAPTER_NAME;
    }
    name = "#" + String(index) + " - " + name;
}

String Gamepad::getName()
{
    return name;
}

String Gamepad::toString()
{
    char buffer[128];
    sprintf(buffer,"%s [%s,%s,led(0X%02X,0X%02X,0X%02X),rumble(0X%02X,0X%02X)]",name.c_str(),bd_addr_to_str(address),state == State::CONNECTED ? "connected" : state == State::DISCONNECTED ? "disconnected" : "connecting", color.red, color.green, color.blue, rumbleLeft,rumbleRight);
    return String(buffer);
}
