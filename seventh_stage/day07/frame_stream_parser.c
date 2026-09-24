#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define FRAME_HEAD  0xAA
#define FRAME_TAIL  0x55
#define MAX_PAYLOAD 16

typedef enum
{
    WAIT_HEAD,
    WAIT_LEN,
    WAIT_PAYLOAD,
    WAIT_CHECKSUM,
    WAIT_TAIL
}parser_state_t;

typedef struct 
{
    parser_state_t state;

    uint8_t length;
    uint8_t index;
    uint8_t checksum;

    uint8_t payload[MAX_PAYLOAD];
}frame_parser_t;


void parser_init(frame_parser_t *parser)
{
    if(parser==NULL)
    {
        return;
    }
    parser->state=WAIT_HEAD;

    parser->checksum=0;
    parser->index=0;
    parser->length=0;

}
static void parser_recover(frame_parser_t *parser,uint8_t byte)
{
    parser->length=0;
    parser->index=0;
    parser->checksum=0;

    parser->state=(byte==FRAME_HEAD)?WAIT_LEN:WAIT_HEAD;
}


bool parser_feed(
    frame_parser_t *parser,
    uint8_t byte)
{
    if(parser==NULL)
    {
        return false;
    }
    switch (parser->state)
    {
    case WAIT_HEAD:
    {
        if(byte==FRAME_HEAD)
        {
            parser->state=WAIT_LEN;
        }
        return false;
    }
    case WAIT_LEN:
    {
        if(byte>MAX_PAYLOAD)
        {
            parser_recover(parser,byte);
            return false;
        }
        parser->length=byte;
        parser->index=0;
        parser->checksum=byte;

        if(byte==0)
        {
            parser->state=WAIT_CHECKSUM;
        }
        else
        {
            parser->state=WAIT_PAYLOAD;
        }
        return false;
    }
    case WAIT_PAYLOAD:
    {
        parser->payload[parser->index]=byte;
        parser->checksum^=byte;
        parser->index++;

        if(parser->index==parser->length)
        {
            parser->state=WAIT_CHECKSUM;
        }
        return false;
    }
    case WAIT_CHECKSUM:
    {
        if(byte==parser->checksum)
        {
            parser->state=WAIT_TAIL;
        }
        else
        {
            parser_recover(parser,byte);
        }
        return false;
    }
    case WAIT_TAIL:
    {
        if(byte==FRAME_TAIL)
        {
            parser->state=WAIT_HEAD;
            return true;
        }
        parser_recover(parser,byte);
        return  false;    
    }
    default:
    {
        parser_recover(parser,byte);
        return false;
    }
    }
}
int main(void)
{
    frame_parser_t parser;

    parser_init(&parser);

    /*
     * HEAD = AA
     * LEN = 02
     * PAYLOAD = 10 20
     * CHECKSUM = 02 ^ 10 ^ 20 = 32
     * TAIL = 55
     */
    uint8_t frame[] =
    {
        0xAA,
        0x02,
        0x10,
        0x20,
        0x32,
        0x55
    };

    bool completed = false;

    for (unsigned int i = 0;
         i < sizeof(frame);
         i++)
    {
        completed =
            parser_feed(&parser, frame[i]);

        if (i < sizeof(frame) - 1)
        {
            assert(completed == false);
        }
    }

    assert(completed == true);

    assert(parser.length == 2);
    assert(parser.payload[0] == 0x10);
    assert(parser.payload[1] == 0x20);


    parser_init(&parser);

    uint8_t empty_frame[] =
    {
    0xAA, 0x00, 0x00, 0x55
    };

    completed = false;

    for (size_t i = 0; i < sizeof(empty_frame); i++)
    {
        completed = parser_feed(&parser, empty_frame[i]);
    }

    assert(completed == true);
    assert(parser.length == 0);

    parser_init(&parser);

    uint8_t bad_checksum[] =
    {   
    0xAA, 0x01, 0x10, 0x00, 0x55
    };

    int valid_count = 0;

    for (size_t i = 0; i < sizeof(bad_checksum); i++)
    {
    if (parser_feed(&parser, bad_checksum[i]))
    {
        valid_count++;
    }
    }

    assert(valid_count == 0);
    assert(parser.state == WAIT_HEAD);



    printf("frame parser passed\n");

    return 0;
}