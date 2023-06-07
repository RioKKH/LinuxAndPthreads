#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>

std::atomic<int> count(0); // アトミックな整数を0で初期化

void increment()
{
    for (int i = 0; i < 100000; ++i)
    {
        ++count; // これがアトミック演算になる
    }
}



int main()
{
    std::thread t1(increment);
    std::thread t2(increment);

    t1.join();
    t2.join();

    std::cout << "Count: " << count << std::endl; // 200,000を出力する

    return 0;
}
