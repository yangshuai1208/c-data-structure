#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

struct Command
{
    unsigned int sequence;
    std::string name;
};

class PendingCommands
{
public:
    bool add(Command command)
    {
        /*
         * TODO 1：
         * 1. 使用std::find_if查找相同sequence。
         * 2. 如果已经存在，返回false。
         * 3. 否则把command移动进commands_。
         * 4. 返回true。
         */
        const auto iterator=std::find_if(commands_.begin(),commands_.end(),[&command](const Command &c)
        {
            return c.sequence==command.sequence;
        }
        );
      if(iterator!=commands_.end())
      {
        return false;
      }
      commands_.push_back(std::move(command));
      return true;

       
    }

    std::optional<Command> find(
        unsigned int sequence
    ) const
    {
        /*
         * TODO 2：
         * 1. 使用std::find_if查找sequence。
         * 2. 找不到返回std::nullopt。
         * 3. 找到后返回命令副本。
         */
        const auto iterator=std::find_if(commands_.begin(),commands_.end(),[&sequence](const Command &c)
        {
            return c.sequence==sequence;
        }
        );
        if(iterator==commands_.end())
        {
            return std::nullopt;
        }
        return *iterator;
    }

    bool remove(unsigned int sequence)
    {
        /*
         * TODO 3：
         * 1. 使用std::find_if查找sequence。
         * 2. 找不到返回false。
         * 3. 使用commands_.erase(iterator)删除。
         * 4. 返回true。
         */
        const auto iterator=std::find_if(commands_.begin(),commands_.end(),[&sequence](const Command &c)
        {
            return c.sequence==sequence;
        }
        );  
        if(iterator==commands_.end())
        {
         
            return false;
        }

        commands_.erase(iterator);
        return true;
    }

    std::size_t size() const noexcept
    {
        return commands_.size();
    }

private:
    std::vector<Command> commands_;
};

int main()
{
    PendingCommands pending;

    assert(pending.size() == 0U);
    assert(!pending.find(1U).has_value());

    assert(pending.add({1U, "OPEN"}));
    assert(pending.add({2U, "GRAB"}));
    assert(pending.size() == 2U);

    /* 相同sequence必须拒绝 */
    assert(!pending.add({1U, "STOP"}));
    assert(pending.size() == 2U);

    const std::optional<Command> found =
        pending.find(2U);

    assert(found.has_value());
    assert(found->sequence == 2U);
    assert(found->name == "GRAB");

    /*
     * 保存的是命令副本，不是vector内部指针。
     * 后续vector变化不会让found悬空。
     */
    const std::optional<Command> saved =
        pending.find(1U);

    assert(saved.has_value());

    assert(pending.add({3U, "RELEASE"}));
    assert(pending.add({4U, "STOP"}));

    assert(saved->sequence == 1U);
    assert(saved->name == "OPEN");

    assert(pending.remove(2U));
    assert(!pending.find(2U).has_value());
    assert(pending.size() == 3U);

    /* 重复删除必须失败且状态不变 */
    assert(!pending.remove(2U));
    assert(pending.size() == 3U);

    assert(pending.remove(1U));

    /*
     * 原命令已经从容器删除，
     * 但saved是独立副本，仍然有效。
     */
    assert(saved->name == "OPEN");

    std::cout
        << "Day21 pending commands: "
        << "all checks passed.\n";

    return 0;
}