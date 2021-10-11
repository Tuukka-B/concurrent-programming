//
// Created by tuukka on 5.10.2021.
//

#ifndef CONCURRENT_PROGRAMMING_ASS3_H
#define CONCURRENT_PROGRAMMING_ASS3_H

#endif //CONCURRENT_PROGRAMMING_ASS3_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

/*
 * Home Assignment 3
 *
 * Create a new console app.
 * Declare a std::vector as an dynamic array of numbers
 * Create two threads, another will add numbers to the vector and another will remove them
 * Synchronize the threads with a Mutex.
 * Copy your source code to the Moodle return box.
 */

using namespace std;

class DataContainer
{
public:
    DataContainer()
    {
        numbers = std::vector<int>(0);
    }

    void add_number() {
            m.lock();
            numbers.push_back(rand());
            cout << "Added " << numbers[numbers.size() - 1] << "\n";
            m.unlock();
    }

    void remove_number() {
            m.lock();
            int number_to_remove = numbers[numbers.size() - 1];
            numbers.pop_back();
            cout << "Removed " << number_to_remove << "\n";
            m.unlock();

    }

    std::vector<int> numbers;
    std::mutex m;
};

void add_number(DataContainer& data) {
    for (int i = 0; i < 100; i++) {
        data.add_number();
    }
}

void remove_number(DataContainer& data) {
    int i = 0;
    while (i < 100) {
        if (!data.numbers.empty()) {
            data.remove_number();
            i++;
        }
    }
}

int ass3_main() {
    DataContainer numberClass;

    thread worker1 = thread(add_number, std::ref(numberClass));
    thread worker2 = thread(remove_number, std::ref(numberClass));

    worker1.join();
    worker2.join();
}