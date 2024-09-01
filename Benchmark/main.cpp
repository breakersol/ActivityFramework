#include <benchmark/benchmark.h>

#include "Components/TA_Serialization.h"

static void BM_Serialization(benchmark::State &state)
{
    // Data preparation
    std::vector<int> vectorData(100000, 42);
    std::list<int> listData(100000, 42);
    std::map<int, double> mapData;
    for (int i = 0; i < 100000; ++i)
    {
        mapData[i] = i * 0.5;
    }

    // Benchmark serialization
    {
        CoreAsync::TA_Serializer output("./test.afw");
        for (auto _ : state) {
            output << vectorData << listData << mapData;
            output.flush();
            output.close();
        }
    }
}
BENCHMARK(BM_Serialization)->Iterations(1);

static void BM_Deserialization(benchmark::State& state)
{
    // Data preparation
    std::vector<int> vec;
    std::list<int> lt;
    std::map<int, double> md;

    // Benchmark deserialization
    {
        CoreAsync::TA_Serializer<CoreAsync::BufferReader> input("./test.afw");
        for (auto _ : state) {
            input >> vec >> lt >> md;
            input.close();

            // To prevent the compiler from optimizing out the whole loop
            benchmark::DoNotOptimize(vec);
            benchmark::DoNotOptimize(lt);
            benchmark::DoNotOptimize(md);
        }
    }
}
BENCHMARK(BM_Deserialization)->Iterations(1);

BENCHMARK_MAIN();
