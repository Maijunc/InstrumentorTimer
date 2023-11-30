#include "InstrumentorMacro.h"
#include "InstrumentorTimer.h"

namespace Benchmark {

    void PrintFunction(int val) {
        PROFILE_FUNCTION();
        for (int i = 0; i < 10000; i++)
            std::cout << "Hello world #" << (i + val) << std::endl;
    }

    void PrintFunction() {
        PROFILE_FUNCTION();
        for (int i = 0; i < 10000; i++)
            std::cout << "Hello world #" << i << std::endl;
    }

    void Function1() {
        PROFILE_FUNCTION();
        for (int i = 0; i < 10000; i++)
            std::cout << "Hello world #" << i << std::endl;
    }

    void Function2() {
        PROFILE_FUNCTION();
        for (int i = 0; i < 10000; i++)
            std::cout << "Hello world #" << sqrt(i) << std::endl;
    }

    int Fibonacci(int x)
    {
        std::string name = std::string("Fib ") + std::to_string(x);
        PROFILE_SCOPE(name.c_str());

        std::this_thread::sleep_for(std::chrono::microseconds(1)); // Compensate statistical temporal precision problem

        if(x < 3)
            return 1;

        return Fibonacci(x - 1) + Fibonacci(x - 2);
    }

    void RunBenchmarks() {
        PROFILE_FUNCTION();

        std::cout << "Running Benchmarks...\n";
        std::thread a([](){Fibonacci(9); });
        std::thread b([](){Fibonacci(10); });

        a.join();
        b.join();
    }
}

int main() {

    Instrumentor::Get().BeginSession("Profile");
    Benchmark::RunBenchmarks() ;
    Instrumentor::Get().EndSession();

    return 0;
}