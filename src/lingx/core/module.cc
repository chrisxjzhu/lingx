#include <lingx/core/module.h>

namespace lnx {

size_t Max_modules_n = 0;

void Preinit_modules()
{
    for (uint idx = 0; idx < Modules.size(); ++idx)
        Modules[idx].get().index_ = idx;

    Max_modules_n = Modules.size();
}

}
