#include <lingx/core/open_file.h>
#include <unistd.h>  // ::close()

namespace lnx {

void OpenFile::close() noexcept
{
    if (fd_ != -1) {
        ::close(fd_);
        fd_   = -1;
        cldt_ = false;
    }
}

OpenFile::~OpenFile()
{
    if (cldt_)
        close();
}

}
