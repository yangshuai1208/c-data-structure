#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

enum class AckState
{
    Waiting,
    Executing,
    Succeeded,
    Failed
};

struct AckEntry
{
    uint32_t seq;
    AckState state;
};

class AckTable
{
public:
    explicit AckTable(size_t capacity)
        : capacity_(capacity)
    {
    }

    bool insert(
        uint32_t seq,
        AckState state)
    {

        for (const auto& entry : entries_)
        {
            if (entry.seq == seq)
            {
                return false;
            }
        }

        if (entries_.size() >= capacity_)
        {
            return false;
        }

        entries_.push_back(
            AckEntry{seq, state});

        return true;
    }

    bool update(
        uint32_t seq,
        AckState state)
    {
        for (auto& entry : entries_)
        {
            if (entry.seq == seq)
            {
                entry.state = state;
                return true;
            }
        }

        return false;
    }

    bool find(
        uint32_t seq,
        AckState& state) const
    {
        for (const auto& entry : entries_)
        {
            if (entry.seq == seq)
            {
                state = entry.state;
                return true;
            }
        }

        return false;
    }

    bool erase(uint32_t seq)
    {
        for (auto it = entries_.begin();
             it != entries_.end();
             ++it)
        {
            if (it->seq == seq)
            {
                entries_.erase(it);
                return true;
            }
        }

        return false;
    }

private:
    std::vector<AckEntry> entries_;
    size_t capacity_;
};

int main()
{
    AckTable table(2);

    AckState state = AckState::Failed;

    /* 1. 插入Waiting */
    assert(table.insert(
        100U,
        AckState::Waiting));

    /* 2. 查找 */
    assert(table.find(
        100U,
        state));

    assert(state == AckState::Waiting);

    /* 3. 更新到Executing */
    assert(table.update(
        100U,
        AckState::Executing));

    assert(table.find(
        100U,
        state));

    assert(state == AckState::Executing);

    /* 4. 更新到Succeeded */
    assert(table.update(
        100U,
        AckState::Succeeded));

    assert(table.find(
        100U,
        state));

    assert(state == AckState::Succeeded);

    /* 5. 重复SEQ插入失败 */
    assert(!table.insert(
        100U,
        AckState::Waiting));

    /* 6. 第二条记录 */
    assert(table.insert(
        101U,
        AckState::Waiting));

    /* 7. 满容量后插入失败 */
    assert(!table.insert(
        102U,
        AckState::Waiting));

    /* 8. 查找失败时不修改输出参数 */
    state = AckState::Failed;

    assert(!table.find(
        999U,
        state));

    assert(state == AckState::Failed);

    /* 9. 删除成功 */
    assert(table.erase(100U));

    /* 10. 删除不存在SEQ失败 */
    assert(!table.erase(100U));

    /* 删除后又有空间 */
    assert(table.insert(
        102U,
        AckState::Waiting));

    std::cout
        << "ack_table passed\n";

    return 0;
}