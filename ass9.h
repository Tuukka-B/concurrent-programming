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
        queue = vector<int>(init_queue_n, -1);
        customer_release_barrier = vector<bool>(thread_n, false);
        customer_reservation_barrier = vector<bool>(thread_n, false);
        threads = thread_n;
        wait_mul = wait_multiplier;
        callback = [](){ return; };
        }

    AirportLine(string line, int init_queue_n, int thread_n, int wait_multiplier, function<FuncEnqueue> callback_func){
        line_name = line;
        queue = vector<int>(init_queue_n, -1);
        customer_release_barrier = vector<bool>(thread_n, false);
        customer_reservation_barrier = vector<bool>(thread_n, false);
        threads = thread_n;
        wait_mul = wait_multiplier;
        callback = std::move(callback_func);
    }

    void start(int n_of_thread){
        while (!queue.empty()) {
            // reserve customer
            while (check_barrier(customer_reservation_barrier, n_of_thread, "straight") && !done) this_thread::sleep_for(chrono::seconds(1));

            int result = find_index(n_of_thread);
            while (result == -1 && !done) {
                this_thread::sleep_for(chrono::milliseconds(1));
                result = find_index(n_of_thread);
            }
            release_barrier(customer_reservation_barrier, n_of_thread, "straight");
            if (done) break;

            // process customer
            int sleep_counter = 60 * wait_mul;
            while (sleep_counter > 0 && !queue.empty()) {
                this_thread::sleep_for(chrono::seconds(1 ));
                sleep_counter--;
            }

            if (queue.empty()) break;

            // release customer
            int index = check_index(n_of_thread);
            if (index != -1) queue.erase(begin(queue) + index);

            //cout << "Customer has finished their security check! Customers left: " << queue.size() - 1 << "\n";

            callback();

            //if (queue.empty()) cout << line_name << " line is now empty" << endl;
        }
        this_thread::sleep_for(chrono::seconds(1));
        if (!done) this->start(n_of_thread);
    }

    void enqueue(){
        queue.emplace_back(-1);
        //cout << "New customer has entered " << line_name <<"! Customers in the queue: " <<  queue.size() -1  << endl;
    }

    bool has_queue(){
        return !queue.empty();
    }

    void stop() {
        done = true;
    }

private:

    bool check_barrier(vector<bool> barrier, int n, string order){
        if (threads == 1) return false;
        if (order == "reverse") {
            if (n > 0) return barrier[n-1];
            return !barrier[0];
        }
        if (n > 0) return !barrier[n-1];
        return barrier[0];

    }
    int find_index(int n){
        for (int i = queue.size() - 1; i > -1 ; --i) {

            if (queue[i] == n) return i;
            if (queue[i] == -1) {
                queue[i] = n;
                return i;
            }
        }
        return -1;
    }

    int check_index(int n){
        for (int i = queue.size() - 1; i > -1 ; --i) {
            if (queue[i] == n) return i;
        }
        return -1;
    }


    void release_barrier(vector<bool>& barrier, int n, string order){
        if (threads == 1) return;
        barrier[n] = true;


        // DEBUG //
        /*
        cout << "barrier situation: ";
        for (int i = 0; i < barrier.size(); ++i) {
            cout << "[" << i <<"]: "  << barrier[i] << ", ";
        }
        cout << "\n";
        */
        bool reset = false;
        if (order == "reverse") {
            for (int i = barrier.size() - 1; i < -1 ; --i) {
                if (!barrier[i]) break;
                if (i == barrier.size() -1 && barrier[i]) reset = true;
            }
        } else {
            for (int i = 0; i < barrier.size(); ++i) {
                if (!barrier[i]) break;
                if (i == barrier.size() -1 && barrier[i]) {
                    reset = true;
                }
            }
        }
        if (reset) barrier = vector<bool>(threads, false);
    }

    string line_name;
    bool done = false;
    vector<int> queue;
    vector<bool> customer_release_barrier;
    vector<bool> customer_reservation_barrier;
    int threads;
    function<FuncEnqueue> callback;
    int wait_mul;
};

void ass8_worker(int thread_n){
    this_thread::sleep_for(chrono::milliseconds(thread_n));
    AirportLine security_check_line("Security Check", 0, thread_n, 10);

    auto enqueue_for_security_check = [&security_check_line](){security_check_line.enqueue();};
    AirportLine boarding_pass_line("Boarding Pass Scan", 8, 1, 1, enqueue_for_security_check);

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
        /*
        thread worker2 = thread([](){ ass8_worker(2); });
        thread worker3 = thread([](){ ass8_worker(1); });
        */
        worker1.join();
        /*
        worker2.join();
        worker3.join();
        */
    /**

        OUTPUT:
            > Start of the situation (1 security lines): 4 people in the boarding pass scan line, 0 people in the security check line
            > Start of the situation (2 security lines): 4 people in the boarding pass scan line, 0 people in the security check line
            > Start of the situation (4 security lines): 4 people in the boarding pass scan line, 0 people in the security check line
            > All finished! time taken (4 security lines): 14 minutes 2 seconds
            > All finished! time taken (2 security lines): 22 minutes 1 seconds
            > All finished! time taken (1 security lines): 41 minutes 1 seconds

        QUESTIONS:
            1. Assume that there is only one machine for scanning the boarding pass and only one security line. Explain
            why this pipeline is unbalanced. Compute its throughput.
                - the pipeline's throughput would be 1 customer per little over 10 minutes
                - this is unbalanced considering the amount of wait doubles by every new customer that is in the queue
                - also considering it takes 10x the time to get through the security scan line than the boarding pass
                  scan line, only 1 security scan line would be wildly unbalanced in any case.
                - it could be balanced if there were very little customers (1 or less in 10 minutes), but that would be
                  unrealistic in a real situation

            2. Now assume that there are 2 security lines. Which is the new throughput?
                - new throughput is 1 customer in 5.5 minutes (with 4 customers). With more customers, the average would
                  be closer to ~5 minutes per customer.

            3. If there were 4 security lines opened, would the pipeline be balanced?
                - depends on the amount of customers, but for this example, it would be balanced. The throughput for
                  4 security lines is 1 customer every 3.5 minutes for 4 customers. The average time would be closer to
                  ~2.5 minutes per customer if we'd have more customers.

        Comments:
            I wanted to try implementing a spinning barrier in this assignment, just to see if it worked. It worked out
            in the end but it took a lot of time to implement (it had many bugs in the beginning). I'm not sure if this
            program would work if we removed all the sleep() commands as it helps to ease contention. But it was fun,
            and I see implementing a thread that can access all of the class' variables does have it's benefits.

            By the way, this program uses 8 threads so it can simulate 1, 2 and 4 security line scenarios concurrently.

     */

}