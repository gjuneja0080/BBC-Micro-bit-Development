#include "includes.h"

void Navigator::activate()
{
    // Adds listeners for A/B click, and A/B hold
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, this, &Navigator::handleA);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, this, &Navigator::handleB);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_HOLD, this, &Navigator::handleLongA);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_HOLD, this, &Navigator::handleLongB);
    // If there are states, focus top state
    if (states.size() > 0)
        states.front()->focus();
}
void Navigator::deactivate()
{
    // Deactivate all handlers
    uBit.messageBus.ignore(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, this, &Navigator::handleA);
    uBit.messageBus.ignore(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, this, &Navigator::handleB);
    uBit.messageBus.ignore(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_HOLD, this, &Navigator::handleLongA);
    uBit.messageBus.ignore(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_HOLD, this, &Navigator::handleLongB);
}
void Navigator::handleA(MicroBitEvent e)
{
    // Call handler for A button
    states.back()->handleA();
}
void Navigator::handleB(MicroBitEvent e)
{
    // Call handler for B button
    states.back()->handleB();
}
void Navigator::handleLongA(MicroBitEvent e)
{
    // Ensure only A is pressed
    if (uBit.buttonB.isPressed())
        return;
    // If at the top of the stack, focus it and return
    if (states.size() == 1)
    {
        states.front()->focus();
        return;
    }

    // Delete bottom of stack, go up the stack and focus
    delete states.back();
    states.pop_back();
    states.back()->focus();
}
void Navigator::handleLongB(MicroBitEvent e)
{
    // If only B is pressed, call the confirm handler for it
    if (uBit.buttonA.isPressed())
        return;
    states.back()->handleConfirm();
}
void Navigator::transitTo(State *state)
{
    // Pushes state to bottom of stack and focuses it
    states.push_back(state);
    states.back()->focus();
}