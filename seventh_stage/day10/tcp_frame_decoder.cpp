#include <cstdint>
#include <cstddef>
#include <vector>
#include <cassert>
#include <iostream>

class LengthFrameDecoder
{
public:
    bool feed(
        const uint8_t* data,
        size_t size,
        std::vector<std::vector<uint8_t>>& frames)
    {

        if (data == nullptr && size != 0U)
        {
            reset();
            return false;
        }


        if (size == 0U)
        {
            return true;
        }


        buffer_.insert(
            buffer_.end(),
            data,
            data + size);

        while (true)
        {

            if (buffer_.size() < 2U)
            {
                break;
            }

            uint16_t payload_len =
                (static_cast<uint16_t>(buffer_[0]) << 8U) |
                static_cast<uint16_t>(buffer_[1]);


            if (payload_len == 0U ||
                payload_len > 32U)
            {
                reset();
                return false;
            }

            size_t frame_size =
                2U + static_cast<size_t>(payload_len);

            if (buffer_.size() < frame_size)
            {
                break;
            }

    
            frames.emplace_back(
                buffer_.begin() + 2,
                buffer_.begin() + frame_size);

  
            buffer_.erase(
                buffer_.begin(),
                buffer_.begin() + frame_size);
        }

        return true;
    }

    void reset()
    {
        buffer_.clear();
    }

private:
    std::vector<uint8_t> buffer_;
};

int main()
{
    LengthFrameDecoder decoder;
    std::vector<std::vector<uint8_t>> frames;

    /* =========================
       1. 完整帧
       00 04 G R A B
       ========================= */

    const uint8_t frame1[] =
    {
        0x00, 0x04,
        'G', 'R', 'A', 'B'
    };

    assert(decoder.feed(
        frame1,
        sizeof(frame1),
        frames));

    assert(frames.size() == 1U);

    assert(frames[0] ==
           std::vector<uint8_t>(
               {'G', 'R', 'A', 'B'}));

    /* =========================
       2. 半包
       ========================= */

    decoder.reset();
    frames.clear();

    const uint8_t part1[] =
    {
        0x00, 0x04, 'G'
    };

    const uint8_t part2[] =
    {
        'R', 'A', 'B'
    };

    assert(decoder.feed(
        part1,
        sizeof(part1),
        frames));

    /* 尚未组成完整帧 */
    assert(frames.empty());

    assert(decoder.feed(
        part2,
        sizeof(part2),
        frames));

    assert(frames.size() == 1U);

    assert(frames[0] ==
           std::vector<uint8_t>(
               {'G', 'R', 'A', 'B'}));

    /* =========================
       3. 粘包
       OPEN + STOP
       ========================= */

    decoder.reset();
    frames.clear();

    const uint8_t sticky[] =
    {
        0x00, 0x04,
        'O', 'P', 'E', 'N',

        0x00, 0x04,
        'S', 'T', 'O', 'P'
    };

    assert(decoder.feed(
        sticky,
        sizeof(sticky),
        frames));

    assert(frames.size() == 2U);

    assert(frames[0] ==
           std::vector<uint8_t>(
               {'O', 'P', 'E', 'N'}));

    assert(frames[1] ==
           std::vector<uint8_t>(
               {'S', 'T', 'O', 'P'}));

    /* =========================
       4. 非法长度0
       ========================= */

    decoder.reset();
    frames.clear();

    const uint8_t invalid_zero[] =
    {
        0x00, 0x00
    };

    assert(!decoder.feed(
        invalid_zero,
        sizeof(invalid_zero),
        frames));

    /* =========================
       5. 长度超过32
       0x0021 = 33
       ========================= */

    const uint8_t invalid_large[] =
    {
        0x00, 0x21
    };

    assert(!decoder.feed(
        invalid_large,
        sizeof(invalid_large),
        frames));

    /* =========================
       6. 错误后恢复
       ========================= */

    frames.clear();

    assert(decoder.feed(
        frame1,
        sizeof(frame1),
        frames));

    assert(frames.size() == 1U);

    /* =========================
       7. NULL参数
       ========================= */

    assert(!decoder.feed(
        nullptr,
        1U,
        frames));

    std::cout
        << "tcp_frame_decoder passed\n";

    return 0;
}