#include <lingx/http/http.h>
#include <lingx/http/http_config.h>
#include <lingx/core/conf_file.h>
#include <lingx/core/module.h>
#include <lingx/core/cycle.h>

namespace lnx {
namespace {

const char* Http_block_(const Conf& cf, const Command& cmd, MConfPtr& conf);

const char* Http_merge_servers_(const Conf& cf,
                                const HttpCoreMainConfPtr& cmcf,
                                const HttpModuleCtx& ctx, uint ctx_index);

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

uint Http_max_module = 0;

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

    std::shared_ptr<HttpConfCtx> conf_ctx = std::make_shared<HttpConfCtx>();

    conf = conf_ctx;

    Http_max_module = cf.cycle()->count_modules(HTTP_MODULE);

    conf_ctx->main_conf = std::make_shared<ConfCtx>(Http_max_module);
    conf_ctx->srv_conf = std::make_shared<ConfCtx>(Http_max_module);
    conf_ctx->srv_conf = std::make_shared<ConfCtx>(Http_max_module);

    for (const Module& mod : cf.cycle()->modules()) {
        if (mod.type() != HTTP_MODULE)
            continue;

        const HttpModuleCtx& ctx = static_cast<const HttpModuleCtx&>(mod.ctx());
        uint mi = mod.ctx_index();

        if (ctx.create_main_conf) {
            (*conf_ctx->main_conf)[mi] = ctx.create_main_conf(cf);
            if ((*conf_ctx->main_conf)[mi] == nullptr)
                return CONF_ERROR;
        }

        if (ctx.create_srv_conf) {
            (*conf_ctx->srv_conf)[mi] = ctx.create_srv_conf(cf);
            if ((*conf_ctx->srv_conf)[mi] == nullptr)
                return CONF_ERROR;
        }

        if (ctx.create_loc_conf) {
            (*conf_ctx->loc_conf)[mi] = ctx.create_loc_conf(cf);
            if ((*conf_ctx->loc_conf)[mi] == nullptr)
                return CONF_ERROR;
        }
    }

    Conf ncf = cf;
    ncf.set_ctx(&conf_ctx->main_conf);

    for (const Module& mod : ncf.cycle()->modules()) {
        if (mod.type() != HTTP_MODULE)
            continue;

        const HttpModuleCtx& ctx = static_cast<const HttpModuleCtx&>(mod.ctx());

        if (ctx.preconfiguration) {
            if (ctx.preconfiguration(ncf) != OK)
                return CONF_ERROR;
        }
    }

    /* parse inside the http{} block */

    ncf.set_module_type(HTTP_MODULE);
    ncf.set_cmd_type(HTTP_MAIN_CONF);

    const char* rv = ncf.parse("");
    if (rv != CONF_OK)
        return rv;

    /*
     * init http{} main_conf's, merge the server{}s' srv_conf's
     * and its location{}s' loc_conf's
     */

    HttpCoreMainConfPtr cmcf = std::static_pointer_cast<HttpCoreMainConf>
                       ((*conf_ctx->main_conf)[Http_core_module.ctx_index()]);

    for (const Module& mod : ncf.cycle()->modules()) {
        if (mod.type() != HTTP_MODULE)
            continue;

        const HttpModuleCtx& ctx = static_cast<const HttpModuleCtx&>(mod.ctx());
        uint mi = mod.ctx_index();

        if (ctx.init_main_conf) {
            const char* rv = ctx.init_main_conf(ncf, (*conf_ctx->main_conf)[mi].get());
            if (rv != CONF_OK)
                return rv;
        }

        const char* rv = Http_merge_servers_(ncf, cmcf, ctx, mi);
        if (rv != CONF_OK)
            return rv;
    }

    return CONF_OK;
}

const char* Http_merge_servers_(const Conf& /*cf*/,
                                const HttpCoreMainConfPtr& /*cmcf*/,
                                const HttpModuleCtx& /*ctx*/, uint /*ctx_index*/)
{
    return CONF_OK;
}

}

}
