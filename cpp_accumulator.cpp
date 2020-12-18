#include "interface.h"
#include "num_threads.h"
#include <thread>
#include <memory>
#include <vector>

double cpp_accumulate_false_sharing(const double *V, size_t count) {
    double full_sum = 0;
    unsigned T = get_num_threads();
    std::vector<std::thread> threads;
    std::vector<double> partial_sums(T);
    for (std::size_t t = 0; t < T; t++)
        threads.emplace_back(std::thread([T, t, V, count, &partial_sums]() {
            for (size_t i = t; i < count; i += T)
                partial_sums[t] += V[i];
        }));
    for (auto &thread:threads)
        thread.join();
    for (int i = 0; i < T; ++i)
        full_sum += partial_sums[i];
    return full_sum;
}

#ifdef _MSC_VER
constexpr std::size_t CACHE_LINE = std::hardware_destructive_interference_size;
#else
#define CACHE_LINE 64
#endif
struct aligned_double {
    alignas(CACHE_LINE) double value;
};

double cpp_accumulate_aligned(const double *V, size_t count) {
    double full_sum = 0;
    unsigned T = get_num_threads();
    std::vector<aligned_double> partial_sums(T);
    std::vector<std::thread> threads;
    for (std::size_t t = 0; t < T; ++t)
        threads.emplace_back([t, T, V, count, &partial_sums]() {
            for (size_t i = t; i < count; i += T)
                partial_sums[t].value += V[i];
        });
    for (auto &thread:threads)
        thread.join();
    for (int i = 0; i < T; ++i)
        full_sum += partial_sums[i].value;
    return full_sum;
}

#include <atomic>

double cpp_accumulate_atomic(const double *V, size_t count) {
    std::atomic<double> sum = 0;
    std::vector<std::thread> threads;
    unsigned T = get_num_threads();
    for (std::size_t t = 0; t < T; ++t)
        threads.emplace_back([t, T, V, count, &sum]() {
            for (size_t i = t; i < count; i += T)
                sum = sum + V[i];
        });
    for (auto &thread:threads)
        thread.join();
    return sum;
}

#include <mutex>
#include <cmath>

double cpp_accumulate_mutex(const double *V, size_t count) {
    double sum = 0;
    std::mutex mtx;
    std::vector<std::thread> threads;
    unsigned T = get_num_threads();
    for (std::size_t t = 0; t < T; ++t)
        threads.emplace_back([t, T, V, count, &sum, &mtx]() {
            double local_sum = 0;
            for (size_t i = t; i < count; i += T)
                local_sum += V[i];
            {
                std::lock_guard lock(mtx); //std::scoped_lock
                sum += local_sum;
            }
        });
    for (auto &thread:threads)
        thread.join();
    return sum;
}

/* C++20
MS Visual Studio 2019 (cl 19.28), /c++:latest
#include <latch>   std::latch
#include <barrier> std::barrier <---

{
	//std::latch bar{T};
	std::barrier bar{T};
	
	bar.arrive_and_wait();
}
#include <barrier>
*/

#include <mutex>
#include <condition_variable>

class barrier
{
    std::mutex mtx;
    std::condition_variable cv;
    const unsigned initial_count;
    unsigned count;
    unsigned use_count = 0;
public:
    explicit barrier(unsigned threads) :
        initial_count(threads),
        count(threads) {}

    void arrive_and_wait() {
        std::unique_lock lck{ mtx };

        if (--count == 0) {
            count = initial_count;
            cv.notify_all();
            use_count++;
        }
        else {
            auto current = use_count;
            do
            {
                cv.wait(lck);
            } while (use_count==current);
        }
    }
};

double cpp_accumulate_reduction_static(const double *V, size_t count) {
    double full_sum = 0;
    unsigned T = get_num_threads();
    std::vector<std::thread> threads;
    std::vector<double> partial_sums(T);
	barrier bar{T};
    for (std::size_t t = 0; t < T; t++)
        threads.emplace_back(std::thread([T, t, V, count, &partial_sums, &bar]() {
            double sub = 0;
            for (size_t i = t; i < count; i += T) {
                sub += V[i];
            }
            partial_sums[t] = sub;
            unsigned S = 1;
            while (S < T) {
                bar.arrive_and_wait();
                if ((t % (S * 2)) == 0 && t + S < T) {
                    partial_sums[t] = partial_sums[t] + partial_sums[t + S];
                }
                S *= 2;
            }
        }));


    /*waiting threads*/
    for (auto &thread:threads)
        thread.join();


    return partial_sums[0];
}
