#ifndef Tools_h
#define Tools_h
void blink(int times, int msHigh, int msLow);

class State
{
  public:
    State(int f, int d);
    State();
    int freq;
    int duty;

  private:
};

void readState(State* state);
void saveState(State* state);

#endif
