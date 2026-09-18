#include <stdio.h>
#include <stdbool.h>

typedef enum
{
    CMD_OPEN=0,
    CMD_GRAB,
    CMD_RELEASE,
    CMD_STOP,
    CMD_MAX
}Command;

typedef void(*command_handler_t)(void);


void handle_open(void)
{
    printf("HAND_OPEN\n");
}
void handle_grab(void)
{
    printf("HAND_GRAB\n");
}

void handle_release(void)
{
    printf("HAND_RELEASE\n");
}

void handle_stop(void)
{
    printf("HAND_STOP\n");
}

command_handler_t command_handlers[CMD_MAX] = {
    handle_open,
    handle_grab,
    handle_release,
    handle_stop
};

bool dispatch_command(Command cmd)
{
    if(cmd<0||cmd>=CMD_MAX)
    {
        printf("Invalid command:%d\n",cmd);
        return false;
    }
    command_handlers[cmd]();
    return true;
}
int main(void)
{
    dispatch_command(CMD_OPEN);
    dispatch_command(CMD_GRAB);
    dispatch_command(CMD_STOP);
    dispatch_command((Command)100);
    return 0;
}
