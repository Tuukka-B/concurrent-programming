//
// Created by tuukka on 8.10.2021.
//

#ifndef CONCURRENT_PROGRAMMING_ASS5_H
#define CONCURRENT_PROGRAMMING_ASS5_H

#endif //CONCURRENT_PROGRAMMING_ASS5_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
using namespace std;


/**
 * Teacher's Instructions for Assignment 5:
 *
 * Create a new console app.
 * Create a worker thread with a long running task.
 * Use 5 condition variables within the worker thread to notify the main thread of the progress.
 *
 * Each condition variable amounts to 20% of the total task done
 *
 * In main thread, wait for the condition variables to complete and print the progress of the worker thread.
 * Copy your source code to the Moodle return box.
 */

// global variables
mutex m_mutex;
unique_lock<mutex> mlock(m_mutex);
condition_variable progress_20;
condition_variable progress_40;
condition_variable progress_60;
condition_variable progress_80;
condition_variable progress_100;

class Application
{
public:
    Application()
    {
        progress = 0;
    }
    int work(){
            this_thread::sleep_for(chrono::seconds (1));
            progress += 20;


    }

    int check_progress() {
        return progress;
    }

    private:
    int progress;
};

void worker(Application& app) {
    auto progress_check = [&app](int progress){ return app.check_progress() >= progress; };

    while (app.check_progress() < 100) {

        app.work();

        if (progress_check(100)) {
            progress_100.notify_one();

        } else if (progress_check(80)) {
            progress_80.notify_one();

        } else if (progress_check(60)) {
            progress_60.notify_one();

        } else if (progress_check(40)) {
            progress_40.notify_one();

        } else if (progress_check(20)) {
            progress_20.notify_one();
        }

    }

}


int ass5_main() {
    Application app;
    auto progress_check = [&app](int progress){ return app.check_progress() >= progress; };
    thread worker1 = thread(worker, ref(app));
    worker1.detach();
    progress_20.wait(mlock, [&app](){ return app.check_progress() >= 20; });
    cout << "Received worker 20% finished\n";

    progress_40.wait(mlock, [&app](){ return app.check_progress() >= 40; });
    cout << "Received worker 40% finished\n";

    progress_60.wait(mlock, [&app](){ return app.check_progress() >= 60; });
    cout << "Received worker 60% finished\n";

    progress_80.wait(mlock, [&app](){ return app.check_progress() >= 80; });
    cout << "Received worker 80% finished\n";

    progress_100.wait(mlock, [&app](){ return app.check_progress() >= 100; });
    cout << "Received worker 100% finished\n";

    cout << "Worker task finished!";

}