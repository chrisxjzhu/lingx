#include <lingx/core/os.h>
#include <lingx/core/log.h>
#include <lingx/core/strings.h>
#include <sys/utsname.h>

namespace lnx {
namespace {

char Linux_kern_ostype_[50];
char Linux_kern_osrelease_[50];

Os_io_t Linux_io_ = {
    Unix_recv,
    Unix_send,
    0
};

}

rc_t Os_specific_init(const Log* log) noexcept
{
    struct utsname u;

    if (::uname(&u) == -1) {
        Log_error(log, Log::ALERT, errno, "uname() failed");
        return ERROR;
    }

    Strncpy(Linux_kern_ostype_, u.sysname, sizeof(Linux_kern_ostype_));
    Strncpy(Linux_kern_osrelease_, u.release, sizeof(Linux_kern_osrelease_));

    Os_io = Linux_io_;

    return OK;
}

void Os_specific_status(const Log* log) noexcept
{
    Log_error(log, Log::NOTICE, 0, "OS: %s %s",
              Linux_kern_ostype_, Linux_kern_osrelease_);
}

}
