#ifndef H_NAVIGATOR
#define H_NAVIGATOR

#include "includes.h"

class State
{
public:
  virtual ~State(){};
  virtual void handleA() = 0;
  virtual void handleB() = 0;
  virtual void handleConfirm() = 0;
  virtual void focus() = 0;
};

class Navigator
{
  vector<State *> states;

public:
  void activate();
  void deactivate();
  void handleA(MicroBitEvent e);
  void handleB(MicroBitEvent e);
  void handleLongA(MicroBitEvent e);
  void handleLongB(MicroBitEvent e);
  void transitTo(State *state);
};

#endif