#include <lingx/core/strings.h>
#include <cassert>

namespace {

void Strncpy_test()
{
    char str[20];
    char* p;
    const char* msg = "Hello World";
    const size_t len = std::strlen(msg);

    p = lnx::Strncpy(str, msg, len + 1);
    assert(p == str + len);
    assert(std::strcmp(str, msg) == 0);

    p = lnx::Strncpy(str, msg, len);
    assert(p == str + len - 1);
    assert(std::strcmp(str, "Hello Worl") == 0);

    p = lnx::Strncpy(str, msg, 1);
    assert(p == str);
    assert(std::strcmp(str, "") == 0);
}

void Slprintf_test()
{
    char str[20];
    char* p;
    const char* msg = "Hi Chris, 28";
    const size_t len = std::strlen(msg);

    p = lnx::Slprintf(str, str + len + 1, "Hi %s, %d", "Chris", 28);
    assert(p == str + len);
    assert(std::strcmp(str, msg) == 0);

    p = lnx::Slprintf(str, str + len, "Hi %s, %d", "Chris", 28);
    assert(p == str + len - 1);
    assert(std::strcmp(str, "Hi Chris, 2") == 0);

    p = lnx::Slprintf(str, str + 5, "");
    assert(p == str);
    assert(str[0] == '\0');

    str[0] = 'a';
    p = lnx::Slprintf(str, str - 5, "xyz");
    assert(p == str);
    assert(str[0] == 'a');
}

}

int main()
{
    Strncpy_test();
    Slprintf_test();

    return 0;
}
