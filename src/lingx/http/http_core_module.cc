#include <lingx/http/http_config.h>
#include <lingx/http/http_core_module.h>
#include <lingx/core/conf_file.h>
#include <lingx/core/module.h>
#include <lingx/core/cycle.h>

namespace lnx {
namespace {

rc_t Http_core_preconfiguration_(const Conf& cf);
rc_t Http_core_postconfiguration_(const Conf& cf);
MConfPtr Http_core_create_main_conf_(const Conf& cf);
MConfPtr Http_core_create_srv_conf_(const Conf& cf);
MConfPtr Http_core_create_loc_conf_(const Conf& cf);
const char* Http_core_init_main_conf_(const Conf& cf, MConf* conf);
const char* Http_core_merge_srv_conf_(const Conf& cf, const MConf* prev, MConf* conf);
const char* Http_core_merge_loc_conf_(const Conf& cf, const MConf* prev, MConf* conf);

const char* Http_core_server_(const Conf& cf, const Command& cmd, MConfPtr& dummy);

std::vector<Command> Http_core_commands_ {
    Command {
        "server",
        HTTP_MAIN_CONF|CONF_BLOCK|CONF_NOARGS,
        Http_core_server_,
        0,
        0
    }
};

HttpModuleCtx Http_core_module_ctx_ {
    Http_core_preconfiguration_,
    Http_core_postconfiguration_,
    Http_core_create_main_conf_,
    Http_core_init_main_conf_,
    Http_core_create_srv_conf_,
    Http_core_merge_srv_conf_,
    Http_core_create_loc_conf_,
    Http_core_merge_loc_conf_
};

}

Module Http_core_module {
    "lnx_http_core_module",
    Http_core_module_ctx_,
    Http_core_commands_,
    HTTP_MODULE,
    nullptr,
    nullptr
};

namespace {

rc_t Http_core_preconfiguration_(const Conf&)
{
    return OK;
}

rc_t Http_core_postconfiguration_(const Conf&)
{
    return OK;
}

MConfPtr Http_core_create_main_conf_(const Conf&)
{
    return nullptr;
}

MConfPtr Http_core_create_srv_conf_(const Conf&)
{
    return nullptr;
}

MConfPtr Http_core_create_loc_conf_(const Conf&)
{
    return nullptr;
}

const char* Http_core_init_main_conf_(const Conf&, MConf*)
{
    return CONF_OK;
}

const char* Http_core_merge_srv_conf_(const Conf&, const MConf*, MConf*)
{
    return CONF_OK;
}

const char* Http_core_merge_loc_conf_(const Conf&, const MConf*, MConf*)
{
    return CONF_OK;
}

const char* Http_core_server_(const Conf& cf, const Command& /*cmd*/, MConfPtr&)
{
    std::shared_ptr<HttpConfCtx> conf_ctx = std::make_shared<HttpConfCtx>();

    conf_ctx->main_conf = *cf.ctx();
    conf_ctx->srv_conf = std::make_shared<ConfCtx>(Http_max_module);
    conf_ctx->loc_conf = std::make_shared<ConfCtx>(Http_max_module);

    for (const Module& mod : cf.cycle()->modules()) {
        if (mod.type() != HTTP_MODULE)
            continue;

        const HttpModuleCtx& ctx = static_cast<const HttpModuleCtx&>(mod.ctx());
        uint mi = mod.ctx_index();

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

    HttpCoreSrvConfPtr cscf = std::static_pointer_cast<HttpCoreSrvConf>
                       ((*conf_ctx->srv_conf)[Http_core_module.ctx_index()]);

    cscf->ctx = conf_ctx;

    HttpCoreMainConfPtr cmcf = std::static_pointer_cast<HttpCoreMainConf>
                       ((*conf_ctx->main_conf)[Http_core_module.ctx_index()]);

    cmcf->servers.push_back(cscf);

    Conf ncf = cf;
    ncf.set_ctx(&conf_ctx->main_conf);
    ncf.set_cmd_type(HTTP_SRV_CONF);

    const char* rv = ncf.parse("");

    /* TODO: check cscf->listen */

    return rv;
}

}

}
