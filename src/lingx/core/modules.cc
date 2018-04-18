#include <lingx/core/module.h>

namespace lnx {

extern Module Core_module;
extern Module Events_module;

std::vector<std::reference_wrapper<Module>> Modules {
    Core_module,
    Events_module
};

}
