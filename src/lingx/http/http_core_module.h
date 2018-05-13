#ifndef _LINGX_HTTP_HTTP_CORE_MODULE_H
#define _LINGX_HTTP_HTTP_CORE_MODULE_H

#include <lingx/core/common.h>

namespace lnx {

struct HttpCoreLocConf : MConf {
};

struct HttpCoreSrvConf : MConf {
    std::shared_ptr<HttpConfCtx> ctx;
};

using HttpCoreSrvConfPtr = std::shared_ptr<HttpCoreSrvConf>;

struct HttpCoreMainConf : MConf {
    std::vector<HttpCoreSrvConfPtr> servers;
};

using HttpCoreMainConfPtr = std::shared_ptr<HttpCoreMainConf>;

extern Module Http_core_module;
extern uint Http_max_module;

}

#endif
