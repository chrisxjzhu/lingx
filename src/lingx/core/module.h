#ifndef _LINGX_CORE_MODULE_H
#define _LINGX_CORE_MODULE_H

#include <lingx/core/common.h>

namespace lnx {

enum {
    CORE_MODULE  = 0x45524F43, /* "CORE" */
    EVENT_MODULE = 0x544E5645, /* "EVNT" */
    HTTP_MODULE  = 0x50545448  /* "HTTP" */
};

class Module {
    friend void Preinit_modules();

public:
    typedef std::function<rc_t(Cycle*)> init_module_t;
    typedef std::function<rc_t(Cycle*)> init_process_t;

    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;

    Module(const char* name,
           const ModuleCtx& ctx,
           const std::vector<Command>& commands,
           int type,
           const init_module_t& init_module,
           const init_process_t& init_process)
        : name_(name), ctx_(ctx), commands_(commands), type_(type),
          init_module(init_module),
          init_process(init_process)
    { }

    uint index() const noexcept
    { return index_; }

    uint ctx_index() const noexcept
    { return ctx_index_; }

    const ModuleCtx& ctx() const noexcept
    { return ctx_; }

    const std::vector<Command>& commands() const noexcept
    { return commands_; }

    int type() const noexcept
    { return type_; }

    void set_ctx_index(uint index) noexcept
    { ctx_index_ = index; }

private:
    uint index_ = -1;
    uint ctx_index_ = -1;
    std::string_view name_;
    const ModuleCtx& ctx_;
    const std::vector<Command>& commands_;
    int type_;

public:
    const init_module_t init_module;
    const init_process_t init_process;
};

struct CoreModuleCtx : ModuleCtx {
    typedef std::function<MConfPtr(Cycle*)> create_conf_t;
    typedef std::function<const char*(Cycle*, MConf*)> init_conf_t;

    CoreModuleCtx(const char* name,
                  const create_conf_t& create_conf,
                  const init_conf_t& init_conf)
        : name(name), create_conf(create_conf), init_conf(init_conf)
    { }

    std::string_view name;
    create_conf_t create_conf;
    init_conf_t init_conf;
};

extern std::vector<std::reference_wrapper<Module>> Modules;
extern size_t Max_modules_n;

void Preinit_modules();

}

#define Get_conf(type, cycle, module)                                        \
    std::static_pointer_cast<type>((*(cycle)->conf_ctx())[(module).index()])

#endif
