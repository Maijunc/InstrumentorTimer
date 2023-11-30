#pragma once

#include <iostream>
#include <cmath>
#include <chrono>
#include <fstream>
#include <thread>
#include <mutex>

struct ProfileResult
{
    std::string Name;
    long long Start, End;
    uint32_t ThreadID;
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
    std::mutex m_Lock;
public:
    Instrumentor(Instrumentor&) = delete;

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
        std::lock_guard<std::mutex> lockGuard(m_Lock);

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
        m_OutputStream << "\"tid\":" << result.ThreadID << ",";
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
        // only create one instance while the Get Function was called the first time
        static Instrumentor* instance = new Instrumentor();
        return *instance;
    }
private:
    Instrumentor()
            :m_CurrentSession(nullptr), m_ProfileCount(0)
    {
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

        uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
        Instrumentor::Get().WriteProfile({m_Name, start, end, threadID});

        m_Stopped = true;
    }


private:
    const char* m_Name;
    std::chrono::time_point<std::chrono::steady_clock> m_startTimepoint;
    bool m_Stopped;
};

