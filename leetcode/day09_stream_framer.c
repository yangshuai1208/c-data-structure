#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define FRAME_BUFFER_SIZE 16U

typedef enum
{
    FRAMER_WAITING = 0,
    FRAMER_FRAME_READY,
    FRAMER_FRAME_OVERFLOW,
    FRAMER_OUTPUT_TOO_SMALL,
    FRAMER_INVALID_ARGUMENT
} FramerResult;

typedef struct
{
    char buffer[FRAME_BUFFER_SIZE];

    /*
     * 当前已经保存的有效字符数。
     * 不包括字符串结束符。
     */
    size_t length;

    /*
     * true表示前一帧已经溢出，
     * 当前正在丢弃剩余字符，
     * 直到遇到帧尾。
     */
    bool discarding;
} StreamFramer;

static bool is_delimiter(char byte)
{
    return byte == '\r' || byte == '\n';
}

static void framer_init(StreamFramer *framer)
{
    /*
     * TODO 1：
     * 1. 检查空指针。
     * 2. 将整个结构体初始化为0。
     */
    if(framer==NULL)
    {
        return ;
    }
    *framer=(StreamFramer){0};
}

static FramerResult framer_feed_byte(
    StreamFramer *framer,
    char byte,
    char *output,
    size_t output_capacity)
{
    /*
     * TODO 2：
     * 检查framer、output和output_capacity。
     *
     * 参数错误返回：
     * FRAMER_INVALID_ARGUMENT
     *
     * 参数错误时不能修改framer和output。
     */
    if(framer==NULL||output==NULL||output_capacity==0)
    {
        return FRAMER_INVALID_ARGUMENT;
    }



    /*
     * TODO 3：
     * 如果当前处于discarding状态：
     *
     * 1. 普通字符直接丢弃。
     * 2. 遇到\r或\n时退出discarding状态。
     * 3. 返回FRAMER_WAITING。
     */
    if(framer->discarding)
    {
        if(is_delimiter(byte))
        {
            framer->discarding=false;
        }
        return FRAMER_WAITING;
    }

    /*
     * TODO 4：
     * 如果收到\r或\n：
     *
     * 1. length为0时忽略空帧。
     * 2. 检查output容量能否容纳：
     *
     *    length个字符 + 1个'\0'
     *
     * 3. 容量不足：
     *    - 丢弃当前帧；
     *    - length清零；
     *    - 返回FRAMER_OUTPUT_TOO_SMALL。
     *
     * 4. 容量足够：
     *    - 把内部buffer复制到output；
     *    - 在output末尾补'\0'；
     *    - length清零；
     *    - 返回FRAMER_FRAME_READY。
     */
if (is_delimiter(byte))
{
    if (framer->length == 0U)
    {
        return FRAMER_WAITING;
    }

    if (framer->length + 1U > output_capacity)
    {
        framer->length = 0U;
        return FRAMER_OUTPUT_TOO_SMALL;
    }

    memcpy(
        output,
        framer->buffer,
        framer->length
    );

    output[framer->length] = '\0';
    framer->length = 0U;

    return FRAMER_FRAME_READY;
}










    /*
     * TODO 5：
     * 如果不是帧尾，检查内部缓冲区是否已满。
     *
     * buffer必须预留一个位置给'\0'，
     * 所以最多只能保存：
     *
     * FRAME_BUFFER_SIZE - 1U
     *
     * 如果已满：
     * 1. length清零；
     * 2. discarding设为true；
     * 3. 返回FRAMER_FRAME_OVERFLOW。
     */
     if (framer->length >= FRAME_BUFFER_SIZE - 1U)
    {
    framer->length = 0U;
    framer->discarding = true;

    return FRAMER_FRAME_OVERFLOW;
    }
      

    /*
     * TODO 6：
     * 缓冲区未满：
     *
     * 1. 把byte保存到buffer[length]。
     * 2. length增加。
     * 3. 返回FRAMER_WAITING。
     */
    framer->buffer[framer->length] = byte;
    ++framer->length;

   
    return FRAMER_WAITING;

   
}

static void test_fragmented_frame(void)
{
    StreamFramer framer;
    char output[64] = "UNCHANGED";

    const char *part1 = "temp=25,";
    const char *part2 = "humi=60\n";

    framer_init(&framer);

    for (size_t index = 0U;
         index < strlen(part1);
         ++index)
    {
        assert(framer_feed_byte(
            &framer,
            part1[index],
            output,
            sizeof(output)
        ) == FRAMER_WAITING);
    }

    for (size_t index = 0U;
         index < strlen(part2);
         ++index)
    {
        FramerResult result = framer_feed_byte(
            &framer,
            part2[index],
            output,
            sizeof(output)
        );

        if (part2[index] == '\n')
        {
            assert(result == FRAMER_FRAME_READY);
        }
        else
        {
            assert(result == FRAMER_WAITING);
        }
    }

    assert(strcmp(
        output,
        "temp=25,humi=60"
    ) == 0);

    assert(framer.length == 0U);
    assert(!framer.discarding);
}

static void test_multiple_frames_and_crlf(void)
{
    StreamFramer framer;
    char output[64];

    const char *stream = "A\nB\r\n";
    const char *expected[] = {"A", "B"};

    size_t frame_count = 0U;

    framer_init(&framer);

    for (size_t index = 0U;
         index < strlen(stream);
         ++index)
    {
        FramerResult result = framer_feed_byte(
            &framer,
            stream[index],
            output,
            sizeof(output)
        );

        if (result == FRAMER_FRAME_READY)
        {
            assert(frame_count < 2U);
            assert(strcmp(
                output,
                expected[frame_count]
            ) == 0);

            ++frame_count;
        }
        else
        {
            assert(result == FRAMER_WAITING);
        }
    }

    assert(frame_count == 2U);
}

static void test_overflow_and_recovery(void)
{
    StreamFramer framer;
    char output[64] = "UNCHANGED";

    framer_init(&framer);

    /*
     * 先写入15个字符，正好填满有效区域。
     */
    for (size_t index = 0U;
         index < FRAME_BUFFER_SIZE - 1U;
         ++index)
    {
        assert(framer_feed_byte(
            &framer,
            'X',
            output,
            sizeof(output)
        ) == FRAMER_WAITING);
    }

    /*
     * 第16个字符触发溢出。
     */
    assert(framer_feed_byte(
        &framer,
        'X',
        output,
        sizeof(output)
    ) == FRAMER_FRAME_OVERFLOW);

    assert(framer.discarding);
    assert(framer.length == 0U);
    assert(strcmp(output, "UNCHANGED") == 0);

    /*
     * 溢出后的字符应继续丢弃。
     */
    assert(framer_feed_byte(
        &framer,
        'Y',
        output,
        sizeof(output)
    ) == FRAMER_WAITING);

    /*
     * 遇到帧尾后恢复同步。
     */
    assert(framer_feed_byte(
        &framer,
        '\n',
        output,
        sizeof(output)
    ) == FRAMER_WAITING);

    assert(!framer.discarding);

    /*
     * 下一帧应该可以正常接收。
     */
    assert(framer_feed_byte(
        &framer,
        'O',
        output,
        sizeof(output)
    ) == FRAMER_WAITING);

    assert(framer_feed_byte(
        &framer,
        'K',
        output,
        sizeof(output)
    ) == FRAMER_WAITING);

    assert(framer_feed_byte(
        &framer,
        '\n',
        output,
        sizeof(output)
    ) == FRAMER_FRAME_READY);

    assert(strcmp(output, "OK") == 0);
}

static void test_output_too_small(void)
{
    StreamFramer framer;

    char normal_output[64];
    char small_output[4] = "OLD";

    const char *text = "ABCDE";

    framer_init(&framer);

    for (size_t index = 0U;
         index < strlen(text);
         ++index)
    {
        assert(framer_feed_byte(
            &framer,
            text[index],
            normal_output,
            sizeof(normal_output)
        ) == FRAMER_WAITING);
    }

    assert(framer_feed_byte(
        &framer,
        '\n',
        small_output,
        sizeof(small_output)
    ) == FRAMER_OUTPUT_TOO_SMALL);

    /*
     * 输出失败不能越界写入。
     */
    assert(strcmp(small_output, "OLD") == 0);

    /*
     * 当前失败帧已丢弃，
     * 下一帧仍应正常处理。
     */
    assert(framer.length == 0U);
}

static void test_empty_and_invalid(void)
{
    StreamFramer framer;
    char output[64] = "STABLE";

    framer_init(&framer);

    assert(framer_feed_byte(
        &framer,
        '\n',
        output,
        sizeof(output)
    ) == FRAMER_WAITING);

    assert(framer_feed_byte(
        &framer,
        '\r',
        output,
        sizeof(output)
    ) == FRAMER_WAITING);

    assert(strcmp(output, "STABLE") == 0);

    assert(framer_feed_byte(
        NULL,
        'A',
        output,
        sizeof(output)
    ) == FRAMER_INVALID_ARGUMENT);

    assert(framer_feed_byte(
        &framer,
        'A',
        NULL,
        sizeof(output)
    ) == FRAMER_INVALID_ARGUMENT);

    assert(framer_feed_byte(
        &framer,
        'A',
        output,
        0U
    ) == FRAMER_INVALID_ARGUMENT);

    /*
     * NULL初始化不应崩溃。
     */
    framer_init(NULL);
}

int main(void)
{
    test_fragmented_frame();
    test_multiple_frames_and_crlf();
    test_overflow_and_recovery();
    test_output_too_small();
    test_empty_and_invalid();

    printf("stream framer tests passed\n");
    return 0;
}