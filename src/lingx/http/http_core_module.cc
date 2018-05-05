#include <lingx/http/http_config.h>
#include <lingx/core/conf_file.h>
#include <lingx/core/module.h>

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

std::vector<Command> Http_core_commands_ {
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

}

}
