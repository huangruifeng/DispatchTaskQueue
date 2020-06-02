#pragma once
#include <mutex>
#include <condition_variable>
#include <chrono>
namespace dispatch_task_queue 
{
    enum task_priority
    {
        DROPABLE,
        LOW,
        NORMAL,
        HIGH
    };

    class task_signal
    {
    public:
        task_signal(task_priority pri = NORMAL) :_signal(false), _triggerMs(0), _priority(pri)
        {}
        task_signal(int64_t trigger, task_priority pri = NORMAL) :_signal(false), _triggerMs(trigger), _priority(pri)
        {}
        virtual ~task_signal()= default;

        virtual void run() = 0;

        virtual void signal()
        {
            _signal = true;
            _condition.notify_all();
        }

        virtual void wait()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _condition.wait_for(lock, std::chrono::milliseconds(100), [this] {
                return _signal;
            });
            _signal = false;
        }

        virtual void reset()
        {
            _signal = false;
        }
        inline int64_t triggerMs()const
        {
            return _triggerMs;
        }

        inline void setPriority(task_priority pri)
        {
            _priority = pri;
        }

        inline task_priority priority() const
        {
            return _priority;
        }
    private:
        bool _signal;
        std::mutex _mutex;
        int64_t _triggerMs;
        std::condition_variable _condition;
        task_priority _priority;
    };

    template <class T>
    class Task : public task_signal
    {
    public:
        explicit Task(T task, task_priority pri = NORMAL) :task_signal(pri), _task(task)
        {}
        explicit Task(T task, int64_t trigger, task_priority pri = NORMAL) :task_signal(trigger, pri), _task(task)
        {}
        void run() override
        {
            _task();
        }
    private:
        T _task;
    };

    class task_queue
    {
    public:
        task_queue(int thread_count, const std::string&name = "") :_name(name) {}
        virtual ~task_queue() = default;

        template <class T, typename std::enable_if<std::is_copy_constructible<T>::value>::type* = nullptr>
        void sync(const T&task, task_priority pri = NORMAL)
        {
            sync(std::shared_ptr<task_signal>(new Task<T>(task, pri)));
        }

        template <class T, typename std::enable_if<std::is_copy_constructible<T>::value>::type* = nullptr>
        void async(const T& task, task_priority pri = NORMAL)
        {
            async(std::shared_ptr<task_signal>(new Task<T>(task, pri)));
        }

        template <class T, typename std::enable_if<std::is_copy_constructible<T>::value>::type* = nullptr>
        int64_t asyncDelay(const T &task, int delayMs, task_priority pri = NORMAL) {
            int64_t trigger = delayMs + std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            return asyncDelay(std::shared_ptr<task_signal>(new Task<T>(task, trigger, pri)));
        }

        void sync(std::shared_ptr<task_signal> task)
        {
            if (nullptr != task)
            {
                sync_imp(task);
            }
        }

        int64_t async(std::shared_ptr<task_signal> task)
        {
            if (nullptr != task)
            {
                return async_imp(task);
            }
            return -1;
        }

        int64_t asyncDelay(std::shared_ptr<task_signal> task)
        {
            if (nullptr != task)
            {
                return async_delay_imp(task);
            }
            return -1;
        }
        virtual int64_t clear() = 0;

        virtual std::string name() const {
            return _name;
        }

    protected:
        virtual void sync_imp(std::shared_ptr<task_signal> task) = 0;

        virtual int64_t async_imp(std::shared_ptr<task_signal> task) = 0;

        virtual int64_t async_delay_imp(std::shared_ptr<task_signal> task) = 0;
    private:
        std::string _name;
    };

    std::shared_ptr<task_queue> create(int thread_count = 1);
    std::shared_ptr<task_queue> create(const std::string &name);
    std::shared_ptr<task_queue> create(int thread_count, const std::string &name);

}