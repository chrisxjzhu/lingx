#include <lingx/core/open_file.h>
#include <cassert>
#include <unistd.h>  // unlink()
#include <fcntl.h>   // open()

namespace {

void OpenFile_test()
{
    const char* path = "tmp.txt";

    int fd = ::open(path, O_RDWR|O_CREAT, 0644);
    assert(::fcntl(fd, F_GETFD) != -1);

    {
        lnx::OpenFile f(fd, path);
    }

    assert(::fcntl(fd, F_GETFD) == -1);

    assert(::unlink(path) != -1);
}

}

int main()
{
    OpenFile_test();

    return 0;
}
