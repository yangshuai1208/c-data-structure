#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum
{
    ACTION_NONE = 0,
    ACTION_OPEN,
    ACTION_GRAB,
    ACTION_RELEASE,
    ACTION_STOP
} action_t;

typedef enum
{
    STATE_IDLE = 0,
    STATE_OPENING,
    STATE_GRABBING,
    STATE_RELEASING,
    STATE_STOPPING
} state_t;

bool action_is_stop(action_t action)
{
    return action == ACTION_STOP;
}
bool state_is_busy(state_t state)
{
    return state == STATE_OPENING ||
       state == STATE_GRABBING ||
       state == STATE_RELEASING ||
       state == STATE_STOPPING;
}

state_t action_to_state(action_t action)
{
    switch(action)
    {
        case ACTION_OPEN:
            return STATE_OPENING;
        case ACTION_GRAB:
            return STATE_GRABBING;
        case ACTION_RELEASE:    
            return STATE_RELEASING;
        case ACTION_STOP:
            return STATE_STOPPING;
        default:
            return STATE_IDLE;
    }
}

bool try_start_action(
    state_t *current_state,
    action_t action)
{
    if(current_state == NULL)
    {
        return false;
    }
     if(action==ACTION_NONE)
    {
        return false;
    }
    if(action_is_stop(action))
    {
        *current_state=STATE_STOPPING;
        return true;
    }
    if(state_is_busy(*current_state))
    {
        return false;
    }
  
  
    *current_state=action_to_state(action);
     return true;
   

   
}
int main(void)
{
    state_t state = STATE_IDLE;

    assert(
        try_start_action(
            &state,
            ACTION_GRAB) == true);

    assert(state == STATE_GRABBING);

    assert(
        try_start_action(
            &state,
            ACTION_OPEN) == false);

    assert(state == STATE_GRABBING);

    assert(
        try_start_action(
            &state,
            ACTION_STOP) == true);

    assert(state == STATE_STOPPING);

    state = STATE_IDLE;

    assert(
        try_start_action(
            &state,
            ACTION_NONE) == false);

    assert(state == STATE_IDLE);

    printf("command_preemption passed\n");

    return 0;
}