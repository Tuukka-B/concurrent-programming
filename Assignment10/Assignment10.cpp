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

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <mutex>
#include <deque>

using namespace std;



struct Pixel
{
    float red;
    float green;
    float blue;
};

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

void addPixelColorsMod(const Pixel* image1, const Pixel* image2, Pixel* result, int start, int end)
{
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

Pixel* createPixels(int imageSize)
{
    Pixel* image = new Pixel[imageSize];
    for (int i = 0; i < imageSize; i++)
    {
        image[i].red = (float(rand()) / float((RAND_MAX)));
        image[i].green = (float(rand()) / float((RAND_MAX)));
        image[i].blue = (float(rand()) / float((RAND_MAX)));
    }
    return image;
}

void createPixelsMod(Pixel* image, int start, int end)
{
    for (int i = start; i < end; i++)
    {
        image[i].red = (float(rand()) / float((RAND_MAX)));
        image[i].green = (float(rand()) / float((RAND_MAX)));
        image[i].blue = (float(rand()) / float((RAND_MAX)));
    }
}

int main()
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
    auto* result_c = new Pixel[imageSize];
    auto* image1_c = new Pixel[imageSize];
    auto* image2_c = new Pixel[imageSize];

    const int thread_n = thread::hardware_concurrency() - 1;
    int blocks = imageSize / (thread_n + 1);
    int last_block_size = imageSize - blocks * (thread_n);

    auto create_pixels = [&image1_c, &image2_c]( int start, int end) {
        createPixelsMod(image1_c, start, end);
        createPixelsMod(image2_c, start, end);
    };

    // just testing out deque type of variable, it has no significance at all in this program
    deque<thread> pixel_create_workers(0);

    for (int i = 0; i < thread_n; ++i) {
        pixel_create_workers.emplace_back(thread( create_pixels, blocks * i, blocks * i + blocks));
    }

    // main thread works also on the last block
    createPixelsMod(image1_c, imageSize - last_block_size, imageSize);
    createPixelsMod(image2_c, imageSize - last_block_size, imageSize);

    for (thread& t : pixel_create_workers) {
        t.join();
    }

    vector<thread> workers(0);

    auto addColorFunc = [&image1_c, &image2_c, &result_c](int start, int end) {
        addPixelColorsMod(image1_c, image2_c, result_c, start, end);
    };

    for (int i = 0; i < thread_n; ++i) {
        workers.emplace_back(thread(addColorFunc, blocks * i, blocks * i + blocks));
    }

    // main thread works also on the last block
    addPixelColorsMod(image1_c, image2_c, result_c, imageSize - last_block_size, imageSize);

    for (thread& t : workers) {
        t.join();
    }

    chrono::steady_clock::time_point end_c = chrono::steady_clock::now();
    cout << "Execution time (concurrently): " << chrono::duration_cast<chrono::milliseconds>(end_c - begin_c).count() << "[ms]\n";

    /*
    bool whole_imagesize_is_used = imageSize == blocks * (thread_n) + last_block_size;
    cout << "check:  all block are equal to imgsize: " << whole_imagesize_is_used << endl;
    cout << last_block_size  << endl;
    cout << blocks  << endl;
    cout << blocks * (thread_n) + last_block_size << endl;
    cout << imageSize << endl;
    */

    delete[] result_c;
    delete[] image2_c;
    delete[] image1_c;

    /*
    * RESULTS:
    *    Execution time: 7170[ms]
    *    Execution time (concurrently): 1300[ms]
    *   - which is means that concurrent version did the same thing with only 18% of the time spent compared to non-concurrent version!
    */

}
