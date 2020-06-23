#ifndef H_MAIN
#define H_MAIN

#include "includes.h"

int main();
void initConfig();
void initMenu();
void handleSerialIn(MicroBitEvent e);
void showList(vector<ManagedString> entries);
void handlePacket(ManagedString p);

ManagedString aes(ManagedString message, int key);

void mainMenuHandler(vector<ManagedString> entries, int entryNum);
void twitterMenuHandler(vector<ManagedString> entries, int entryNum);
void weatherMenuHandler(vector<ManagedString> entries, int entryNum);
void twitterUserMenuHandler(vector<ManagedString> entries, int entryNum);
void twitterHashtagMenuHandler(vector<ManagedString> entries, int entryNum);
void contactsMenuHandler(vector<ManagedString> entries, int entryNum);
void sendMenuHandler(vector<ManagedString> entries, int entryNum);

void initPiano();
int readKeyPress();
void playKeyPress(int pressed, int volume, int duration);
void playNote(int volume, int duration, int freq);
void startKeyboard();
void playTwinkle();
void playDesp();
void playRandomTune();
void exitPiano(MicroBitEvent e);
void pianoHandler(vector<ManagedString> entries, int entryNum);

void lock(MicroBitEvent e);
void hello();

#endif
