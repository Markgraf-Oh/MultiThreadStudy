// ThreadTest.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <thread>
#include <random>
#include <future>
#include <mutex>
#include <vector>
#include <iomanip>
#include <map>
#include <string>

/**thread와 promise, future을 이용해 uniform distribution을 구합니다.
* achive uniform distribution by using thread, promise, future.
*/
void test1();

/**thread와 promise, future, mutex를 이용해 각 스레드의 연산결과를 단일 변수에 저장한다.
* by using thread, promise, future, mutex, saves each threads calculated result in the same variable.
*/
void test2();

//testing std::async
void test3();

void GetUniformDistribution(int thread_id, unsigned int max_number, unsigned int refeat_time,  std::promise<std::vector<double>> promise, std::mutex &cout_mutex);

std::vector<double> GetUniformDistribution2(int thread_id, unsigned int max_number, unsigned int refeat_time, std::mutex &cout_mutex);

void GetNomalDistribution(int thread_id, float mean, float distribution, unsigned int min_refeat_time, unsigned int max_refeat_time, std::map<float, double>& result_histogram, std::mutex & result_mutex);

int main()
{
    test3();
}

void test1()
{
    std::mutex cout_mutex;
    const unsigned int number_of_threads = 10;
    const unsigned int max_number = 100;
    const unsigned int refeat_time = 100;

    std::vector< std::promise<std::vector<double>>> promises;
    /*
    * std::vector< std::promise<std::vector<int>>> promises(number_of_threads, std::promise<std::vector<int>>());
    * 위와 같은 promise의 vector의 초기화는 불가능하다. 일일이 emplace_back, 또는 push_back 해줘야.
    */
    
    /*
    * std::promise, std::future, std::thread에는 iterator가 명시적으로 작제되어 있다. 따라서 range-based for문 등이나 vector의 초기화 등이 불가능하다.
    */
    
    std::vector< std::future<std::vector<double>> > futures;
    std::vector< std::thread> threads;
    
    for(int i = 0; i < number_of_threads; i++)
    {
        //promises.push_back(std::promise<std::vector<int>>());
        promises.emplace_back();
        futures.push_back(promises[i].get_future());
        threads.emplace_back(GetUniformDistribution, i, max_number, refeat_time, std::move(promises[i]), std::ref(cout_mutex));
    }
    

    for(int i = 0; i < number_of_threads; i++)
    {
        threads[i].join();
    }

    std::vector<double> total(max_number + 1, 0.0);
    for(int i = 0; i < number_of_threads; i ++)
    {
        std::vector<double> thread_result = futures[i].get();
        for(int j = 0; j < max_number + 1; j++)
        {
            total[j] += thread_result[j];
        }        
    }
    for(int j = 0; j < max_number + 1; j++)
    {
        total[j] /= number_of_threads;
    }

    std::cout << std::string(30, '*') << std::endl;

    double sum = 0.0;
    for(int i = 0; i < max_number + 1; i++)
    {
        sum += total[i];
        std::cout << std::setw(7) << total[i] << ' '<< std::string(int(total[i]/0.001), '*')<< "\n";
    }
    std::cout << "sum of all distribution : " << sum << std::endl;
}

void test2()
{
    std::mutex map_mutex;
    const unsigned int number_of_threads = 10;
    const float mean = 0.0f;
    const float distribution = 1.0f;
    const unsigned int max_refeat_time = 1000;
    const unsigned int min_refeat_time = 500;

    std::map<float, double> histogram{};

    std::vector< std::thread> threads;
    for(int i = 0; i < number_of_threads; i++)
    {
        threads.emplace_back(GetNomalDistribution, i, mean, distribution, min_refeat_time, max_refeat_time, std::ref(histogram), std::ref(map_mutex));
    }

    for(int i = 0; i < number_of_threads; i++)
    {
        threads[i].join();
    }

    for(const std::pair<float, double>& pair : histogram)
    {
        std::cout << std::setw(8) << pair.first << ' ' << std::setw(8) << pair.second << ' ' << std::string(int(pair.second*50), '*') << std::endl;
    }
}


void test3()
{
    std::mutex cout_mutex;
    const unsigned int number_of_threads = 10;
    const unsigned int max_number = 100;
    const unsigned int refeat_time = 100;

    std::vector< std::future<std::vector<double>> > futures;

    for(int i = 0; i < number_of_threads; i++)
    {
        futures.emplace_back(std::async(std::launch::async, GetUniformDistribution2, i, max_number, refeat_time, std::ref(cout_mutex)));
    }

    std::vector<double> total(max_number + 1, 0.0);
    for(int i = 0; i < number_of_threads; i++)
    {
        std::vector<double> thread_result = futures[i].get();
        for(int j = 0; j < max_number + 1; j++)
        {
            total[j] += thread_result[j];
        }
    }
    for(int j = 0; j < max_number + 1; j++)
    {
        total[j] /= number_of_threads;
    }

    std::cout << std::string(30, '*') << std::endl;

    double sum = 0.0;
    for(int i = 0; i < max_number + 1; i++)
    {
        sum += total[i];
        std::cout << std::setw(7) << total[i] << ' ' << std::string(int(total[i] / 0.001), '*') << "\n";
    }
    std::cout << "sum of all distribution : " << sum << std::endl;
}

void GetUniformDistribution(int thread_id, unsigned int max_number, unsigned int refeat_time, std::promise<std::vector<double>> promise, std::mutex &cout_mutex)
{
    cout_mutex.lock();
    std::cout << "thread " << thread_id << " started\n";
    cout_mutex.unlock();
    std::random_device rande;
    std::minstd_rand generator_basic(rande());
    std::uniform_int_distribution<int> dist_uniform_int(0, max_number);
    std::vector<double> result(max_number + 1, 0.0);
    for(int i = 0; i < refeat_time; i++)
    {
        result[dist_uniform_int(generator_basic)] += 1.0;
    }
    for(int i = 0; i < max_number+1; i++)
    {
        result[i] /= (double)refeat_time;
    }
    promise.set_value(result);
    cout_mutex.lock();
    std::cout << "thread " << thread_id << " completed\n";
    cout_mutex.unlock();
}


std::vector<double> GetUniformDistribution2(int thread_id, unsigned int max_number, unsigned int refeat_time, std::mutex & cout_mutex)
{
    cout_mutex.lock();
    std::cout << "thread " << thread_id << " started\n";
    cout_mutex.unlock();
    std::random_device rande;
    std::minstd_rand generator_basic(rande());
    std::uniform_int_distribution<int> dist_uniform_int(0, max_number);
    std::vector<double> result(max_number + 1, 0.0);
    for(int i = 0; i < refeat_time; i++)
    {
        result[dist_uniform_int(generator_basic)] += 1.0;
    }
    for(int i = 0; i < max_number + 1; i++)
    {
        result[i] /= (double)refeat_time;
    }
    cout_mutex.lock();
    std::cout << "thread " << thread_id << " completed\n";
    cout_mutex.unlock();
    return result;
}

void GetNomalDistribution(int thread_id, float mean, float distribution, unsigned int min_refeat_time, unsigned int max_refeat_time, std::map<float, double>& result_histogram, std::mutex & result_mutex)
{
    std::random_device rande;
    std::minstd_rand generator(rande());
    //std::mt19937 generator(rande());
    std::normal_distribution<float> dist_normal(mean, distribution);
    std::map<float, int> histogram{};
    std::uniform_int_distribution<int> dist_uniform_int(min_refeat_time, max_refeat_time);

    int refeat_time = dist_uniform_int(generator);

    for(int i = 0; i < refeat_time; i++)
    {
        ++histogram[std::round(dist_normal(generator) * 10.0f) / 10.0f];
    }

    result_mutex.lock();
    for(std::pair<float, int> pair : histogram)
    {
        result_histogram[pair.first] += double(pair.second)/refeat_time;
    }
    result_mutex.unlock();
}

