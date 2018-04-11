#ifndef _LINGX_CORE_CYCLE_H
#define _LINGX_CORE_CYCLE_H

#include <lingx/core/common.h>

namespace lnx {

class Module;

class Cycle {
    friend rc_t Init_new_cycle(const Cycle& ocycle, Cycle& cycle);

public:
    Cycle(const Cycle&) = delete;
    Cycle& operator=(const Cycle&) = delete;

    Cycle() = default;

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

    std::string prefix_;
    std::string conf_prefix_;
    std::string conf_file_;
    std::string conf_param_;
};

struct CoreConf : MConf {
};

rc_t Init_new_cycle(const Cycle& ocycle, Cycle& cycle);

extern Cycle* pCycle;

extern Module Core_module;
extern bool Opt_test_config;

}

#endif
