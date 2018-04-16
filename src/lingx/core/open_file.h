#ifndef _LINGX_CORE_OPEN_FILE_H
#define _LINGX_CORE_OPEN_FILE_H

#include <string>

namespace lnx {

class OpenFile {
public:
    OpenFile(const OpenFile&) = delete;
    OpenFile& operator=(const OpenFile&) = delete;

    OpenFile(int fd, const std::string& name)
        : fd_(fd), name_(name)
    { }

    int fd() const noexcept
    { return fd_; }

    const std::string& name() const noexcept
    { return name_; }

    int open() noexcept;

    int close() noexcept;

    ~OpenFile() { close(); }

private:
    int fd_ = -1;
    std::string name_;
};

}

#endif
