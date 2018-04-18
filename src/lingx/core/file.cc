#include <lingx/core/file.h>
#include <unistd.h>
#include <fcntl.h>

namespace lnx {

int File::open(const char* name, int flags, mode_t mode)
{
    fd_ = ::open(name, flags, mode);
    if (fd_ != -1)
        name_ = name;

    return fd_;
}

ssize_t File::read(void* buf, size_t len, off_t off) noexcept
{
    ssize_t n = ::pread(fd_, buf, len, off);
    if (n == -1)
        return n;

    offset_ += n;
    return n;
}

ssize_t File::write(const void* buf, size_t len, off_t off) noexcept
{
    ssize_t written = 0;

    for ( ;; ) {
        ssize_t n = ::pwrite(fd_, (const char *) buf + written, len, off);
        if (n == -1) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        offset_ += n;
        written += n;

        if ((size_t) n == len)
            break;

        off += n;
        len -= n;
    }

    return written;
}

int File::close() noexcept
{
    int rc = 0;

    if (fd_ != -1) {
        rc = ::close(fd_);
        if (rc == 0)
            fd_ = -1;
    }

    return rc;
}

}
