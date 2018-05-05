#ifndef _LINGX_HTTP_HTTP_CONFIG_H
#define _LINGX_HTTP_HTTP_CONFIG_H

#include <lingx/core/common.h>

namespace lnx {

const uint HTTP_MAIN_CONF_OFFSET = 0;
const uint HTTP_SRV_CONF_OFFSET  = 1;
const uint HTTP_LOC_CONF_OFFSET  = 2;

struct HttpModuleCtx : ModuleCtx {
    typedef std::function<rc_t(const Conf&)> pre_conf_t;
    typedef std::function<rc_t(const Conf&)> post_conf_t;
    typedef std::function<MConfPtr(const Conf&)> create_conf_t;
    typedef std::function<const char*(const Conf&, MConf*)> init_conf_t;
    typedef std::function<const char*(const Conf&, const MConf*, MConf*)> merge_conf_t;

    HttpModuleCtx(const pre_conf_t&    pre_conf,
                  const post_conf_t&   post_conf,
                  const create_conf_t& create_main_conf,
                  const init_conf_t&   init_main_conf,
                  const create_conf_t& create_srv_conf,
                  const merge_conf_t&  merge_srv_conf,
                  const create_conf_t& create_loc_conf,
                  const merge_conf_t&  merge_loc_conf)
        : preconfiguration(pre_conf),
          postconfiguration(post_conf),
          create_main_conf(create_main_conf),
          init_main_conf(init_main_conf),
          create_srv_conf(create_srv_conf),
          merge_srv_conf(merge_srv_conf),
          create_loc_conf(create_loc_conf),
          merge_loc_conf(merge_loc_conf)
    { }

    pre_conf_t    preconfiguration;
    post_conf_t   postconfiguration;

    create_conf_t create_main_conf;
    init_conf_t   init_main_conf;

    create_conf_t create_srv_conf;
    merge_conf_t  merge_srv_conf;

    create_conf_t create_loc_conf;
    merge_conf_t  merge_loc_conf;
};

}

#endif
