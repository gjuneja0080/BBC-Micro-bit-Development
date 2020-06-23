#ifndef H_RADIO
#define H_RADIO

#include "includes.h"

class Contact;
extern vector<Contact> contacts;

void intraSerialSend(ManagedString s);
void intraSerialReturn(ManagedString s);

void heartbeat();
void radioRecv(MicroBitEvent e);
void sendPacket(char type, ManagedString payload);
void addBroadcast();

void sendKey();
int calculateKey(int base, int exp, int mod);

class Contact
{
  public:
    ManagedString origin;
    char group;
    ManagedString alias;
    int key;
};

#endif
