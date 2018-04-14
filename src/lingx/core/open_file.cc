#include <lingx/core/open_file.h>
#include <unistd.h>  // ::close()

namespace lnx {

int OpenFile::close() noexcept
{
    int rc = 0;
    if (fd_ != -1) {
        rc = ::close(fd_);
        if (rc == 0) {
            fd_ = -1;
            cldt_ = false;
        }
    }
    return rc;
}

OpenFile::~OpenFile()
{
    if (cldt_)
        close();
}

}
