#include <atomic>
#include <thread>
#include <iostream>
#include <functional>
#include <chrono>

int non_atomic_count1 = 0;
int non_atomic_count2 = 0;
int non_atomic_count3 = 0;

std::atomic<int> count1(0);
std::atomic<int> count2(0);
std::atomic<int> count3(0);

constexpr int LOOPNUM = 100000;

void measure_execution_time(std::function<void()> func, const std::string& func_name)
{
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << func_name << " elapsed time: " << elapsed.count() << " ms" << std::endl;
}

void non_atomic_increment1()
{
    for (int i = 0; i < LOOPNUM; ++i)
    {
        ++non_atomic_count1;
    }
}

void non_atomic_increment2()
{
    for (int i = 0; i < LOOPNUM; ++i)
    {
        ++non_atomic_count2;
    }
}

void non_atomic_increment3()
{
    for (int i = 0; i < LOOPNUM; ++i)
    {
        ++non_atomic_count3;
    }
}

void atomic_increment1()
{
    for (int i = 0; i < 100000; ++i)
    {
        ++count1;
    }
}

void atomic_increment2()
{
    for (int i = 0; i < 100000; ++i)
    {
        ++count2;
    }
}

void atomic_increment3()
{
    for (int i = 0; i < 100000; ++i)
    {
        ++count3;
    }
}

void single_threaded_non_atomic_increment()
{
    non_atomic_increment1();
    non_atomic_increment2();
    non_atomic_increment3();
}

void single_threaded_atomic_increment()
{
    atomic_increment1();
    atomic_increment2();
    atomic_increment3();
}

void multi_threaded_non_atomic_increment()
{
    std::thread t1(non_atomic_increment1);
    std::thread t2(non_atomic_increment2);
    std::thread t3(non_atomic_increment3);

    t1.join();
    t2.join();
    t3.join();
}

void multi_threaded_atomic_increment()
{
    std::thread t1(atomic_increment1);
    std::thread t2(atomic_increment2);
    std::thread t3(atomic_increment3);

    t1.join();
    t2.join();
    t3.join();
}

int main()
{
    measure_execution_time(single_threaded_atomic_increment, "Single-threaded atomic increment"); // おおよそ1.1msec
    count1 = 0; // リセット
    count2 = 0; // リセット
    count3 = 0; // リセット
    measure_execution_time(multi_threaded_atomic_increment, "Multi-threaded atomic increment"); // 2.3msec

    measure_execution_time(single_threaded_non_atomic_increment, "Single-threaded non-atomic increment"); // おおよそ1.1msec
    non_atomic_count1 = 0; // リセット
    non_atomic_count2 = 0; // リセット
    non_atomic_count3 = 0; // リセット
    measure_execution_time(multi_threaded_non_atomic_increment, "Multi-threaded non-atomic increment"); // 2.3msec

    return 0;
}

