#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

uint16_t crc16_modbus(
    const uint8_t *data,
    size_t size)
{
    if (data == NULL && size != 0U)
    {
        return 0U;
    }

    uint16_t crc = 0xFFFFU;

    for (size_t i = 0U; i < size; ++i)
    {
        crc ^= (uint16_t)data[i];

        for (int bit = 0; bit < 8; ++bit)
        {
            if ((crc & 0x0001U) != 0U)
            {
                crc =
                    (uint16_t)((crc >> 1U) ^ 0xA001U);
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc;
}

bool frame_crc_valid(
    const uint8_t *frame,
    size_t size)
{
    if (frame == NULL || size < 2U)
    {
        return false;
    }

    /*
     * 最后两个字节：
     * CRC_LOW
     * CRC_HIGH
     */
    size_t payload_size = size - 2U;

    uint16_t expected_crc =
        (uint16_t)frame[payload_size] |
        ((uint16_t)frame[payload_size + 1U] << 8U);

    uint16_t actual_crc =
        crc16_modbus(frame, payload_size);

    return actual_crc == expected_crc;
}

int main(void)
{
    /* 1. 已知测试向量 */
    const uint8_t test_data[] = "123456789";

    uint16_t crc =
        crc16_modbus(
            test_data,
            strlen((const char *)test_data));

    assert(crc == 0x4B37U);

    /* 2. 构造正确CRC帧 */
    uint8_t frame[] =
    {
        0x01U,
        0x03U,
        0x00U,
        0x00U,
        0x00U,
        0x01U,
        0x00U,
        0x00U
    };

    uint16_t frame_crc =
        crc16_modbus(frame, 6U);

    frame[6] =
        (uint8_t)(frame_crc & 0xFFU);

    frame[7] =
        (uint8_t)((frame_crc >> 8U) & 0xFFU);

    assert(frame_crc_valid(
        frame,
        sizeof(frame)));

    /* 3. 修改Payload，CRC应失败 */
    frame[2] ^= 0x01U;

    assert(!frame_crc_valid(
        frame,
        sizeof(frame)));

    /* 4. 长度不足 */
    uint8_t short_frame[] = {0x01U};

    assert(!frame_crc_valid(
        short_frame,
        sizeof(short_frame)));

    /* 5. NULL测试 */
    assert(!frame_crc_valid(NULL, 8U));

    printf("crc16_modbus passed\n");

    return 0;
}