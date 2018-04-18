#ifndef _LINGX_CORE_FILE_H
#define _LINGX_CORE_FILE_H

#include <lingx/core/common.h>
#include <sys/stat.h>

namespace lnx {

class File {
public:
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    File() noexcept
    { std::memset(&info_, 0, sizeof(info_)); }

    int fd() const noexcept
    { return fd_; }

    const std::string& name() const noexcept
    { return name_; }

    const struct stat& info() const noexcept
    { return info_; }

    off_t offset() const noexcept
    { return offset_; }

    int open(const char* name, int flags, mode_t mode = 0644);

    int stat() noexcept
    { return ::fstat(fd_, &info_); }

    ssize_t read(void* buf, size_t len, off_t off) noexcept;
    ssize_t write(const void* buf, size_t len, off_t off) noexcept;

    int close() noexcept;

    ~File() { close(); }

private:
    int fd_ = -1;
    std::string name_;
    struct stat info_;
    off_t offset_ = 0;
};

}

#endif
