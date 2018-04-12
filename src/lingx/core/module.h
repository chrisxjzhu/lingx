#ifndef _LINGX_CORE_MODULE_H
#define _LINGX_CORE_MODULE_H

#include <lingx/core/common.h>

namespace lnx {

enum CommandType {
    MAIN_CONF = 0x01000000
};

struct Command {
    std::string_view name;
    int type = 0;
};

struct ModuleCtx {};

enum ModuleType {
    CORE_MODULE = 0x45524F43   /* "CORE" */
};

class Module {
    friend void Preinit_modules();

public:
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;

    Module(const char* name,
           const ModuleCtx& ctx,
           const std::vector<Command>& commands,
           ModuleType type)
        : name_(name), ctx_(ctx), commands_(commands), type_(type)
    { }

    uint index() const noexcept
    { return index_; }

    const ModuleCtx& ctx() const noexcept
    { return ctx_; }

    ModuleType type() const noexcept
    { return type_; }

private:
    uint index_ = -1;
    std::string_view name_;
    const ModuleCtx& ctx_;
    const std::vector<Command>& commands_;
    ModuleType type_;
};

struct CoreModuleCtx : ModuleCtx {
    typedef std::function<MConfPtr(const CyclePtr&)> create_conf_t;
    typedef std::function<const char*(const CyclePtr&, const MConfPtr&)> init_conf_t;

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

#endif
