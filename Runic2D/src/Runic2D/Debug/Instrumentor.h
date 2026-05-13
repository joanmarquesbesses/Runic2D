#pragma once

#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>
#include <iomanip>
#include <sstream>

namespace Runic2D
{
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

    class Instrumentor
    {
    private:
        InstrumentationSession* m_CurrentSession;
        std::ofstream m_OutputStream;
        int m_ProfileCount;
        std::mutex m_Mutex;
    public:
        Instrumentor()
            : m_CurrentSession(nullptr), m_ProfileCount(0)
        {
        }

        void BeginSession(const std::string& name, std::string filepath = "")
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_CurrentSession)
            {
                InternalEndSession();
            }

            if (filepath.empty())
            {
                // Generem un nom de fitxer basat en el temps actual: Profiling_20240513_1800.json
                auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::stringstream ss;
                ss << "Profiling_" << std::put_time(std::localtime(&now), "%Y%m%d_%H%M%S") << ".json";
                filepath = ss.str();
            }

            m_OutputStream.open(filepath);
            WriteHeader();
            m_CurrentSession = new InstrumentationSession{ name };
        }

        void EndSession()
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            InternalEndSession();
        }

        void WriteProfile(const ProfileResult& result)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            
            if (!m_CurrentSession)
                return;

            if (m_ProfileCount++ > 0)
                m_OutputStream << ",";

            std::string name = result.Name;
            std::replace(name.begin(), name.end(), '"', '\'');

            m_OutputStream << "{";
            m_OutputStream << "\"cat\":\"function\",";
            m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
            m_OutputStream << "\"name\":\"" << name << "\",";
            m_OutputStream << "\"ph\":\"X\",";
            m_OutputStream << "\"pid\":0,";
            m_OutputStream << "\"tid\":" << result.ThreadID << ",";
            m_OutputStream << "\"ts\":" << result.Start;
            m_OutputStream << "}";
        }

        static Instrumentor& Get()
        {
            static Instrumentor instance;
            return instance;
        }

        bool IsSessionActive() const { return m_CurrentSession != nullptr; }

    private:
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

        void InternalEndSession()
        {
            if (m_CurrentSession)
            {
                WriteFooter();
                m_OutputStream.close();
                delete m_CurrentSession;
                m_CurrentSession = nullptr;
                m_ProfileCount = 0;
            }
        }
    };

    class InstrumentationTimer
    {
    public:
        InstrumentationTimer(const char* name)
            : m_Name(name), m_Stopped(false)
        {
            m_StartTimepoint = std::chrono::steady_clock::now();
        }

        ~InstrumentationTimer()
        {
            if (!m_Stopped)
                Stop();
        }

        void Stop()
        {
            if (!Instrumentor::Get().IsSessionActive())
                return;

            auto endTimepoint = std::chrono::steady_clock::now();

            long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

            uint32_t threadID = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
            Instrumentor::Get().WriteProfile({ m_Name, start, end, threadID });

            m_Stopped = true;
        }
    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
        bool m_Stopped;
    };
}

#ifndef R2D_DIST
    #define R2D_PROFILE_BEGIN_SESSION(name, filepath) ::Runic2D::Instrumentor::Get().BeginSession(name, filepath)
    #define R2D_PROFILE_END_SESSION() ::Runic2D::Instrumentor::Get().EndSession()
    #define R2D_PROFILE_SCOPE(name) ::Runic2D::InstrumentationTimer timer##__LINE__(name);
    #define R2D_PROFILE_FUNCTION() R2D_PROFILE_SCOPE(__FUNCSIG__)
#else       
    #define R2D_PROFILE_BEGIN_SESSION(name, filepath)
    #define R2D_PROFILE_END_SESSION()
    #define R2D_PROFILE_SCOPE(name)
    #define R2D_PROFILE_FUNCTION()
#endif
