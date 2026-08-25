#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
    uint8_t temp_int;
    uint8_t temp_dec;
    uint8_t humi_int;
    uint8_t humi_dec;
    uint8_t valid;
} SensorSample;

static bool calculate_valid_average(
    const SensorSample *samples,
    size_t length,
    uint16_t *average_temp_x10,
    uint16_t *average_humi_x10)
{
    /*
     * TODO 1：检查samples、两个输出指针和length。
     */
    if(samples==NULL||average_temp_x10==NULL||average_humi_x10==NULL||length==0U)
    {
        return false;
    }
    /*
     * TODO 2：定义温度总和、湿度总和、有效样本数量。
     * 累加变量建议使用uint32_t。
     */
    uint32_t temp_sum=0U;
    uint32_t humi_sum=0U;
    uint32_t valid_count=0U;
    /*
     * TODO 3：遍历结构体数组。
     *
     * 只统计valid != 0U的样本。
     *
     * 温度放大十倍：
     * temp_int * 10 + temp_dec
     *
     * 湿度放大十倍：
     * humi_int * 10 + humi_dec
     */
    for(size_t i=0U;i<length;i++)
    {
        if(samples[i].valid!=0U)
        {
            temp_sum+=samples[i].temp_int*10U+samples[i].temp_dec;
            humi_sum+=samples[i].humi_int*10U+samples[i].humi_dec;
            valid_count++;
        }
    }



    /*
     * TODO 4：如果没有有效样本，返回false。
     */
    if(valid_count==0U)
    {
        return false;
    }


    /*
     * TODO 5：计算平均值并写入两个输出参数。
     */
    *average_temp_x10=(uint16_t)(temp_sum/valid_count);
    *average_humi_x10=(uint16_t)(humi_sum/valid_count);


    /*
     * TODO 6：返回true。
     */
    return true;
}

static void test_mixed_samples(void)
{
    const SensorSample samples[] =
    {
        {25U, 2U, 60U, 0U, 1U},
        {26U, 4U, 58U, 0U, 1U},
        {99U, 9U, 99U, 9U, 0U}
    };

    uint16_t average_temp = 0U;
    uint16_t average_humi = 0U;

    bool result = calculate_valid_average(
        samples,
        sizeof(samples) / sizeof(samples[0]),
        &average_temp,
        &average_humi
    );

    assert(result);
    assert(average_temp == 258U);
    assert(average_humi == 590U);
}

static void test_single_valid_sample(void)
{
    const SensorSample samples[] =
    {
        {20U, 5U, 50U, 6U, 0U},
        {23U, 8U, 61U, 2U, 1U}
    };

    uint16_t average_temp = 0U;
    uint16_t average_humi = 0U;

    bool result = calculate_valid_average(
        samples,
        2U,
        &average_temp,
        &average_humi
    );

    assert(result);
    assert(average_temp == 238U);
    assert(average_humi == 612U);
}

static void test_all_invalid(void)
{
    const SensorSample samples[] =
    {
        {20U, 0U, 40U, 0U, 0U},
        {30U, 0U, 70U, 0U, 0U}
    };

    uint16_t average_temp = 123U;
    uint16_t average_humi = 456U;

    bool result = calculate_valid_average(
        samples,
        2U,
        &average_temp,
        &average_humi
    );

    assert(!result);

    /*
     * 失败时不应修改输出参数。
     */
    assert(average_temp == 123U);
    assert(average_humi == 456U);
}

static void test_invalid_arguments(void)
{
    const SensorSample sample =
    {
        25U, 0U, 60U, 0U, 1U
    };

    uint16_t temp = 0U;
    uint16_t humi = 0U;

    assert(!calculate_valid_average(
        NULL, 1U, &temp, &humi));

    assert(!calculate_valid_average(
        &sample, 0U, &temp, &humi));

    assert(!calculate_valid_average(
        &sample, 1U, NULL, &humi));

    assert(!calculate_valid_average(
        &sample, 1U, &temp, NULL));
}

int main(void)
{
    test_mixed_samples();
    test_single_valid_sample();
    test_all_invalid();
    test_invalid_arguments();

    printf("sensor average tests passed\n");
    return 0;
}