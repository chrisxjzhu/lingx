#ifndef _LINGX_CORE_CYCLE_H
#define _LINGX_CORE_CYCLE_H

#include <lingx/core/common.h>

namespace lnx {

class Cycle {
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
    LogPtr log_;

    std::string prefix_;
    std::string conf_prefix_;
    std::string conf_file_;
    std::string conf_param_;
};

extern Cycle* pCycle;

extern bool Opt_test_config;

}

#endif
