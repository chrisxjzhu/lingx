#ifndef _LINGX_CORE_FILES_H
#define _LINGX_CORE_FILES_H

#include <unistd.h>
#include <cstring>   // strlen()

namespace lnx {

inline void Write_stdout(const char* text) noexcept
{
    ::write(STDOUT_FILENO, text, std::strlen(text));
}

inline void Write_stderr(const char* text) noexcept
{
    ::write(STDERR_FILENO, text, std::strlen(text));
}

}

#endif
