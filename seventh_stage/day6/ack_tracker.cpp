    #include <cassert>
    #include <cstdint>
    #include <iostream>
    #include <mutex>

    class AckTracker
    {
    public:
        void start(uint32_t seq)
        {
            /*
            * TODO
            *
            * 1. 加锁
            * 2. expected_seq_ = seq
            * 3. waiting_ = true
            */
            std::lock_guard<std::mutex>lock(mutex_);
            expected_seq_=seq;
            waiting_=true;


        }

        bool on_ack(uint32_t seq)
        {
          
            std::lock_guard<std::mutex>lock(mutex_);
            if(!waiting_)
            {
                return false;
            }

            if(seq!=expected_seq_)
            {
                return false;
            }

            waiting_=false;
        
            return true;
            


        }

        bool is_waiting() const
        {
         
            std::lock_guard<std::mutex>lock(mutex_);
            return waiting_;
        }

    private:
        mutable std::mutex mutex_;

        bool waiting_ = false;

        uint32_t expected_seq_ = 0;
    };

    int main()
    {
        AckTracker tracker;

        tracker.start(20);

        assert(
            tracker.is_waiting() == true);

        assert(
            tracker.on_ack(19) == false);

        assert(
            tracker.is_waiting() == true);

        assert(
            tracker.on_ack(20) == true);

        assert(
            tracker.is_waiting() == false);


        /* 再测试一次新的SEQ */
        tracker.start(21);

        assert(
            tracker.on_ack(21) == true);

        assert(
            tracker.is_waiting() == false);


        std::cout
            << "ack_tracker passed\n";

        return 0;
    }