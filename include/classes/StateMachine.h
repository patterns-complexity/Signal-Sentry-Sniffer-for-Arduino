#include <stdio.h>
#include <vector>

enum State {
  IDLE,
  RECORDING,
  SAVING,
  REPLAYING,
};

class StateMachine {
  public:
    StateMachine();
    volatile State getPreviousState();
    volatile State getCurrentState();
    State getStates();
    void setCurrentState(State state);
    void changeState(State state);

  private:
    volatile State current_state;
    volatile State previous_state;
    volatile size_t state_change_counter = 0;
    State states[4];
};