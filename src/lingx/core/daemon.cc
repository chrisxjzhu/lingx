#include <lingx/core/daemon.h>
#include <lingx/core/process_cycle.h>
#include <lingx/core/log.h>
#include <cstdlib>     // exit()
#include <unistd.h>    // fork(), setsid(), dup2(), ...
#include <fcntl.h>     // open()
#include <sys/stat.h>  // umask()

namespace lnx {

rc_t Daemonize(const LogPtr& log) noexcept
{
    switch (::fork()) {
    case -1:
        Log_error(log, Log::EMERG, errno, "fork() failed");
        return LNX_ERROR;

    /* child */
    case 0:
        break;

    /* parent */
    default:
        std::exit(0);
    }

    Pid = ::getpid();

    if (::setsid() == -1) {
        Log_error(log, Log::EMERG, errno, "setsid() failed");
        return LNX_ERROR;
    }

    ::umask(0);

    int fd = ::open("/dev/null", O_RDWR);
    if (fd == -1) {
        Log_error(log, Log::EMERG, errno, "open(\"/dev/null\") failed");
        return LNX_ERROR;
    }

    if (::dup2(fd, STDIN_FILENO) == -1) {
        Log_error(log, Log::EMERG, errno, "dup2(STDIN) failed");
        return LNX_ERROR;
    }

    if (::dup2(fd, STDOUT_FILENO) == -1) {
        Log_error(log, Log::EMERG, errno, "dup2(STDOUT) failed");
        return LNX_ERROR;
    }

#if 0
    /* Reserve the STDERR_FILENO to avoid it being used for other files */

    if (::dup2(fd, STDERR_FILENO) == -1) {
        Log_error(log, Log::EMERG, errno, "dup2(STDERR) failed");
        return LNX_ERROR;
    }
#endif

    if (fd > STDERR_FILENO) {
        if (::close(fd) == -1) {
            Log_error(log, Log::EMERG, errno, "close() failed");
            return LNX_ERROR;
        }
    }

    return LNX_OK;
}

}
