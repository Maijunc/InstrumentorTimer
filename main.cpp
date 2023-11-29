#include <iostream>
#include <cmath>
#include <chrono>
#include <fstream>

struct ProfileResult
{
    std::string Name;
    long long Start, End;
};

struct InstrumentationSession
{
    std::string Name;
};

//Convert the benchmark data into a JSON file
class Instrumentor
{
private:
    InstrumentationSession* m_CurrentSession;
    std::ofstream m_OutputStream;
    int m_ProfileCount;
public:
    Instrumentor()
        :m_CurrentSession(nullptr), m_ProfileCount(0)
    {
    }

    //Write the BeginSession for the json file
    void BeginSession(const std::string& name,const std::string& filepath = "result.json")
    {
        m_OutputStream.open(filepath);
        WriteHeader();
        m_CurrentSession = new InstrumentationSession{name};
    }

    //Write the EndSession for the json file
    void EndSession()
    {
        WriteFooter();
        m_OutputStream.close();
        delete m_CurrentSession;
        m_CurrentSession = nullptr;
        m_ProfileCount = 0;
    }

    void WriteProfile(const ProfileResult& result)
    {
        if(m_ProfileCount++ > 0)
            m_OutputStream << ",";

        std::string name = result.Name;
        std::replace(name.begin(),name.end(),'"', '\'');

        m_OutputStream << "{";
        m_OutputStream << "\"cat\":\"function\",";
        m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
        m_OutputStream << "\"name\":\"" << name << "\",";
        m_OutputStream << "\"ph\":\"X\",";
        m_OutputStream << "\"pid\":0,";
        m_OutputStream << "\"tid\":0,";
        m_OutputStream << "\"ts\":" << result.Start;
        m_OutputStream << "}";

        m_OutputStream.flush(); //Prevent data from disappearing due to refresh
    }

    void WriteHeader()
    {
        m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
        m_OutputStream.flush();
    }

    void WriteFooter()
    {
        m_OutputStream << "]}";
        m_OutputStream.flush();
    }

    static Instrumentor& Get()
    {
        static Instrumentor* instance = new Instrumentor();
        return *instance;
    }
};

class InstrumentationTimer
{
public:
    InstrumentationTimer(const char* name)
        : m_Name(name), m_Stopped(false)
    {
        m_startTimepoint = std::chrono::high_resolution_clock::now();  //get high_resolution clock
    }

    ~InstrumentationTimer()
    {
        if(!m_Stopped)
            Stop();
    }

    void Stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

        Instrumentor::Get().WriteProfile({m_Name, start, end});

        m_Stopped = true;
    }


private:
    const char* m_Name;
    std::chrono::time_point<std::chrono::steady_clock> m_startTimepoint;
    bool m_Stopped;
};

#define PROFILEING 1
#if PROFILEING
#define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__PRETTY_FUNCTION__)
#else
#define PROFILE_SCOPE(name)
#endif

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

    void RunBenchmarks() {
        PROFILE_FUNCTION();

        std::cout << "Running Benchmarks...\n";
        PrintFunction();
        PrintFunction(5);
    }
}

int main() {

    Instrumentor::Get().BeginSession("Profile");
    Benchmark::RunBenchmarks();
    Instrumentor::Get().EndSession();

    return 0;
}
