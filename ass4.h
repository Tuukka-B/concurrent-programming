//
// Created by tuukka on 7.10.2021.
//

#ifndef CONCURRENT_PROGRAMMING_ASS4_H
#define CONCURRENT_PROGRAMMING_ASS4_H

#endif //CONCURRENT_PROGRAMMING_ASS4_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <random>
using namespace std;

/**
 * Teacher's Instructions for Assignment 4
 *
 * Create a class that simulates a bank account operations and holds the current balance of the account.
 * Add member functions for deposit, withdraw and balance.
 * Declare two bank account objects of aforementioned class and initialize with some balance.
 * Start 4 threads that will make random deposit and withdraw transactions between these two accounts.
 * Protect the bank account data with mutexes and avoid deadlocks.
 */

class BankingContainer
{
public:
    BankingContainer(string name,int initial_balance)
    {
        account_holder = name;
        money_amount = initial_balance;
    }

    void deposit(double amount) {
        if (m.try_lock()) {
            money_amount += amount;
            m.unlock();
        } else {
            this_thread::sleep_for(chrono::nanoseconds (1));
            this->deposit(amount);
        }


    }

    void withdraw(double amount) {
        if (m.try_lock()) {
            money_amount -= amount;
            m.unlock();
        } else {
            this_thread::sleep_for(chrono::nanoseconds (1));
            this->withdraw(amount);
        }
    }

    void balance() {
        m.lock();
        cout << "Balance for " << account_holder << ": " << money_amount << "\n";
        m.unlock();

    }
    string account_holder;
    double money_amount;
    mutex m;
};

void choose_operation(BankingContainer& account1, BankingContainer& account2, double random_num){
    // choose a "random" operation
    if (int(random_num * 10) % 4 == 0){
        account1.balance();
        account2.balance();
    }
    else if (int(random_num * 10) % 3 == 0){
        account1.withdraw(random_num);
        account2.deposit(random_num);
    }
    else if (int(random_num * 10) % 2 == 0){
        account2.withdraw(random_num);
        account1.deposit(random_num);

    }
    else if (int(random_num * 10) % 7 == 0) {
        // lucky number 7
        account2.withdraw(random_num * 1.77);
        account1.deposit(random_num * 1.77);
    }
    else {
        choose_operation(account2, account1, random_num + 0.1);
    }
}


void random_worker(BankingContainer& account1,BankingContainer& account2) {
    int i = 0;
    while (i < 500) {
        // https://stackoverflow.com/questions/19665818/generate-random-numbers-using-c11-random-library
        random_device rd;
        mt19937 mt(rd());
        uniform_real_distribution<double> dist(1.0, 100.0);
        double random_num = dist(mt);
        if (i % 2 == 0) {
            choose_operation(account1, account2, random_num);
        } else {
            choose_operation(account2, account1, random_num);
        }

        i++;
    }

}


int ass4_main() {
    BankingContainer alices_account("Alice",1000);
    BankingContainer bobs_account("Bob", 1000);

    thread worker1 = thread(random_worker, ref(alices_account), ref(bobs_account));
    thread worker2 = thread(random_worker, ref(alices_account), ref(bobs_account));
    thread worker3 = thread(random_worker, ref(alices_account), ref(bobs_account));
    thread worker4 = thread(random_worker, ref(alices_account), ref(bobs_account));

    worker1.join();
    worker2.join();
    worker3.join();
    worker4.join();

}