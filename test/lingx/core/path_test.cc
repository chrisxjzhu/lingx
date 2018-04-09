#include <lingx/core/path.h>
#include <cassert>

namespace {

void Path_parent_path_test()
{
    assert(lnx::Path().parent_path().string() == "");
    assert(lnx::Path("a").parent_path().string() == "");
    assert(lnx::Path("a/b").parent_path().string() == "a");
    assert(lnx::Path("a/b//").parent_path().string() == "a/b");
    assert(lnx::Path("a/b//.").parent_path().string() == "a/b");
    assert(lnx::Path("///.").parent_path().string() == "/");
    assert(lnx::Path("/a").parent_path().string() == "/");
    assert(lnx::Path("/a/b//.").parent_path().string() == "/a/b");
    assert(lnx::Path("/a/b//./").parent_path().string() == "/a/b//.");
}

}

int main()
{
    Path_parent_path_test();

    return 0;
}
