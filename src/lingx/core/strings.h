#ifndef _LINGX_CORE_STRINGS_H
#define _LINGX_CORE_STRINGS_H

#include <cstdio>   // va_list
#include <cstring>

namespace lnx {

inline char* Memcpy(char* dst, const char* src, size_t n) noexcept
{ return (char*) std::memcpy(dst, src, n) + n; }

char* Strncpy(char* dst, const char* src, size_t n) noexcept;

char* Slprintf(char* buf, const char* last, const char* fmt, ...) noexcept;
char* Vslprintf(char* buf, const char* last, const char* fmt, va_list args) noexcept;

int Atoi(const char* str, size_t n) noexcept;

}

#endif
