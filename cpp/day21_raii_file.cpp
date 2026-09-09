#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>

class LogFile
{
public:
    explicit LogFile(const char *path)
        : file_(std::fopen(path, "w"))
    {
        if (file_ == nullptr)
        {
            throw std::runtime_error(
                "failed to open file"
            );
        }
    }

    ~LogFile()
    {
        close();
    }

    LogFile(const LogFile &) = delete;
    LogFile &operator=(const LogFile &) = delete;

    LogFile(LogFile &&other) noexcept
        : file_(nullptr)
    {
        /* TODO 1：
         * 接管other的文件指针，
         * 然后把other.file_设为nullptr。
         */
        file_=other.file_;
        other.file_=nullptr;

    }

    LogFile &operator=(LogFile &&other) noexcept
    {
        /* TODO 2：
         * 1. 防止自己移动给自己。
         * 2. 关闭自己原来的文件。
         * 3. 接管other的文件指针。
         * 4. 把other.file_设为nullptr。
         */
        if(this!=&other)
        {
            close(); 
            file_=other.file_;
             other.file_=nullptr;

        }
      
        return *this;
    }

    void write(const std::string &text)
    {
        if (file_ == nullptr)
        {
            throw std::logic_error(
                "file is not open"
            );
        }

        std::fputs(text.c_str(), file_);
        std::fputc('\n', file_);
    }

    bool is_open() const noexcept
    {
        return file_ != nullptr;
    }

private:
    void close() noexcept
    {
        /* TODO 3：
         * 如果file_不为空，关闭文件，
         * 然后将file_设为nullptr。
         */
        if(file_!=nullptr)
        {
            std::fclose(file_);
            file_=nullptr;
        }
    }

    std::FILE *file_;
};

int main()
{
    LogFile first("build/day21_first.log");
    first.write("OPEN");

    LogFile second(std::move(first));

    assert(!first.is_open());
    assert(second.is_open());

    LogFile third("build/day21_second.log");
    third = std::move(second);

    assert(!second.is_open());
    assert(third.is_open());

    third.write("STOP");

    std::puts(
        "Day21 RAII file tests passed."
    );

    return 0;
}