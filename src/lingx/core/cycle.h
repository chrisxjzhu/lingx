#ifndef _LINGX_CORE_CYCLE_H
#define _LINGX_CORE_CYCLE_H

#include <lingx/core/common.h>

namespace lnx {

class Module;

class Cycle {
    friend CyclePtr Init_new_cycle(const CyclePtr& old_cycle);

public:
    Cycle(const Cycle&) = delete;
    Cycle& operator=(const Cycle&) = delete;

    Cycle() = default;

    bool is_init_cycle() const noexcept
    { return conf_ctx_.empty(); }

    const LogPtr& log() const noexcept
    { return log_; }

    void set_log(const LogPtr& log) noexcept
    { log_ = log; }

    void set_prefix(const char* prefix);
    void set_conf_file(const char* file);

    void set_conf_param(const char* param)
    { conf_param_ = param; }

private:
    std::vector<MConfPtr> conf_ctx_;

    LogPtr log_;

    std::vector<std::reference_wrapper<Module>> modules_;

    CyclePtr old_cycle_;

    std::string prefix_;
    std::string conf_prefix_;
    std::string conf_file_;
    std::string conf_param_;
};

struct CoreConf : MConf {
};

CyclePtr Init_new_cycle(const CyclePtr& old_cycle);

extern CyclePtr Cur_cycle;

extern Module Core_module;
extern bool Opt_test_config;

}

#endif
