#include <lingx/core/process.h>
#include <lingx/core/process_cycle.h>
#include <lingx/core/times.h>
#include <lingx/core/cycle.h>
#include <lingx/core/log.h>
#include <csignal>

namespace lnx {
namespace {

struct Signal_ {
    int signo;
    const char* signame;
    const char* name;
    void (*handler)(int);
};

void Signal_handler_(int signo) noexcept;

Signal_ Signals_[] = {
    { SIGTERM, "SIGTERM", "stop", Signal_handler_},
    { SIGQUIT, "SIGQUIT", "quit", Signal_handler_},
    { SIGINT,  "SIGINT",   "",    Signal_handler_},
    { 0, nullptr, "", nullptr }
};

void Signal_handler_(int signo) noexcept
{
    const Signal_* sig = nullptr;

    for (sig = Signals_; sig->signo != 0; ++sig)
        if (sig->signo == signo)
            break;

    Time_sigsafe_update();

    const char* action = "";

    switch (Process_type) {
    case PROCESS_MASTER:
    case PROCESS_SINGLE:
        switch (signo) {
        case SIGQUIT:
            Quit = true;
            action = ", shutting down";
            break;

        case SIGTERM:
        case SIGINT:
            Terminate = true;
            action = ", exiting";
            break;
        }
        break;

    default:
        break;
    }

    Log_error(Cur_cycle->log(), Log::NOTICE, 0,
              "signal %d (%s) received%s", signo, sig->signame, action);
}

}

rc_t Init_signals(const Log* log) noexcept
{
    for (Signal_* sig = Signals_; sig->signo != 0; ++sig) {
        struct sigaction sa;
        std::memset(&sa, 0, sizeof sa);
        sa.sa_handler = sig->handler;
        ::sigemptyset(&sa.sa_mask);
        if (::sigaction(sig->signo, &sa, nullptr) == -1) {
            Log_error(log, Log::EMERG, errno,
                      "sigaction(%s) failed", sig->signame);
            return ERROR;
        }
    }

    return OK;
}

int Os_signal_process(const CyclePtr& cycle, const char* name, pid_t pid) noexcept
{
    for (Signal_* sig = Signals_; sig->signo != 0; ++sig)
        if (std::strcmp(name, sig->name) == 0) {
            if (::kill(pid, sig->signo) == 0)
                return 0;

            Log_error(cycle->log(), Log::ALERT, errno,
                      "kill(%d, %d) failed", pid, sig->signo);
        }

    return 1;
}

}
