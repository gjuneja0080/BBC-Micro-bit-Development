#ifndef H_MENU
#define H_MENU

#include "includes.h"

class Menu : public State
{
  unsigned int currentEntry = 0;
  vector<ManagedString> entries;
  void showCurrent();

  void (*selectHandler)(
      vector<ManagedString> entries,
      int entryNum);

public:
  Menu(
      void (*selectHandler)(
          vector<ManagedString> entries,
          int entryNum))
      : selectHandler(selectHandler){};
  ~Menu(){};
  void focus();
  void handleA();
  void handleB();
  void handleConfirm();
  void addEntry(ManagedString str);
};

#endif