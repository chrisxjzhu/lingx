#include <lingx/core/file.h>
#include <lingx/core/error.h>
#include <lingx/core/log.h>
#include <unistd.h>

namespace lnx {

File::File(const char* path, int flags, mode_t mode)
{
    fd_ = ::open(path, flags, mode);
    if (fd_ == -1)
        throw Error(errno, "open() \"%s\" failed", path);

    path_ = path;
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

int File::close() noexcept
{
    int rc = 0;
    if (fd_ != -1) {
        if ((rc = ::close(fd_)) == 0)
            fd_ = -1;
    }
    return rc;
}

}
