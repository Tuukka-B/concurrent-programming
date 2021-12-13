//
// Created by tuukka on 22.9.2021.
//
#ifndef CONCURRENT_PROGRAMMING_ASS1_H
#define CONCURRENT_PROGRAMMING_ASS1_H

#endif //CONCURRENT_PROGRAMMING_ASS1_H

/**
 * Teacher's Instructions for Assignment 1:
 *
 * Make a new console app.
 * Detect amount of logical processors at app runtime.
 * Create as many threads as your computer has logical processors.
 * Each thread runs simultaneously a for loop for 1000 iterations and print the current loop value while running.
 * In main, wait for all threads to finish.
 * Copy the source code to Moodle return box.
 */


#include <iostream>
#include <thread>

using namespace std;
int ass1_task(int thread_count) {

    for (int i = 1; i < 1001; ++i) {
        cout << "thread #" << thread_count << ": " << i << "\n";
    }

}

int ass1_main() {
    unsigned int thread_n = thread::hardware_concurrency();
    thread available_threads[thread_n];
    for (int i = 0; i < thread_n; ++i) {
        available_threads[i] = thread(ass1_task, i);
    }

    for (int i = 0; i < thread_n; i++){
        available_threads[i].join();
    }
}