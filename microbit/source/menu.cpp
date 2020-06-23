#include "includes.h"

void Menu::showCurrent()
{
    // Displays text at current position
    showText(entries.at(currentEntry));
}
void Menu::focus()
{
    // Displays current focused entry
    showCurrent();
}
void Menu::handleA()
{
    // A button; decrement position unless at 0, in which case wrap
    if (currentEntry == 0)
        currentEntry = entries.size() - 1;
    else
        currentEntry--;

    // Display entry
    showCurrent();
}
void Menu::handleB()
{
    // B button; increment position unless at end of vector, in which case wrap to 0
    if (currentEntry == entries.size() - 1)
        currentEntry = 0;
    else
        currentEntry++;

    // Display entry
    showCurrent();
}
void Menu::handleConfirm()
{
    // If a handler is added, call it with all entries and current entry number
    if (selectHandler != NULL)
        selectHandler(entries, currentEntry);
}
void Menu::addEntry(ManagedString str)
{
    // Add entry to back of entries vector
    entries.push_back(str);
}
