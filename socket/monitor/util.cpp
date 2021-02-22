#include "util.h"
#include "pipe.h"

#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

#include <memory>
#include <vector>

namespace sun {
    namespace details {
        const char *DefaultTimeFormatter(char buf[32], int seconds, int milliseconds) {
            static thread_local char __internal_buf[32];
            char *p = buf ?: __internal_buf;
            sprintf(p, "%d.%03d", seconds, milliseconds);
            return p;
        }

        size_t strlcpy(char *d, const char *s, size_t n) {
//#define LOWZEROS  0x0101010101010101
//#define HIGHZEROS 0x8080808080808080
#define HASZERO(x) (((x)-0x0101010101010101)&~(x)&0x8080808080808080)
//#define PTRALIGN (sizeof(size_t)-1)
#define PTRALIGNOFFSET(x) ((uintptr_t)(x)&(sizeof(size_t)-1))
            char *d0 = d;
            size_t *wd;
            const size_t *ws;
            if (PTRALIGNOFFSET(s) == PTRALIGNOFFSET(d)) {
                for (; PTRALIGNOFFSET(s) && n && (*d = *s); --n, ++s, ++d) {}
                if (n && *s) {
                    wd = (size_t *) d;
                    ws = (const size_t *) s;
                    for (; n >= sizeof(size_t) && !HASZERO(*ws); *wd = *ws, n -= sizeof(size_t), ++ws, ++wd) {}
                    d = (char *) wd;
                    s = (const char *) ws;
                }
            }
            for (; n && (*d = *s); --n, ++s, ++d) {}
            *d = 0;
            return d - d0;
#undef PTRALIGNOFFSET
#undef HASZERO
        }

        void CloseFp(FILE *fp) {
            if (fp) {
                fclose(fp);
            }
        }
    }
    namespace util {
        double Milliseconds() {
            struct timespec ts{};
            clock_gettime(CLOCK_REALTIME, &ts);
            return ts.tv_sec * kThousand + ts.tv_nsec / kMillion;
        }

        bool ValidProcess(int pid) {
            return kill(pid, 0) == 0;
        }

        const char *Now(char buf[32], TimeFormatter timeFormatter) {
            struct timespec ts{};
            clock_gettime(CLOCK_REALTIME, &ts);
            return timeFormatter
                   ? timeFormatter(buf, ts.tv_sec, ts.tv_nsec / kMillion)
                   : details::DefaultTimeFormatter(buf, ts.tv_sec, ts.tv_nsec / kMillion);
        }

        int GetPid() {
            static thread_local int pid = ::getpid();
            return pid;
        }

        int Sleep(int ms) {
            static std::unique_ptr<Pipe> pipe;
            if (!pipe) {
                pipe.reset(new Pipe());
            }
            struct pollfd pfd{.fd=pipe->readEnd(), .events=POLLIN};
            return poll(&pfd, 1, ms);
        }

        int NiceName(std::string &&name) {
            static std::string name_buffer;
            char buf[2048], *p;
            int i;
#define DECLAREPAIR(x) start_##x,end_##x
            unsigned long DECLAREPAIR(data), start_brk, DECLAREPAIR(code),
                    start_stack, DECLAREPAIR(arg), DECLAREPAIR(env), brk_val;
#undef DECLAREPAIR
            std::unique_ptr<FILE, decltype(&details::CloseFp)> fp(fopen("/proc/self/stat", "r"), &details::CloseFp);
            ERRRET(!fp, -1, , 1, "fopen");
            fgets(buf, sizeof(buf), fp.get());
#define SKIPFIELDS(_n) \
for(i=0;i<_n;++i){\
    if(!p){return -1;}\
    p=strchr(p+1,' ');\
}
#define EXPECT(x) if(!(x)){return -1;}
            p = strchr(buf, ' ');
            SKIPFIELDS(24);
            EXPECT(p);
            i = sscanf(p, "%lu %lu %lu", &start_code, &end_code, &start_stack);
            EXPECT(i == 3);
            SKIPFIELDS(19);
            EXPECT(p);
            i = sscanf(p, "%lu %lu %lu %*u %*u %lu %lu",
                       &start_data, &end_data, &start_brk, &start_env, &end_env);
            EXPECT(i == 5);
#undef EXPECT
#undef SKIPFIELDS
            name_buffer.swap(name);
            start_arg = (unsigned long) name_buffer.c_str();
            end_arg = start_arg + name_buffer.size() + 1;
            brk_val = syscall(__NR_brk, 0);
            struct prctl_mm_map map{};
            map.start_code = start_code;
            map.end_code = end_code;
            map.start_stack = start_stack;
            map.start_data = start_data;
            map.end_data = end_data;
            map.start_brk = start_brk;
            map.brk = brk_val;
            map.arg_start = start_arg;
            map.arg_end = end_arg;
            map.env_start = start_env;
            map.env_end = end_env;
            map.exe_fd = -1;
            return prctl(PR_SET_MM, PR_SET_MM_MAP, &map, sizeof(map), 0);
        }

        const char *WhichExe() {
            static __thread char buf[1024];
            if (!*buf) {
                int nr = readlink("/proc/self/exe", buf, sizeof(buf));
                buf[nr] = 0;
            }
            return buf;
        }

        TimeThis::TimeThis(std::string tag) : ms_(0), tag_(std::move(tag)) {
            FUNCLOG("START,%s", tag_.c_str());
            OnStart();
        }

        TimeThis::~TimeThis() {
            FUNCLOG("STOP,%s,%g", tag_.c_str(), OnStop());
        }
    }
}
#if 0
int main() {
    char a[64];
    char b[] = "I love OnePiece!";
    sun::details::strlcpy(a, b, sizeof(b) - 1);
    LOGINFO("%p,%p,\"%s\"", a, b, a);
}
#endif