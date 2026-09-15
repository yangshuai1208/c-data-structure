#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

/* 所有失败情况都必须保留原输出值 */
static void expect_invalid(const uint8_t *buf, size_t size)
{
    uint8_t len = 99U;

    assert(!frame_validate(buf, size, &len));
    assert(len == 99U);
}
bool frame_validate(
    const uint8_t *buf,
    size_t size,
    uint8_t *payload_len)
{

    if (buf == NULL || payload_len == NULL)
    {
        return false;
    }


    if (size < 4U)
    {
        return false;
    }

    if (buf[0] != 0xAAU)
    {
        return false;
    }

    uint8_t len = buf[1];

    if (len > 32U)
    {
        return false;
    }

 
    if (size != (size_t)len + 4U)
    {
        return false;
    }


    if(len+3>=size||buf[len+3]!=0x55U)
    {
        return false;
    }
 
    uint8_t checksum = len;


    for(size_t i=0;i<len;i++)
    {
        checksum^=buf[i+2];
    }
  
    if(checksum!=buf[len+2])
    {
        return false;
    }
 
    *payload_len = len;
    return true;
}
int main(void)
{
    const uint8_t valid[] = {
        0xAA, 0x03, 0x10, 0x20, 0x30, 0x03, 0x55
    };

    const uint8_t empty[] = {
        0xAA, 0x00, 0x00, 0x55
    };

    const uint8_t bad_head[] = {
        0xAB, 0x00, 0x00, 0x55
    };

    const uint8_t bad_tail[] = {
        0xAA, 0x00, 0x00, 0x54
    };

    const uint8_t bad_checksum[] = {
        0xAA, 0x03, 0x10, 0x20, 0x30, 0x02, 0x55
    };

    const uint8_t extra[] = {
        0xAA, 0x00, 0x00, 0x55, 0x99
    };

    /* LEN=33，超过允许上限；总长度仍匹配 */
    uint8_t too_long[37] = {0xAA, 33U};
    too_long[35] = 33U;
    too_long[36] = 0x55;

    uint8_t len = 99U;

    /* 正常帧 */
    assert(frame_validate(valid, sizeof valid, &len));
    assert(len == 3U);

    /* 零载荷 */
    assert(frame_validate(empty, sizeof empty, &len));
    assert(len == 0U);

    /* 最大合法载荷：32字节，载荷全部为0 */
    uint8_t maximum[36] = {0xAA, 32U};
    maximum[34] = 32U;
    maximum[35] = 0x55;

    assert(frame_validate(maximum, sizeof maximum, &len));
    assert(len == 32U);

    /* 空指针与长度异常 */
    expect_invalid(NULL, 0U);
    assert(!frame_validate(valid, sizeof valid, NULL));
    expect_invalid(valid, 0U);
    expect_invalid(valid, 3U);
    expect_invalid(valid, sizeof valid - 1U);

    /* 格式、校验与范围异常 */
    expect_invalid(bad_head, sizeof bad_head);
    expect_invalid(bad_tail, sizeof bad_tail);
    expect_invalid(bad_checksum, sizeof bad_checksum);
    expect_invalid(extra, sizeof extra);
    expect_invalid(too_long, sizeof too_long);

    puts("frame_validate tests passed");
    return 0;
}
