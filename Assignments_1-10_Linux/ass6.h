//
// Created by tuukka on 9.10.2021.
//

#ifndef CONCURRENT_PROGRAMMING_ASS6_H
#define CONCURRENT_PROGRAMMING_ASS6_H

#endif //CONCURRENT_PROGRAMMING_ASS6_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <random>

using namespace std;

atomic_flag consumerLock;

class Ass7BankingContainer
{
public:
    Ass7BankingContainer(string name,int initial_balance)
    {
        account_holder = name;
        money_amount = initial_balance;
        counter = 0;
        interest_counter = 0;
    }

    void deposit(double amount) {
        money_amount.store(this->precise_calc(amount, "add"));
        this->post_operation();

    }

    int withdraw(double amount) {
        if (amount > money_amount.load()) { amount = this->money_amount.load(); }
        money_amount.store(this->precise_calc(amount, "substract"));
        this->post_operation();
        return amount;

    }

    void balance() {
        cout << "Balance for " << account_holder << ": " << money_amount.load() << "\n";

    }
    double balance_amount(){
        return money_amount;

    }

    void interest () {
        cout << "Interest has been paid " << interest_counter << " times for " << account_holder << "\n";
    }

    void print_counter() {
        cout << account_holder << "'s counter is at " << counter << "\n";
    }

private:
    // this method does not do accurate calculations due to it using doubles
    // more info: https://stackoverflow.com/questions/149033/best-way-to-store-currency-values-in-c
    void addInterest(){
        money_amount.store(money_amount.load() * 1.05);
    }

    void post_operation() {
        counter++;
        if (counter == 100) {
            interest_counter += 1;
            counter = 0;
            this->addInterest();
        }
    }

    // attribution: https://forums.codeguru.com/showthread.php?100612-How-to-handle-Imprecise-double-calculations&s=b15fd184ada9f4e863fd42c7d2c460ab&p=260919#post260919
    double precise_calc(double amount, const string operation) {

        long precise_val = (long)(amount * 100);
        if (operation == "substract") return (double)((long)(money_amount.load() * 100 - precise_val) / 100);
        if (operation == "add") return (double)((long)(money_amount.load() * 100 + precise_val) / 100);

    }
    atomic<int> interest_counter;
    atomic<int> counter;
    string account_holder;
    atomic<double>  money_amount;
    mutex m; // delete this

};

// attribution: https://www.geeksforgeeks.org/rounding-floating-point-number-two-decimal-places-c-c/
double round(double random_num){
    float rounded_int = (int)( random_num * 100 + .5);
    return  (double)rounded_int / 100;

}

void choose_operation(Ass7BankingContainer& account1, Ass7BankingContainer& account2, double random_num){
    // choose a "random" operation

     if (int(random_num * 10) % 3 == 0){
         // true_amount is the maximum amount one can withdraw from their account
         // account's balance can never be negative
        double true_amount =  account1.withdraw(round(random_num));
        account2.deposit(round(true_amount));
    }
     else if (int(random_num * 10) % 2 == 0){
         double true_amount =  account2.withdraw(round(random_num));
        account1.deposit(round(true_amount));

    }
     else if (int(random_num * 10) % 7 == 0) {
        // lucky number 7
         double true_amount =  account2.withdraw(round(random_num * 1.77));
        account1.deposit(round(true_amount));
    }
     else {
        choose_operation(account2, account1, random_num + 0.1);
    }
}

void ass7_random_worker(Ass7BankingContainer& account1,Ass7BankingContainer& account2) {


    int i = 0;
    while (i < 500) {

        // attribution (spinning barrier): https://stackoverflow.com/questions/8115267/writing-a-spinning-thread-barrier-using-c11-atomics#answer-8115400
        while (consumerLock.test_and_set()) { /* spin */ }

        // attribution (random number generator): https://stackoverflow.com/questions/19665818/generate-random-numbers-using-c11-random-library
        random_device rd;
        mt19937 mt(rd());
        uniform_real_distribution<double> dist(1.0, 100.0);
        double random_num = dist(mt);
        // "coin flip": odd or even
        if (i % 2 == 0) {
            choose_operation(account1, account2, random_num);
        } else {
            choose_operation(account2, account1, random_num);
        }

        i++;

        consumerLock.clear();
    }

}


int ass6_main() {
    int initial_balance = 4000;
    Ass7BankingContainer alices_account("Alice", initial_balance);
    Ass7BankingContainer bobs_account("Bob", initial_balance);

    thread worker1 = thread(ass7_random_worker, ref(alices_account), ref(bobs_account));
    thread worker2 = thread(ass7_random_worker, ref(alices_account), ref(bobs_account));
    thread worker3 = thread(ass7_random_worker, ref(alices_account), ref(bobs_account));
    thread worker4 = thread(ass7_random_worker, ref(alices_account), ref(bobs_account));

    worker1.join();
    worker2.join();
    worker3.join();
    worker4.join();

    cout << "\n*** END ***\n\n";
    cout << "Final balances:\n";
    alices_account.balance();
    bobs_account.balance();

    alices_account.interest();
    bobs_account.interest();

    // below does not always print the correct value because double addition is an imprecise operation
    // (for reference, I get the same result using mutexes, so it's not a problem with the barrier or my concurrency implementation)
    cout <<
    "Total amount of money on both accounts (should be " <<
    initial_balance * 2 * (1 + 0.05 * 20) <<
    " after taking interest into account): " <<
    alices_account.balance_amount() + bobs_account.balance_amount();
}