#ifndef H_UTIL
#define H_UTIL

#include "includes.h"

extern MicroBit uBit;
extern Navigator navigator;
extern Config *config;

void showText(ManagedString text);
ManagedString getSerial();
void showNote(ManagedString text, int duration);

#endif
