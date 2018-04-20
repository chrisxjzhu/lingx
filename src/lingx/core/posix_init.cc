#include <lingx/core/os.h>
#include <lingx/core/log.h>
#include <lingx/config.h>
#include <lingx/lingx.h>

namespace lnx {

Os_io_t Os_io = {
    Unix_recv,
    Unix_send,
    0
};

rc_t Os_init(const LogPtr& log) noexcept
{
    if (Os_specific_init(log) != OK)
        return ERROR;

    /* TODO: other work */

    return OK;
}

void Os_status(const LogPtr& log) noexcept
{
    Log_error(log, Log::NOTICE, 0, LINGX_VER);

    Log_error(log, Log::NOTICE, 0, "built by " LNX_COMPILER);

    Os_specific_status(log);

    /* TODO: log rlmt */
}

}
