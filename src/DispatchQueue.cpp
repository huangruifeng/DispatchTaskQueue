#include "DispatchQueue.h"
#include <deque>
#include <vector>
#include <sstream>
#include <thread>

namespace dispatch_task_queue {
    template<class T>
    class priority_queue
    {
    public:
        priority_queue() = default;
        ~priority_queue() = default;
        priority_queue(const priority_queue*) = delete;
        bool empty() const
        {
            return (_drop_fifo.empty() && _low_fifo.empty() && _normal_fifo.empty() && _priority_fifo.empty());
        }

        size_t size()
        {
            return _drop_fifo.size() + _low_fifo.size() + _normal_fifo.size() + _priority_fifo.size();
        }

        void push(const T& task)
        {
            if (task->priority() == HIGH)
                _priority_fifo.push_back(task);
            else if (task->priority() == NORMAL)
                _normal_fifo.push_back(task);
            else if (task->priority() == LOW)
                _low_fifo.push_back(task);
            else
                _drop_fifo.push_back(task);
        }

        T& top()
        {
            if (!_priority_fifo.empty())
                return _priority_fifo.front();

            if (!_normal_fifo.empty())
                return _normal_fifo.front();

            if (!_low_fifo.empty())
                return _low_fifo.front();

            return _drop_fifo.front();
        }

        void pop()
        {
            if (!_priority_fifo.empty())
            {
                _priority_fifo.pop_front();
                return;
            }
            if (!_normal_fifo.empty())
            {
                _normal_fifo.pop_front();
                return;
            }
            if (!_low_fifo.empty())
            {
                _low_fifo.pop_front();
                return;
            }
            _drop_fifo.pop_front();
        }
    protected:
        std::deque<T> _drop_fifo;
        std::deque<T> _low_fifo;
        std::deque<T> _normal_fifo;
        std::deque<T> _priority_fifo;
    };


    template<class TCond = std::condition_variable>
    class mutex_task_queue : public task_queue
    {
    public:
        mutex_task_queue(int thread_count, const std::string &name) :task_queue(thread_count, name),
            _thread_count(thread_count),
            _cancel(false)
        {
            for (int i = 0;i< _thread_count;i++)
            {
                std::stringstream thName;
                thName << i << "_" << name;
                create_thread(thName.str());
            }
        }
        ~mutex_task_queue()
        {
            _cancel = true;
            _condition.notify_all();
            for (auto& tr: _threads)
            {
                tr->join();
                delete tr;
            }
        }

    protected:
        void sync_imp(std::shared_ptr<task_signal> task) override
        {
            if(_thread_count == 1 && 
                _thread_ids[0] == std::this_thread::get_id())
            {
                task->reset();
                task->run();
                task->signal();
            }else
            {
                async_imp(task);
                task->wait();
            }
        }

        int64_t async_imp(std::shared_ptr<task_signal> task) override
        {
            if(_tasks.size() > _max_queue_length && task->priority() == DROPABLE)
            {
                return -2;
            }

            _mutex.lock();
            task->reset();
            _tasks.push(task);
            _mutex.unlock();
            _condition.notify_one();
            return 0;
        }

        int64_t async_delay_imp(std::shared_ptr<task_signal> task) override
        {
            const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            if (task->triggerMs() <= now)
            {
                return async_imp(task);
            }

            if (_delat_tasks.size() > _max_queue_length && task->priority() == DROPABLE)
            {
                return -2;
            }
            _mutex.lock();
            task->reset();
            _delat_tasks.push(task);
            _mutex.unlock();
            _condition.notify_one();
            return 0;

        }

        int64_t clear() override
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            const int64_t size = _tasks.size();
            while (!_tasks.empty())
            {
                std::shared_ptr<task_signal> task(_tasks.top());
                _tasks.pop();
                task->signal();
            }
            return size;
        }
    private:
        void* thread_func(std::string name)
        {
            _thread_ids.emplace_back(std::this_thread::get_id());

            while(!_cancel)
            {
                std::unique_lock<std::mutex> signal_mutex(_signal_mutex);
                while (!_cancel && _tasks.empty() && _delat_tasks.empty())
                {
                    _condition.wait_for(signal_mutex, std::chrono::milliseconds(50));
                }
                
                while(!_cancel)
                {
                    _mutex.lock();

                    int size = _delat_tasks.size();
                    for(int i = 0;i<size;i++)
                    {
                        const int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                        const std::shared_ptr<task_signal> delayTask = _delat_tasks.top();
                        if(delayTask->triggerMs() < now)
                        {
                            _tasks.push(_delat_tasks.top());
                            _delat_tasks.pop();
                        }
                    }
                    if(_tasks.empty())
                    {
                        _mutex.unlock();
                        break;
                    }
                    std::shared_ptr<task_signal> task(_tasks.top());
                    _tasks.pop();
                    _mutex.unlock();
                    if(nullptr != task)
                    {
                        try
                        {
                            task->run();
                        }catch(...)
                        {
                            
                        }
                        task->signal();
                    }
                }
            }
            return 0;
        }

        void create_thread(std::string name)
        {
            _threads.emplace_back(new std::thread([this,name]{
                thread_func(name);
            }));
        }
    private:
        std::vector<std::thread*> _threads;
        std::vector<std::thread::id> _thread_ids;
        std::recursive_mutex _mutex;
        std::mutex _signal_mutex;
        TCond _condition;

        priority_queue<std::shared_ptr<task_signal>> _tasks;
        priority_queue<std::shared_ptr<task_signal>> _delat_tasks;
        bool _cancel;
        int _thread_count;
        const int _max_queue_length = 2048;
        uint64_t _runUtcTime;
    };


    std::shared_ptr<task_queue> create(int thread_count)
    {
        return create(thread_count, "");
    }

    std::shared_ptr<task_queue> create(const std::string& name)
    {
        return create(1, name);
    }

    std::shared_ptr<task_queue> create(int thread_count, const std::string& name)
    {
        return std::static_pointer_cast<task_queue>(std::make_shared<mutex_task_queue<>>(thread_count, name));
    }
}