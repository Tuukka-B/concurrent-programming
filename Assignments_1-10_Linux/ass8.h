//
// Created by tuukka on 13.10.2021.
//

#ifndef CONCURRENT_PROGRAMMING_ASS8_H
#define CONCURRENT_PROGRAMMING_ASS8_H

#endif //CONCURRENT_PROGRAMMING_ASS8_H

#include <iostream>
#include <thread>
#include <functional>
#include <random>
#include <algorithm>
using namespace std;


/**
 * Teacher's Instructions for assignment 8:
 *
 * 1. Create a new console app.
 * 2. Declare a class 'RandomNumberGenerator'. In class, specify enum of task type as LIGHT or HEAVY.
 *  - If RandomNumberGenerator is LIGHT, it will generate 100 random numbers.
 *  - If RandomNumberGenerator is HEAVY, it will generate 10 000 000 random numbers.
 * 3. Allocate 100 RandomNumberGenerators and set the task type to LIGHT or HEAVY randomly.
 * 4. Run the allocated RandomNumberGenerators as async tasks.
 *   - If generator is LIGHT, use deferred launching.
 *   - If generator is HEAVY, use async launching.
 * 5. Measure how long it takes to complete all generators.
 * 6. Modify your code to run all tasks with async launching.
 * 7. Measure how long it takes to complete all generators.
 * 8. Add comments into the source code of your findings. Which way is faster? Why?
 */

class RandomNumberGenerator {
public:
    RandomNumberGenerator(string state) {
        if (state != "LIGHT" and state != "HEAVY") {
            cout << "Error! State must be either LIGHT or HEAVY!";
            return;
        }
        generator_state = state;

        if (generator_state == "HEAVY") random_number_cap = 10000000;
        else random_number_cap = 100;

    }

    bool gen_numbers() {
        uniform_int_distribution<int> dist(0, numeric_limits<int>::max());
        int count = 0;
        while (count < random_number_cap) {
            random_device rd;
            mt19937 mt(rd());
            int random_num = dist(mt);
            count++;
        }
        return true;
    }
        int random_number_cap;
        string generator_state;
};

string ass8_choose_state(){
    uniform_int_distribution<int> divider_dist(1, 5);
    uniform_int_distribution<int> dist(0, numeric_limits<int>::max());
    random_device rd;
    mt19937 mt(rd());
    int random_num = dist(mt);

    int divider = divider_dist(mt);
    if (random_num % divider == 0) return "HEAVY";

    return "LIGHT";
}

void ass8_main(){


    vector<RandomNumberGenerator> generators(0, RandomNumberGenerator("LIGHT"));
    auto light_x = 0;
    auto heavy_x = 0;
    for (int i = 0; i < 100 ; ++i) {
        RandomNumberGenerator generator(ass8_choose_state());
        if (generator.generator_state == "HEAVY") heavy_x++;
        else light_x ++;
        generators.push_back(generator);

    }
    cout << "Beginning mixed round...\n";
    auto start = chrono::high_resolution_clock::now();
    vector<shared_future<bool>> results;
    for (auto & generator : generators) {
        future<bool> result;
        if (generator.generator_state == "LIGHT") results.emplace_back(async(launch::deferred, [&generator]() { return generator.gen_numbers(); }));
        results.emplace_back(async(launch::async, [&generator]() { return generator.gen_numbers(); }));
    }
    // get the resuls of the futures
    for_each(results.begin(), results.end(), [](const shared_future<bool> x){ x.get(); });
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(end - start);

    cout << "Beginning async round...\n";
    auto async_start = chrono::high_resolution_clock::now();
    vector<shared_future<bool>> async_results;
    // using the same logic in a for loop to mimize the differences between two methods
    for (auto & generator : generators) {
        shared_future<bool> result;
        if (generator.generator_state == "LIGHT") async_results.emplace_back(async(launch::async, [&generator]() { return generator.gen_numbers(); }));
        else async_results.emplace_back(async(launch::async, [&generator]() { return generator.gen_numbers(); }));
    }
    for_each(async_results.begin(), async_results.end(), [](const shared_future<bool>& x){ x.get(); });
    auto async_end = chrono::high_resolution_clock::now();
    auto async_duration = chrono::duration_cast<chrono::seconds>(async_end - async_start);

    cout << "Time done with mixed async/deferred: " << duration.count() / 60.0 << " minutes\n";
    cout << "Time done with async only: " << async_duration.count() / 60.0 << " minutes\n";
    cout << "HEAVY generators: " << heavy_x << "\nLIGHT generators : " << light_x << "\n";

    /*

     RESULTS (Console print):
         > Beginning mixed round...
         > Beginning async round...
         > Time done with mixed async/deferred: 33.7333 minutes
         > Time done with async only: 34.6333 minutes
         > HEAVY generators: 40
         > LIGHT generators : 60

     ANALYSIS:

        EDIT: I think my Linux installation is somehow bad with threads and that might explain the results.
        I had to do Assignment 10 with Windows, for example, because with Linux I couldn't get any speed gains.
        My original analysis is below, but I think it might be wrong.

        It seems like the time to complete both rounds was almost the same. For background, I did this with a Linux
        Machine with 12 logical CPUs (6 cores, I think). All the CPUs were at 100% until the program was finished.

        If I had to guess where the differences came to be, I'd say it comes from the cost of making a thread. Since
        only async::async policy creates "a real thread", and we used only that in the second round for the light
        generators, the cost of creating threads in the second run creates the overhead we see in the results.

        It was the heavy generators that made this program so slow. The light generators were complete near instantly.
        The heavy generators took ~ a minute or more to finish each. Since heavy generators were true async at both goes,
        that cannot explain the time difference. It must've been switching the light generators to true async, which is
        an operation that demands resources more than running it in sync in the same thread.

        Why this is so? I've read that creating a thread is a costly operation. Creating a thread for such
        a light-weight task like our light generator creates more costs than it has benefits. It makes more sense to
        create threads for heavy tasks, such as our heavy generators. I bet we could make the heavies async::deferred,
        and see that it takes _even longer_ to finish the program, since in that case the cost of creating a thread
        would be negligible to the cost of not using multiple threads for these kinds of calculations.

     */

}