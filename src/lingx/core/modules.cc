#include <lingx/core/module.h>

namespace lnx {

extern Module Core_module;
extern Module Errlog_module;
extern Module Events_module;
extern Module Event_core_module;
extern Module Event_poll_module;
extern Module Http_module;
extern Module Http_core_module;

std::vector<std::reference_wrapper<Module>> Modules {
    Core_module,
    Errlog_module,
    Events_module,
    Event_core_module,
    Event_poll_module,
    Http_module,
    Http_core_module
};

}
