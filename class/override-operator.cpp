//
// Created by Steve on 7/24/2020.
//

#include <iostream>
#include <ostream>
#include <sstream>
#include <string.h>

struct A{
    A(unsigned a):a(a){}
    unsigned int operator&(unsigned int b){
        a&=b;
        return a;
    }
//    unsigned& operator->(){
//        return a;
//    }
    unsigned int a;
};

struct Client{
    int a;
};

struct Proxy{
    Client* target;
    Client* operator->() const{
        return target;
    }
};

struct Proxy2{
    Proxy* target;
    Proxy& operator->() const{
        return *target;
    }
};

namespace sun{
    typedef int LogSeverity;

    constexpr LogSeverity LOG_INFO=0;
    constexpr LogSeverity LOG_WARNING=1;
    constexpr LogSeverity LOG_ERROR=2;
    constexpr LogSeverity LOG_FATAL=3;
    constexpr LogSeverity LOG_NUM_SEVERITIES=4;

    const char *GetNameForLogSeverity(LogSeverity severity){
        switch (severity) {
            case LOG_INFO:
                return "INFO";
            case LOG_WARNING:
                return "WARNING";
            case LOG_ERROR:
                return "ERROR";
            case LOG_FATAL:
                return "FATAL";
            default:
                return "UNKNOWN";
        }
    }

    const char* StripDots(const char* p){
        while(strncmp(p,"../",3)==0){
            p+=3;
        }
        return p;
    }

    const char* StripPath(const char* p){
        auto*s=strrchr(p,'/');
        return s?s+1:p;
    }

    class LogMessageVoidify{
    public:
        void operator&(std::ostream&){}
    };

    class LogMessage{
    public:
        LogMessage(LogSeverity severity,
                   const char* file,
                   int line,
                   const char* condition)
                   :severity_(severity),file_(file),line_(line){
            stream_<<"[";
            if(severity>=LOG_INFO){
                stream_<<GetNameForLogSeverity(severity);
            }else{
                stream_<<"VERBOSE"<<-severity;
            }
            stream_<<":"<<(severity>LOG_INFO?StripDots(file):StripPath(file))
                   <<"("<<line<<")]";
            if(condition){
                stream_<<"Check failed: "<<condition<<". ";
            }
            std::cerr<<stream_.str();
            std::cerr.flush();
            if(severity>=LOG_FATAL){
                abort();
            }
        }
        ~LogMessage(){
            stream_<<std::endl;
        }
        std::ostream&stream(){return stream_;}

    private:
        std::ostringstream stream_;
        const LogSeverity severity_;
        const char *file_;
        const int line_;
    };
}

#define LOG_LAZY_STREAM(stream,condition) \
!(condition)?(void)0 : ::sun::LogMessageVoidify() & (stream)

#include "macro.h"

MAIN(){
    unsigned n=12;
    A a=15;
    LOG("%d",a.a);
    a&n;
    LOG("%d",a.a);
    std::ostringstream oss;
    LOG_LAZY_STREAM(oss,false)<<"false";
    LOG_LAZY_STREAM(oss,true)<<"true";
    LOG("%s",oss.str().c_str());
    Client x={3};
    Proxy y={&x};
    Proxy2 z={&y};
    //drill-down behavior
    LOG("%d,%d,%d",x.a,y->a,z->a);
}