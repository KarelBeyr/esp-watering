#include <Arduino.h>
#include "tools.h"
#include <Preferences.h>

Preferences preferences;

State::State(int f, int d)
{
    freq = f;
    duty = d;
};

State::State()
{
};

void readState(State* state)
{
  preferences.begin("my-app", false);
  state->freq = preferences.getInt ("freq");
  preferences.end();

  state->duty = 10;
}

void saveState(State* state)
{
  preferences.begin("my-app", false);
  preferences.putInt("freq", state->freq);
  preferences.end();
}