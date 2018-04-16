#include <lingx/core/open_file.h>
#include <unistd.h>  // close()
#include <fcntl.h>   // open()

namespace lnx {

int OpenFile::open() noexcept
{
    if (fd_ != -1 || name_.empty())
        return fd_;

    fd_ = ::open(name_.c_str(), O_WRONLY|O_APPEND|O_CREAT|O_CLOEXEC, 0644);

    return fd_;
}

int OpenFile::close() noexcept
{
    int rc = 0;
    if (fd_ != -1 && fd_ != STDERR_FILENO) {
        if ((rc = ::close(fd_)) == 0) {
            fd_ = -1;
        }
    }
    return rc;
}

}
