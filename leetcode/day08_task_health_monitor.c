#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_MONITORED_TASKS 32U

typedef struct
{
    /*
     * 任务最后一次正常运行的Tick。
     */
    uint32_t last_alive_tick;

    /*
     * 该任务允许的最大不活动时间。
     */
    uint32_t deadline_ticks;

    /*
     * 任务是否至少报告过一次心跳。
     */
    bool seen;
} TaskHealth;

/*
 * expired_mask的第index位：
 *
 * 0：对应任务健康
 * 1：对应任务超时或从未报告心跳
 *
 * 返回false表示参数错误。
 * 参数错误时不能修改expired_mask。
 */
static bool collect_expired_mask(
    const TaskHealth *tasks,
    size_t length,
    uint32_t now_tick,
    uint32_t *expired_mask)
{
    /*
     * TODO 1：
     * 检查tasks、expired_mask、length。
     *
     * length必须：
     * 1～MAX_MONITORED_TASKS
     */
    if(tasks==NULL||expired_mask==NULL||length==0||length>MAX_MONITORED_TASKS)
    {
        return false;
    }
    /*
     * TODO 2：
     * 定义局部mask，初始值为0。
     *
     * 先计算到局部变量，
     * 最后成功时再写入输出参数。
     */
    uint32_t mask=0;    





    /*
     * TODO 3：
     * 遍历所有任务。
     *
     * 以下两种情况判定为异常：
     *
     * 1. seen == false
     * 2. now_tick - last_alive_tick >
     *    deadline_ticks
     *
     * 不要专门判断：
     *
     * now_tick >= last_alive_tick
     *
     * uint32_t无符号减法能够处理Tick回绕。
     */
 
    for (size_t index = 0U; index < length; ++index)
    {
        uint32_t elapsed =
            now_tick - tasks[index].last_alive_tick;

        if (!tasks[index].seen ||
            elapsed > tasks[index].deadline_ticks)
        {
            mask |= (UINT32_C(1) << index);
        }
    }



    /*
     * TODO 4：
     * 任务异常时，把对应Bit设置为1。
     *
     * 示例：
     *
     * local_mask |= (1UL << index);
     */







    /*
     * TODO 5：
     * 把局部mask写入expired_mask，
     * 返回true。
     */
    *expired_mask = mask;
    return true;
}

static void test_normal_and_expired(void)
{
    const TaskHealth tasks[] =
    {
        /*
         * now=1601
         *
         * 任务0：
         * 经过601，deadline=500，超时。
         */
        {1000U, 500U, true},

        /*
         * 经过601，deadline=1000，健康。
         */
        {1000U, 1000U, true},

        /*
         * 从未报告心跳，异常。
         */
        {0U, 500U, false},

        /*
         * 经过101，deadline=100，超时。
         */
        {1500U, 100U, true}
    };

    uint32_t mask = 0U;

    assert(collect_expired_mask(
        tasks,
        sizeof(tasks) / sizeof(tasks[0]),
        1601U,
        &mask
    ));

    assert(mask ==
           ((1UL << 0U) |
            (1UL << 2U) |
            (1UL << 3U)));
}

static void test_exact_deadline(void)
{
    const TaskHealth task =
    {
        1000U,
        600U,
        true
    };

    uint32_t mask = 123U;

    assert(collect_expired_mask(
        &task,
        1U,
        1600U,
        &mask
    ));

    /*
     * 经过时间正好等于deadline，
     * 本题规定还没有超时。
     */
    assert(mask == 0U);
}

static void test_tick_wrap(void)
{
    const TaskHealth tasks[] =
    {
        /*
         * last = UINT32_MAX - 50
         * now  = 25
         *
         * 无符号减法结果为76。
         */
        {
            UINT32_MAX - 50U,
            100U,
            true
        },

        {
            UINT32_MAX - 50U,
            75U,
            true
        }
    };

    uint32_t mask = 0U;

    assert(collect_expired_mask(
        tasks,
        2U,
        25U,
        &mask
    ));

    /*
     * 任务0：76 <= 100，健康。
     * 任务1：76 > 75，超时。
     */
    assert(mask == (1UL << 1U));
}

static void test_invalid_arguments(void)
{
    const TaskHealth task =
    {
        100U,
        50U,
        true
    };

    uint32_t mask = 0xA5A5A5A5UL;

    assert(!collect_expired_mask(
        NULL,
        1U,
        200U,
        &mask
    ));

    assert(mask == 0xA5A5A5A5UL);

    assert(!collect_expired_mask(
        &task,
        0U,
        200U,
        &mask
    ));

    assert(!collect_expired_mask(
        &task,
        MAX_MONITORED_TASKS + 1U,
        200U,
        &mask
    ));

    assert(!collect_expired_mask(
        &task,
        1U,
        200U,
        NULL
    ));
}

int main(void)
{
    test_normal_and_expired();
    test_exact_deadline();
    test_tick_wrap();
    test_invalid_arguments();

    printf("task health monitor tests passed\n");
    return 0;
}