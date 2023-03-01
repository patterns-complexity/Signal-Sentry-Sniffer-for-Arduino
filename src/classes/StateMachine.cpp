#include "classes/StateMachine.h"

StateMachine::StateMachine() {
  this->states[0] = IDLE;
  this->states[1] = RECORDING;
  this->states[2] = SAVING;
  this->states[3] = REPLAYING;
  this->current_state = IDLE;
}

volatile State StateMachine::getCurrentState() {
  return this->current_state;
}

volatile State StateMachine::getPreviousState() {
  return this->previous_state;
}

void StateMachine::setCurrentState(State state) {
  if (state == this->current_state) {
    return;
  }
  this->current_state = state;
}

void StateMachine::changeState(State state) {
  if (state == this->current_state) {
    return;
  }
  this->previous_state = this->current_state;
  this->current_state = state;
  this->state_change_counter++;
}

