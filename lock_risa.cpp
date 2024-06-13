#include <cassert>
#include <unordered_map>
#include <thread>
#include <vector>
#include <iostream>
#include <stdio.h>

class Lock
{
public:
    Lock()
        : cnt(0) {}
    void initialize()
    {
        cnt = 0; // if locked: -1, if unlocked: 0
    }
    void lock()
    {
        int64_t expected;
        while (true)
        {
            expected = __atomic_load_n(&cnt, __ATOMIC_SEQ_CST); // 値はatomicにloadする必要がある
            if (expected == 0 && __atomic_compare_exchange_n(&cnt, &expected, -1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
            {
                return;
            }
        }
    }

    void unlock()
    {
        //__atomic_fetch_add(&cnt, 1, __ATOMIC_SEQ_CST); -1 => 0
        __atomic_store_n(&cnt, 0, __ATOMIC_SEQ_CST);

        // この実装だと壊れる
        // int cur_cnt = cnt; // load
        // cnt = cur_cnt + 1; // store
        //
    } // unlock: 1

private:
    int64_t cnt = 0;
    // cnt==-1 誰かがlockを取っている
    // cnt==0 誰もlockをとっていない
};

int river_tshirt = 100;
Lock lock;

void function()
{
    // river_tshirt--;

    lock.lock(); // acquire lock
    int current_stock = river_tshirt;
    river_tshirt = current_stock - 1;
    lock.unlock(); // release lock
}

int main()
{
    int thread_num = 100;
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_num; i++)
    {
        threads.emplace_back(function); // start thread
    }

    for (int i = 0; i < thread_num; i++)
    {
        threads[i].join(); // end thread
    }

    std::cout << river_tshirt << std::endl;
}

// tony: function()   load:3                  store:2
// risa: function()           load:3                   store:2
// rina: function()                                             load:2    store:1
