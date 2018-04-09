#ifndef _LINGX_CORE_ERRNO_H
#define _LINGX_CORE_ERRNO_H

#include <cstddef>  // size_t

namespace lnx {

char* Strerror(int err, char* estr, size_t size) noexcept;

}

#endif
