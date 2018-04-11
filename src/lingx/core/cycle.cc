#include <lingx/core/cycle.h>
#include <lingx/core/path.h>

namespace lnx {

Cycle* pCycle = nullptr;

bool Opt_test_config = false;

void Cycle::set_prefix(const char* prefix)
{
    prefix_ = prefix;
    Path::tail_separator(prefix_);
}

void Cycle::set_conf_file(const char* file)
{
    conf_file_ = file;
    conf_prefix_ = Path(file).parent_path().string();
    Path::tail_separator(conf_prefix_);
}

}
