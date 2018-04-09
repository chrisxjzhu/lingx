#ifndef _LINGX_CORE_OPEN_FILE_H
#define _LINGX_CORE_OPEN_FILE_H

namespace lnx {

class OpenFile {
public:
    OpenFile(const OpenFile&) = delete;
    OpenFile& operator=(const OpenFile&) = delete;

    explicit OpenFile(int fd, bool cldt = true) noexcept
        : fd_(fd), cldt_(cldt)
    { }

    void close() noexcept;

    ~OpenFile();

private:
    int fd_ = -1;
    bool cldt_ = false;
};

}

#endif
