#include "includes.h"

MicroBit uBit;
Navigator navigator;
Config *config;

void showText(ManagedString text)
{
    // Clear display text
    uBit.display.stopAnimation();
    if (text.length() == 1)
        // If only 1 character, just print
        uBit.display.print(text);
    else
        // Otherwise scroll at 55 speed
        uBit.display.scrollAsync(text, 55);
}

ManagedString getSerial()
{
    // Get serial
    auto serial = uBit.getSerial();

    // Pad to 10 bytes
    for (int i = 0; i < 10 - serial.length(); i++)
    {
        serial = serial + ManagedString("0");
    }

    return serial;
}

void showNote(ManagedString text, int duration)
{
   uBit.display.stopAnimation();
   uBit.display.printAsync(text, duration/text.length());
}
