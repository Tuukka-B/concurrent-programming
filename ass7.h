//
// Created by tuukka on 11.10.2021.
//

#ifndef CONCURRENT_PROGRAMMING_ASS7_H
#define CONCURRENT_PROGRAMMING_ASS7_H

#endif //CONCURRENT_PROGRAMMING_ASS7_H

#include <iostream>
#include <thread>
#include <functional>
#include <atomic>
#include <random>
#include <future>
#include <algorithm>

using namespace std;

/*
 * Teacher's instructions for assignment 7:
 *
 *  * 1. Create a new console app.
 *  * 2. Declare a class Sensor.
 *  *   - Sensor uses a thread internally to read sensor values.
 *  *   - Simulate sensor values by generating random numbers in between 0-4095
 *  * 3. Declare a class SensorReader.
 *  *   - SensorReader allocates 4 sensors and starts a thread to read sensor values.
 *  *   - SensorReader thread can be stopped by a function call to SensorReader::stop.
 *  * 4. Assume situation that reader is only interested of sensor values larger than 4000.
 *  *   - Implement future-promise; reader will wait for the result of sensor value over 4000.
 *  *   - When SensorReader gets a sensor value over 4000, print it to the console.
 *  * 5. On your main function, allocate a SensorReader and let it run until user presses a key in console.
 */

void sensor_worker(atomic<int>& reading, bool& stop) {

    std::uniform_int_distribution<int> dist(0, 4095);

    while (!stop) {
        std::random_device rd;
        std::mt19937 mt(rd());
        int random_num = dist(mt);

        // store variable to Sensor class instance
        reading.store(random_num);

        std::this_thread::sleep_for(std::chrono::milliseconds (100));
    }


}

class Sensor
{
public:
    Sensor()
    {
        reading.store(0);
        worker = std::thread([this]() { sensor_worker(reading, stop_thread); });
        stop_thread = false;
    }


    bool finish(){
        stop_thread = true;
        worker.join();
        return true;
    }

    bool stop_thread;
    std::thread worker;
    atomic<int> reading;

private:

};


int sensor_reader_worker(Sensor& sensor1, Sensor& sensor2, Sensor& sensor3, Sensor& sensor4) {

    while (true) {
        array<int, 4> readings = { sensor1.reading.load(), sensor2.reading.load(), sensor3.reading.load(), sensor4.reading.load() };
        std::this_thread::sleep_for(std::chrono::milliseconds (99));
        auto result = std::find_if(readings.cbegin(), readings.cend(), [](int reading) { return reading >= 4000; } );
        if (*result >= 4000 && *result <= 4095) {
            return *result;
        }

    }

}


class SensorReader
{
public:
    SensorReader()
    {
        w1_halted = false;
        w2_halted = false;
        halt_operations = false;

    }

    void stop(){
        halt_operations = true;
        worker2.detach();
        worker1.detach();

    }

    void start(){
        worker1 = std::thread([&]() {
            Sensor sensor1;
            Sensor sensor2;
            Sensor sensor3;
            Sensor sensor4;
            this->read_sensors(sensor1, sensor2, sensor3, sensor4);
        });

        worker2 = std::thread([&]() { this->read_thresholds(); });
    }

    bool is_halted() {
        return (w1_halted && w2_halted);
    }

private:

    // worker 1 method
    void read_sensors(Sensor& sensor1, Sensor& sensor2, Sensor& sensor3, Sensor& sensor4){

        int val = sensor_reader_worker(sensor1, sensor2, sensor3, sensor4);
        threshold_value.set_value(val);
        if (not halt_operations)  {
            is_received.get_future().get();
            is_received = promise<bool>();
        }

        if (not halt_operations) {
            this->read_sensors(sensor1, sensor2, sensor3, sensor4);
        } else {
            sensor1.finish();
            sensor2.finish();
            sensor3.finish();
            sensor4.finish();
            w1_halted = true;
        }
    }

    // worker 2 method
    void read_thresholds(){
        cout << threshold_value.get_future().get() << "\n";
        threshold_value = promise<int>();
        is_received.set_value(true);

        if (not halt_operations) {
            this->read_thresholds();
        } else {
            w2_halted = true;
        }
    }

    // promise for communicating values over 4000 ("threshold values") from Sensor
    promise<int> threshold_value;

    // promise for communicating that the threshold_value was received and can now be reinitialized
    promise<bool> is_received;

    // boolean to control whether threads should exit their loops
    bool halt_operations;

    thread worker1;
    thread worker2;

    // booleans for the threads to signal they have exited gracefully
    bool w1_halted;
    bool w2_halted;
};

int ass7_main() {

    SensorReader sensor;
    sensor.start();
    cout << "Press any key + enter to exit... \n";
    string exit;
    cin >> exit;
    cout << "Received exit command, shutting down processes...\n";
    sensor.stop();

    while (!sensor.is_halted()) { /* spin */ };

    cout << "Goodbye!\n";

}