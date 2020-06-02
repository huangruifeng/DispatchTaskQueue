#include <iostream>
#include <thread>
#include "../src/DispatchQueue.h"

static int index = 0;
int main(){
    auto taskQueue = dispatch_task_queue::create();
    taskQueue->asyncDelay([] {
        std::cout << std::this_thread::get_id() << std::endl;
        index = 3;
        std::cout << index << std::endl;;
    }, 10000);

    std::thread t([taskQueue]{
        taskQueue->async([]{
            std::cout << std::this_thread::get_id() << std::endl;
            index = 1;
            std::cout << index<<std::endl;
        });

        taskQueue->asyncDelay([]{
            std::cout << std::this_thread::get_id() << std::endl;
            index = 3;
            std::cout << index << std::endl;;
        }, 1000);
        taskQueue->async([] {
            std::cout << std::this_thread::get_id() << std::endl;
            index = 2;
            std::cout << index <<std::endl;
        },dispatch_task_queue::HIGH);
    });
    t.detach();
    taskQueue->sync([taskQueue]{
        std::cout << std::this_thread::get_id()<<std::endl;
        taskQueue->async([]{
            std::cout << "i am  here!"<<std::endl; },dispatch_task_queue::LOW);

        taskQueue->sync([]{
            index = 4;
            std::cout << index<<std::endl;
        });
    });
    taskQueue->async([] {
        std::cout << std::this_thread::get_id() << std::endl;
    });

    //while(1)
    std::this_thread::sleep_for(std::chrono::seconds(12));
    std::cout << "Hello world!";
    return 0;
}
