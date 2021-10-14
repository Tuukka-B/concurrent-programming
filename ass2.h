//
// Created by tuukka on 23.9.2021.
//

#ifndef CONCURRENT_PROGRAMMING_ASS2_H
#define CONCURRENT_PROGRAMMING_ASS2_H

#endif //CONCURRENT_PROGRAMMING_ASS2_H

#include <iostream>
#include <thread>
#include <vector>

/**
 * Teacher's instructions for assignment 2:
 *
 *  * Create new console app.
 *  * Ask user of how many numbers to allocate.
 *  * Allocate an array of numbers and pass it to thread that sets array elements to random numbers.
 *  * In main, wait for the thread to finish and print the contents of the array.
 *  * Copy your source code to Moodle return box.
 */

using namespace std;

vector<int> thread_task(vector<int>& numbers) {

    for (int i = 0; i < numbers.size(); ++i) {
        numbers[i] = rand();
    }
}

int ass2_main() {
    int number;
    cout << "Give a number as a size for the array: ";
    cin >> number;

    vector<int> numbers(number);

    thread worker = thread(thread_task, ref(numbers));

    worker.join();
    cout << "The lucky numbers for the array are:\n";

    for (int i = 0; i < numbers.size(); ++i) {
        cout << "Slot " << i + 1 <<": " << numbers[i] << "\n";
    }

    cout << "Bye!\n";
}