#include <lingx/core/file.h>
#include <lingx/core/error.h>
#include <lingx/core/log.h>
#include <unistd.h>
#include <fcntl.h>

namespace lnx {

int File::open(const char* path, int flags, mode_t mode)
{
    fd_ = ::open(path, flags, mode);

    if (fd_ != -1)
        path_ = path;
    else
        Log_error(log_, Log::CRIT, errno, "open() \"%s\" failed", path);

    return fd_;
}

ssize_t File::read(void* buf, size_t len, off_t off) noexcept
{
    ssize_t n = ::pread(fd_, buf, len, off);
    if (n == -1) {
        Log_error(log_, Log::CRIT, errno,
                  "pread() \"%s\" failed", path_.c_str());
        return n;
    }

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

            Log_error(log_, Log::CRIT, errno,
                      "pwrite() \"%s\" failed", path_.c_str());

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
        if (rc == -1)
            Log_error(log_, Log::ALERT, errno,
                      "close() \"%s\" failed", path_.c_str());
        else
            fd_ = -1;
    }

    return rc;
}

}
