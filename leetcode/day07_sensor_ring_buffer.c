#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define SENSOR_RING_CAPACITY 5U

typedef struct
{
    uint32_t sequence;
    uint16_t temp_x10;
    uint16_t humi_x10;
    uint8_t valid;
} SensorSample;

typedef struct
{
    SensorSample items[SENSOR_RING_CAPACITY];

    /*
     * 指向下一次写入的位置。
     */

    size_t write_index;

    /*
     * 当前有效元素数量。
     */
    size_t count;
} SensorRingBuffer;

static void ring_buffer_init(
    SensorRingBuffer *buffer)
{
    /*
     * TODO：
     * 1. 检查空指针。
     * 2. 将整个结构体初始化为0。
     */
    if(buffer==NULL)
    {
        return ;
    }
    *buffer = (SensorRingBuffer){0};
}

static bool ring_buffer_push(
    SensorRingBuffer *buffer,
    const SensorSample *sample)
{
    /*
     * TODO：
     * 1. 检查两个指针。
     * 2. 在write_index处保存结构体副本。
     * 3. write_index向前移动并处理回绕。
     * 4. 缓冲区未满时count增加。
     * 5. 缓冲区已满时count保持不变。
     * 6. 成功返回true。
     */
    if(buffer==NULL||sample==NULL)
    {
        return false;
    }
    buffer->items[buffer->write_index]=*sample;
    buffer->write_index=(buffer->write_index+1)%SENSOR_RING_CAPACITY;
    if(buffer->count<SENSOR_RING_CAPACITY)
    {
        buffer->count++;
    }
    return true;
}


static bool ring_buffer_get(
    const SensorRingBuffer *buffer,
    size_t logical_index,
    SensorSample *output)
{
    /*
     * logical_index从最旧数据开始计算：
     *
     * 0             -> 最旧数据
     * count - 1     -> 最新数据
     *
     * TODO：
     * 1. 检查指针。
     * 2. 检查logical_index是否越界。
     * 3. 计算当前最旧元素的真实数组下标。
     * 4. 计算logical_index对应的真实下标。
     * 5. 输出结构体副本。
     * 6. 返回true。
     */
    if(buffer==NULL||output==NULL)
    {
        return false;
    }
    if(logical_index>=buffer->count)
    {
        return false;
    }
      size_t oldest_index =
        (buffer->write_index +
         SENSOR_RING_CAPACITY -
         buffer->count) %
        SENSOR_RING_CAPACITY;

    size_t physical_index =
        (oldest_index + logical_index) %
        SENSOR_RING_CAPACITY;

    *output = buffer->items[physical_index];

    return true;


}

static void test_empty_buffer(void)
{
    SensorRingBuffer buffer;
    SensorSample output =
    {
        999U, 999U, 999U, 1U
    };

    ring_buffer_init(&buffer);

    assert(buffer.count == 0U);
    assert(buffer.write_index == 0U);

    assert(!ring_buffer_get(
        &buffer,
        0U,
        &output
    ));

    /*
     * 读取失败不能修改输出参数。
     */
    assert(output.sequence == 999U);
}

static void test_not_full(void)
{
    SensorRingBuffer buffer;

    const SensorSample sample1 =
    {
        1U, 251U, 601U, 1U
    };

    const SensorSample sample2 =
    {
        2U, 252U, 602U, 1U
    };

    const SensorSample sample3 =
    {
        3U, 253U, 603U, 1U
    };

    SensorSample output = {0};

    ring_buffer_init(&buffer);

    assert(ring_buffer_push(&buffer, &sample1));
    assert(ring_buffer_push(&buffer, &sample2));
    assert(ring_buffer_push(&buffer, &sample3));

    assert(buffer.count == 3U);
    assert(buffer.write_index == 3U);

    assert(ring_buffer_get(&buffer, 0U, &output));
    assert(output.sequence == 1U);

    assert(ring_buffer_get(&buffer, 1U, &output));
    assert(output.sequence == 2U);

    assert(ring_buffer_get(&buffer, 2U, &output));
    assert(output.sequence == 3U);
}

static void test_overwrite_and_wrap(void)
{
    SensorRingBuffer buffer;
    SensorSample output = {0};

    ring_buffer_init(&buffer);

    /*
     * 容量为5，连续写入1～7。
     * 最终应保留3、4、5、6、7。
     */
    for (uint32_t sequence = 1U;
         sequence <= 7U;
         ++sequence)
    {
        SensorSample sample =
        {
            sequence,
            (uint16_t)(250U + sequence),
            (uint16_t)(600U + sequence),
            1U
        };

        assert(ring_buffer_push(
            &buffer,
            &sample
        ));
    }

    assert(buffer.count == SENSOR_RING_CAPACITY);
    assert(buffer.write_index == 2U);

    for (size_t index = 0U;
         index < SENSOR_RING_CAPACITY;
         ++index)
    {
        assert(ring_buffer_get(
            &buffer,
            index,
            &output
        ));

        assert(output.sequence ==
               (uint32_t)(index + 3U));
    }

    assert(!ring_buffer_get(
        &buffer,
        SENSOR_RING_CAPACITY,
        &output
    ));
}

static void test_invalid_arguments(void)
{
    SensorRingBuffer buffer;
    SensorSample sample =
    {
        1U, 250U, 600U, 1U
    };

    SensorSample output = {0};

    ring_buffer_init(&buffer);

    ring_buffer_init(NULL);

    assert(!ring_buffer_push(NULL, &sample));
    assert(!ring_buffer_push(&buffer, NULL));

    assert(!ring_buffer_get(
        NULL, 0U, &output));

    assert(!ring_buffer_get(
        &buffer, 0U, NULL));
}

int main(void)
{
    test_empty_buffer();
    test_not_full();
    test_overwrite_and_wrap();
    test_invalid_arguments();

    printf("sensor ring buffer tests passed\n");
    return 0;
}