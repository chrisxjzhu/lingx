#ifndef _LINGX_CORE_OS_H
#define _LINGX_CORE_OS_H

#include <lingx/core/common.h>
#include <lingx/core/os_io.h>

namespace lnx {

rc_t Os_init(const Log* log) noexcept;
void Os_status(const Log* log) noexcept;
rc_t Os_specific_init(const Log* log) noexcept;
void Os_specific_status(const Log* log) noexcept;

}

#endif
