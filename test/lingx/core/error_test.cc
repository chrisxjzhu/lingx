#include <lingx/core/error.h>
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

void Throw_error_test()
{
    const char* path = "/non-exist";

    try {
        throw lnx::Error(2, "open() \"%s\" failed", path);
    } catch (const lnx::Error& error) {
        assert(std::strcmp(error.what(),
        "open() \"/non-exist\" failed (2: No such file or directory)") == 0);
    }
}

}

int main()
{
    Strerror_test();
    Throw_error_test();

    return 0;
}
