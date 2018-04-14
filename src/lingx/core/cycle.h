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

    const std::vector<MConfPtr>& conf_ctx() const noexcept
    { return conf_ctx_; }

    const LogPtr& log() const noexcept
    { return log_; }

    std::string prefix() const noexcept
    { return prefix_; }

    void set_log(const LogPtr& log) noexcept
    { log_ = log; }

    void set_prefix(const char* prefix);
    void set_conf_file(const std::string& file);

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
    bool daemon;
    std::string pid_path;
};

CyclePtr Init_new_cycle(const CyclePtr& old_cycle);

int Signal_process(const CyclePtr& cycle, const char* sig) noexcept;

extern CyclePtr Cur_cycle;

extern Module Core_module;
extern bool Opt_test_config;

}

#endif
