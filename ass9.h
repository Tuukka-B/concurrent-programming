//
// Created by tuukka on 14.10.2021.
//

#ifndef CONCURRENT_PROGRAMMING_ASS9_H
#define CONCURRENT_PROGRAMMING_ASS9_H

#endif //CONCURRENT_PROGRAMMING_ASS9_H

#include <iostream>
#include <thread>
#include <utility>

using namespace std;
using FuncEnqueue = auto (void) -> void;


/** Teacher's Instructions for Assignment 9:
 *
 * 4 people are at the airport. To prepare for departure, each of them has to first scan their boarding pass (which takes 1 min), and then to do the security check (which takes 10 minutes).
 *  1. Assume that there is only one machine for scanning the boarding pass and only one security line. Explain why this pipeline is unbalanced. Compute its throughput.
 *  2. Now assume that there are 2 security lines. Which is the new throughput?
 *  3. If there were 4 security lines opened, would the pipeline be balanced?
 * Implement a test program to verify your findings.
 * Tip: You can simulate the throughput faster than 1/10 minutes. But after simulation, show the results based on original 1/10 minutes.
 */

class AirportLine {
public:
    AirportLine(string line, int init_queue_n, int thread_n, int wait_multiplier){
        line_name = line;
        queue = init_queue_n;
        barrier = vector<bool>(thread_n, false);
        threads = thread_n;
        callback = [](){ return; };
        wait_mul = wait_multiplier;
        }

    AirportLine(string line_name, int init_queue_n, int thread_n, int wait_multiplier, function<FuncEnqueue> callback_func){
        queue = init_queue_n;
        barrier = vector<bool>(thread_n, false);
        threads = thread_n;
        callback = std::move(callback_func);
        wait_mul = wait_multiplier;
    }

    void start(int n_of_thread){
        while (queue > 0) {
            this_thread::sleep_for(chrono::seconds(60 * wait_mul ));
            //cout << "Customer has finished their security check! Customers left: " << queue.size() - 1 << "\n";
            while (check_barrier(n_of_thread)) { this_thread::sleep_for(chrono::nanoseconds(1)); }
            queue += - 1;
            callback();
            release_barrier(n_of_thread);
            //if (queue.empty()) cout << "Security check line is now empty\n";
        }
        this_thread::sleep_for(chrono::seconds(1));
        if (!done) this->start(n_of_thread);
    }

    void enqueue(){
        queue += 1;
        //cout << "New customer has entered " << line_name <<"! Customers in the queue: " <<  queue  << "\n";
    }

    bool has_queue(){
        return queue > 0;
    }

    void stop() {
        done = true;
    }



private:

    bool check_barrier(int n){
        if (threads == 1) return false;
        if (n > 0) return !barrier[n-1];
        return barrier[0];
    }

    void release_barrier(int n){
        barrier[n] = true;

        /*
        cout << "barrier situation: ";
        for (int i = 0; i < barrier.size(); ++i) {
            cout << "[" << i <<"]: "  << barrier[i] << ", ";
        }
        cout << "\n";
        */
        bool reset = false;
        for (int i = 0; i < barrier.size(); ++i) {
            if (!barrier[i]) break;
            if (i == barrier.size() -1 && barrier[i]) reset = true;
        }
        if (reset) {
            barrier = vector<bool>(threads, false);
        }
    }

    string line_name;
    bool done = false;
    atomic<int> queue;
    vector<bool> barrier;
    int threads;
    function<FuncEnqueue> callback;
    int wait_mul;
};

void ass8_worker(int thread_n){
    this_thread::sleep_for(chrono::milliseconds(thread_n));
    AirportLine security_check_line("Security Check", 0, thread_n, 10);

    auto enqueue_security_check = [&security_check_line](){security_check_line.enqueue();};
    AirportLine boarding_pass_line("Boarding Pass Scan", 4, 1, 1, enqueue_security_check);

    cout << "Start of the situation (" << thread_n << " security lines): 4 people in the boarding pass scan line, 0 people in the security check line\n";
    auto start = chrono::high_resolution_clock::now();
    thread boarding_thread = thread([&boarding_pass_line](){boarding_pass_line.start(1);});

    vector<thread> workers;

    workers.reserve(thread_n);
    for (int i = 0; i < thread_n; ++i) {
        workers.emplace_back(thread([&security_check_line](int i){security_check_line.start(i);}, i));
    }

    while (security_check_line.has_queue() || boarding_pass_line.has_queue()) { this_thread::sleep_for(chrono::seconds(1)); }
    security_check_line.stop();
    boarding_pass_line.stop();
    boarding_thread.join();
    for (thread& t : workers) {
        t.join();
    }
    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::seconds>(end - start);

    cout << "All finished! time taken (" << thread_n << " security lines): " << duration.count() / 60 << " minutes " << duration.count() % 60 <<  " seconds \n";
}

void ass9_main(){

    thread worker1 = thread([](){ ass8_worker(4); });
    thread worker2 = thread([](){ ass8_worker(2); });
    thread worker3 = thread([](){ ass8_worker(1); });

    worker1.join();
    worker2.join();
    worker3.join();



    /**

        OUTPUT:
            > Start of the situation (1 security lines): 4 people in the boarding pass scan line, 0 people in the security check line
            > Start of the situation (2 security lines): 4 people in the boarding pass scan line, 0 people in the security check line
            > Start of the situation (4 security lines): 4 people in the boarding pass scan line, 0 people in the security check line
            > All finished! time taken (4 security lines): 14 minutes 1 seconds
            > All finished! time taken (2 security lines): 22 minutes 1 seconds
            > All finished! time taken (1 security lines): 41 minutes 1 seconds
     */


}