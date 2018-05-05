#include <lingx/http/http_config.h>
#include <lingx/core/conf_file.h>
#include <lingx/core/module.h>
#include <lingx/core/cycle.h>

namespace lnx {
namespace {

const char* Http_block_(const Conf& cf, const Command& cmd, MConfPtr& conf);

std::vector<Command> Http_commands_ {
    Command {
        "http",
        MAIN_CONF|CONF_BLOCK|CONF_NOARGS,
        Http_block_,
        0,
        0
    }
};

CoreModuleCtx Http_module_ctx_ {
    "http",
    nullptr,
    nullptr
};

}

Module Http_module {
    "lnx_http_module",
    Http_module_ctx_,
    Http_commands_,
    CORE_MODULE,
    nullptr,
    nullptr
};

namespace {

const char* Http_block_(const Conf& cf, const Command&, MConfPtr& conf)
{
    if (conf.get())
        return "is duplicate";

    std::shared_ptr<MConfs> hconfs = std::make_shared<MConfs>();

    conf = hconfs;

    uint http_max_module = cf.cycle()->count_modules(HTTP_MODULE);

    hconfs->ctxs.resize(http_max_module * 3);

    for (const Module& mod : cf.cycle()->modules()) {
        if (mod.type() != HTTP_MODULE)
            continue;

        const HttpModuleCtx& ctx = static_cast<const HttpModuleCtx&>(mod.ctx());
        uint mi = mod.ctx_index();

        if (ctx.create_main_conf) {
            hconfs->ctxs[mi] = ctx.create_main_conf(cf);
            if (hconfs->ctxs[mi] == nullptr)
                return CONF_ERROR;
        }

        if (ctx.create_srv_conf) {
            hconfs->ctxs[http_max_module + mi] = ctx.create_srv_conf(cf);
            if (hconfs->ctxs[http_max_module + mi] == nullptr)
                return CONF_ERROR;
        }

        if (ctx.create_loc_conf) {
            hconfs->ctxs[http_max_module * 2 + mi] = ctx.create_loc_conf(cf);
            if (hconfs->ctxs[http_max_module * 2 + mi] == nullptr)
                return CONF_ERROR;
        }
    }

    return CONF_OK;
}

}

}
