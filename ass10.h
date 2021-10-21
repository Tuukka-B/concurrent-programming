//
// Created by tuukka on 17.10.2021.
//

/**
 * Teacher's Instructions for assignment 10:
 *
 * Take a look at the following program. It specifies pixel data as red, green and blue components,
 * creates two images from them and uses addPixelColors function to produce a third image.
 *
 * Modify this program to execute in parallel for the performance gain. Measure and note the gains in your source code.
 */

#ifndef CONCURRENT_PROGRAMMING_ASS10_H
#define CONCURRENT_PROGRAMMING_ASS10_H

#endif //CONCURRENT_PROGRAMMING_ASS10_H

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <cmath>
#include <condition_variable>
#include <mutex>
#include <deque>
#include <future>

using namespace std;



struct Pixel
{
    float red;
    float green;
    float blue;
};

Pixel* createPixelsMod(int imageSize)
{
    Pixel* image = new Pixel[imageSize];
    for (int i = 0; i < imageSize; i++)
    {
        image[i].red = (float(rand())/float((RAND_MAX)));
        image[i].green = (float(rand())/float((RAND_MAX)));
        image[i].blue = (float(rand())/float((RAND_MAX)));
    }
    return image;
}

void addPixelColorsMod(const Pixel* image1, const Pixel* image2, Pixel* result, int start, int end)
{
    /*
    this_thread::sleep_for(chrono::milliseconds(1));
    cout << "start " << start <<", end " << end << endl;
    */
    for (int i = start; i < end; i++)
    {
        result[i].red = image1[i].red + image2[i].red;
        if (result[i].red > 1.0f)
        {
            result[i].red = 1.0f;
        }

        result[i].green = image1[i].green + image2[i].green;
        if (result[i].green > 1.0f)
        {
            result[i].green = 1.0f;
        }

        result[i].blue = image1[i].blue + image2[i].blue;
        if (result[i].blue > 1.0f)
        {
            result[i].blue = 1.0f;
        }
    }
}

void addPixelColors(const Pixel* image1, const Pixel* image2, Pixel* result, int imageSize)
{
    for (int i = 0; i < imageSize; i++)
    {
        result[i].red = image1[i].red + image2[i].red;
        if (result[i].red > 1.0f)
        {
            result[i].red = 1.0f;
        }

        result[i].green = image1[i].green + image2[i].green;
        if (result[i].green > 1.0f)
        {
            result[i].green = 1.0f;
        }

        result[i].blue = image1[i].blue + image2[i].blue;
        if (result[i].blue > 1.0f)
        {
            result[i].blue = 1.0f;
        }
    }
}


Pixel* createPixels(int imageSize)
{
    Pixel* image = new Pixel[imageSize];
    for (int i = 0; i < imageSize; i++)
    {
        image[i].red = (float(rand())/float((RAND_MAX)));
        image[i].green = (float(rand())/float((RAND_MAX)));
        image[i].blue = (float(rand())/float((RAND_MAX)));
    }
    return image;
}


int ass10_main()
{
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    constexpr int imageSize = 4096 * 4096;
    Pixel* image1 = createPixels(imageSize);
    Pixel* image2 = createPixels(imageSize);
    Pixel* result = new Pixel[imageSize];

    addPixelColors(image1, image2, result, imageSize);

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    cout << "Execution time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]\n";

    delete[] result;
    delete[] image2;
    delete[] image1;

    chrono::steady_clock::time_point begin_c = chrono::steady_clock::now();
    /*
    Pixel* image1_c = createPixels(imageSize);
    Pixel* image2_c = createPixels(imageSize);
    */
    auto worker1 = async(launch::async, [](){
        return createPixelsMod(imageSize);
    });
    auto worker2 = async(launch::async, [](){
        return createPixelsMod(imageSize);
    });
    auto image1_c = worker1.get();
    auto image2_c = worker2.get();



    auto* result_c = new Pixel[imageSize];
    const int thread_n = thread::hardware_concurrency(); // -1 since we use main thread also
    vector<thread> workers(0);

    int blocks = imageSize / (thread_n + 1);
    int last_block_size = imageSize - blocks * (thread_n - 1);
    deque<bool> condition_clauses(thread_n -1, false);
    //int count = 0;
    for (int i = 0; i < thread_n -1; ++i) {
        //count ++;
        workers.emplace_back(thread([&image1_c, &image2_c, &result_c, blocks, i](){
            //this_thread::sleep_for(chrono::milliseconds(i));
            addPixelColorsMod(image1_c, image2_c, result_c, blocks * i, blocks * i + blocks);}));
    }
    for (thread& t : workers) {
        t.join();
    }
    //addPixelColorsMod(image1_c, image2_c, result_c, imageSize - last_block_size, imageSize, );
    //cout << "count: " << count << endl;
    /*
    */
    /*
    int count = 0;
    for (condition_variable& c : conditions) {
        c.wait(mlock, [count, &condition_clauses](){ return condition_clauses[count]; });
        count++;
    }
     */

    chrono::steady_clock::time_point end_c = chrono::steady_clock::now();
    cout << "Execution time (concurrently): " << chrono::duration_cast<chrono::milliseconds>(end_c - begin_c).count() << "[ms]\n";

    //cout << "check:  all block are equal to imgsize: " << imageSize -  blocks * (thread_n) + ( imageSize - blocks * thread_n ) << endl;

    /*
    cout << last_block_size  << endl;
    cout << blocks  << endl;
    cout << blocks * (thread_n - 1) + last_block_size << endl;
    cout << imageSize << endl;
    */
    delete[] result_c;
    delete[] image2_c;
    delete[] image1_c;

}
