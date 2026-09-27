
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

typedef enum
{
    CMD_OPEN,
    CMD_GRAB,
    CMD_RELEASE,
    CMD_STOP
} command_t;

bool parse_command_frame(
    const char *line,
    uint32_t *seq,
    command_t *command)
{
    if (line == NULL || seq == NULL || command == NULL)
    {
        return false;
    }

   const char *p=line;
   uint32_t parsed_seq=0;

    command_t parsed_command;

    if(strncmp(p,"SEQ:",4)!=0)
    {
        return false;
    }
    p+=4;

    bool has_digit=false;

    while(*p>='0'&&*p<='9')
    {
        has_digit=true;

        uint32_t digit=(uint32_t)(*p-'0');

        if(parsed_seq>(UINT32_MAX-digit)/10U)
        {
            return  false;
        }
        parsed_seq=parsed_seq*10U+digit;

        ++p;
    }
    if(!has_digit||parsed_seq==0U)
    {
        return false;
    }
    if(strncmp(p," CMD:HAND_",10)!=0)
    {
        return false;
    }
    p+=10;
    if(strncmp(p,"OPEN",4)==0)
    {
        parsed_command=CMD_OPEN;
        p+=4;
    }
    else  if(strncmp(p,"GRAB",4)==0)
    {
        parsed_command=CMD_GRAB;
        p+=4;
    }
    else   if(strncmp(p,"RELEASE",7)==0)
    {
        parsed_command=CMD_RELEASE;
        p+=7;
    }
    else   if(strncmp(p,"STOP",4)==0)
    {
        parsed_command=CMD_STOP;
        p+=4;
    }
    else
    {
        return false;
    }

    if(*p=='\0')
    {

    }
    else if(*p=='\n'&&p[1]=='\0')
    {

    }
    else if(*p=='\r'&&p[1]=='\n'&&p[2]=='\0')
    {

    }

    else 
    {
        return false;
    }
    *seq=parsed_seq;
    *command=parsed_command;

    return true;
}

static void assert_rejected(const char *input)
{
    uint32_t seq = 777U;
    command_t command = CMD_STOP;

    assert(!parse_command_frame(
        input,
        &seq,
        &command));

    assert(seq == 777U);
    assert(command == CMD_STOP);
}

int main(void)
{
    uint32_t seq = 0;
    command_t command = CMD_OPEN;

    assert(parse_command_frame(
        "SEQ:101 CMD:HAND_GRAB\r\n",
        &seq,
        &command));

    assert(seq == 101U);
    assert(command == CMD_GRAB);

    assert(parse_command_frame(
        "SEQ:4294967295 CMD:HAND_OPEN",
        &seq,
        &command));

    assert(seq == UINT32_MAX);
    assert(command == CMD_OPEN);

    assert(parse_command_frame(
        "SEQ:102 CMD:HAND_RELEASE\n",
        &seq,
        &command));

    assert(seq == 102U);
    assert(command == CMD_RELEASE);

    assert(parse_command_frame(
        "SEQ:103 CMD:HAND_STOP",
        &seq,
        &command));

    assert(seq == 103U);
    assert(command == CMD_STOP);

    assert_rejected("ABC:101 CMD:HAND_GRAB");

    assert_rejected("SEQ:0 CMD:HAND_STOP");

    assert_rejected("SEQ:4294967296 CMD:HAND_OPEN");

    assert_rejected("SEQ:12x CMD:HAND_OPEN");

    assert_rejected("SEQ:12 CMD:HAND_JUMP");

    assert_rejected("SEQ:12 CMD:HAND_STOP abc");

    assert_rejected("SEQ: CMD:HAND_OPEN");

    assert_rejected("SEQ:-1 CMD:HAND_OPEN");

    assert_rejected("SEQ:12 CMD:HAND_OPEN\r");

    assert_rejected(NULL);

    assert(!parse_command_frame(
        "SEQ:101 CMD:HAND_OPEN",
        NULL,
        &command));

    assert(!parse_command_frame(
        "SEQ:101 CMD:HAND_OPEN",
        &seq,
        NULL));

    printf("command_frame_parser passed\n");

    return 0;
}
