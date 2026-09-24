#include <cassert>
#include <cstdint>
#include <iostream>


enum class AckStatus
{
    IN_PROGRESS,
    OK,
    PREEMPTED,
    BUSY,
    ERROR
};


enum class RetryState
{
    IDLE,
    WAITING,
    SUCCEEDED,
    FAILED
};


enum class TimeoutAction
{
    NONE,
    RETRY,
    FAILED
};

class RetryPolicy
{
public:
    RetryPolicy(
        uint64_t timeout_ms = 100,
        unsigned max_retries = 3)
        : timeout_ms_(timeout_ms),
          max_retries_(max_retries)
    {
    }

    /*
     * 开始一个新请求。
     *
     * 要求：
     * 1. 保存SEQ
     * 2. 保存第一次发送时间
     * 3. 重试次数清零
     * 4. 状态变为WAITING
     */
    void start(
        uint32_t seq,
        uint64_t now_ms)
    {
        // TODO：自己实现
        expected_seq_=seq;
        last_send_ms_=now_ms;
        retries_used_=0;
        state_=RetryState::WAITING;

    }

    /*
     * 处理ACK。
     *
     * 返回值：
     * true  = 属于当前等待的SEQ
     * false = 没有等待或SEQ不匹配
     *
     * IN_PROGRESS：
     *   仍然等待最终结果。
     *
     * OK：
     *   SUCCEEDED。
     *
     * PREEMPTED/BUSY/ERROR：
     *   FAILED。
     */
    bool on_ack(
        uint32_t seq,
        AckStatus status)
    {
       if(state_!=RetryState::WAITING)
       {
        return false;
       }
       if(seq!=expected_seq_)
       {
        return false;
       }
       switch(status)
       {
        case AckStatus::IN_PROGRESS:
        {
            return true;
        }
        case AckStatus::OK:
        {
            state_=RetryState::SUCCEEDED;
            return true;
        }
        case AckStatus::PREEMPTED:
        case AckStatus::BUSY:
        case AckStatus::ERROR:
        {
            state_=RetryState::FAILED;
            return true;
        }
        default:
        {
            return false;
        }

       }
       
    }

    /*
     * 检查超时。
     *
     * 尚未超时：
     *   NONE
     *
     * 已超时且仍有重试次数：
     *   更新发送时间
     *   重试次数+1
     *   RETRY
     *
     * 已超时且重试次数耗尽：
     *   状态改为FAILED
     *   返回FAILED
     */
    TimeoutAction on_timeout(
        uint64_t now_ms)
    {
        if (state_ != RetryState::WAITING)
    {
        return TimeoutAction::NONE;
    }


    if (now_ms - last_send_ms_ < timeout_ms_)
    {
        return TimeoutAction::NONE;
    }


    if (retries_used_ >= max_retries_)
    {
        state_ = RetryState::FAILED;

        return TimeoutAction::FAILED;
    }

  
    retries_used_++;

    last_send_ms_ = now_ms;

    return TimeoutAction::RETRY;
}

 
    RetryState state() const
    {
        return state_;
    }

    unsigned retry_count() const
    {
        return retries_used_;
    }

    uint32_t current_seq() const
    {
        return expected_seq_;
    }

private:
    uint32_t expected_seq_ = 0;

    uint64_t last_send_ms_ = 0;

    uint64_t timeout_ms_ = 100;

    unsigned max_retries_ = 3;

    unsigned retries_used_ = 0;

    RetryState state_ = RetryState::IDLE;
};


int main()
{
    RetryPolicy policy(100, 3);

    /* 测试1：开始请求 */
    policy.start(20, 0);

    assert(
        policy.state() ==
        RetryState::WAITING);

    assert(policy.current_seq() == 20);

    assert(policy.retry_count() == 0);


    /* 测试2：错误SEQ不能结束等待 */
    assert(
        policy.on_ack(
            19,
            AckStatus::OK) == false);

    assert(
        policy.state() ==
        RetryState::WAITING);


    /* 测试3：IN_PROGRESS不是最终成功 */
    assert(
        policy.on_ack(
            20,
            AckStatus::IN_PROGRESS) == true);

    assert(
        policy.state() ==
        RetryState::WAITING);


    /* 测试4：99ms尚未超时 */
    assert(
        policy.on_timeout(99) ==
        TimeoutAction::NONE);


    /* 测试5：100ms触发第一次重试 */
    assert(
        policy.on_timeout(100) ==
        TimeoutAction::RETRY);

    assert(policy.retry_count() == 1);

    assert(policy.current_seq() == 20);


    /* 测试6：匹配最终ACK */
    assert(
        policy.on_ack(
            20,
            AckStatus::OK) == true);

    assert(
        policy.state() ==
        RetryState::SUCCEEDED);


    /* 成功后不能继续重试 */
    assert(
        policy.on_timeout(200) ==
        TimeoutAction::NONE);


    /* 测试7：最大重试次数 */
    policy.start(21, 1000);

    assert(
        policy.on_timeout(1100) ==
        TimeoutAction::RETRY);

    assert(
        policy.on_timeout(1200) ==
        TimeoutAction::RETRY);

    assert(
        policy.on_timeout(1300) ==
        TimeoutAction::RETRY);

    assert(policy.retry_count() == 3);

    assert(policy.current_seq() == 21);


    /* 第四次超时：不允许再发送 */
    assert(
        policy.on_timeout(1400) ==
        TimeoutAction::FAILED);

    assert(
        policy.state() ==
        RetryState::FAILED);


    /* 已失败请求不能被迟到ACK改成成功 */
    assert(
        policy.on_ack(
            21,
            AckStatus::OK) == false);


    /* 测试8：STOP抢占后的业务状态 */
    policy.start(30, 2000);

    assert(
        policy.on_ack(
            30,
            AckStatus::PREEMPTED) == true);

    assert(
        policy.state() ==
        RetryState::FAILED);


    std::cout
        << "retry_policy passed\n";

    return 0;
}