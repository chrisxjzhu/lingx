#include <lingx/core/errno.h>
#include <cstring>  // strlen()
#include <cassert>

namespace {

void Strerror_test()
{
    char estr[50];
    const char* msg = "No such file or directory";
    const size_t len = std::strlen(msg);
    char* p;

    p = lnx::Strerror(2, estr, 50);
    assert(std::strcmp(estr, msg) == 0);
    assert(p == estr + len);

    p = lnx::Strerror(2, estr, 5);
    assert(std::strcmp(estr, "No s") == 0);
    assert(p == estr + 4);
}

}

int main()
{
    Strerror_test();

    return 0;
}
