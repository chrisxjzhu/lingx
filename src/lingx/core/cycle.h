#ifndef _LINGX_CORE_CYCLE_H
#define _LINGX_CORE_CYCLE_H

#include <lingx/core/common.h>
#include <lingx/core/open_file.h>
#include <list>

namespace lnx {

class Module;

class Cycle {
    friend CyclePtr Init_new_cycle(const CyclePtr& old_cycle);

public:
    Cycle(const Cycle&) = delete;
    Cycle& operator=(const Cycle&) = delete;

    Cycle() = default;

    bool is_init_cycle() const noexcept
    { return conf_ctxs_.empty(); }

    std::vector<MConfPtr>& conf_ctxs() noexcept
    { return conf_ctxs_; }

    const std::vector<std::reference_wrapper<Module>>& modules() const noexcept
    { return modules_; }

    const LogPtr& log() const noexcept
    { return log_; }

    const std::string& prefix() const noexcept
    { return prefix_; }

    const std::string& conf_file() const noexcept
    { return conf_file_; }

    const std::string& conf_param() const noexcept
    { return conf_param_; }

    void set_log(const LogPtr& log) noexcept
    { log_ = log; }

    void set_connection_n(uint conn_n) noexcept
    { connection_n_ = conn_n; }

    void set_prefix(const char* prefix);
    void set_conf_file(const std::string& file);

    void set_conf_param(const char* param)
    { conf_param_ = param; }

    OpenFilePtr log_open_file(const std::string& name);

    rc_t log_redirect_stderr() noexcept;

    uint count_modules(int type) noexcept;

private:
    rc_t log_open_default_();

    uint count_module_ctx_index_(int type, uint index) noexcept;

    std::vector<MConfPtr> conf_ctxs_;

    LogPtr new_log_;
    LogPtr log_;

    std::vector<std::reference_wrapper<Module>> modules_;

    std::list<OpenFilePtr> open_files_;

    uint connection_n_ = 0;

    CyclePtr old_cycle_;

    std::string prefix_;
    std::string conf_prefix_;
    std::string conf_file_;
    std::string conf_param_;
};

struct CoreConf : MConf {
    flag_t daemon;
    flag_t master;
    std::string pid_path;
};

CyclePtr Init_new_cycle(const CyclePtr& old_cycle);

int Signal_process(const CyclePtr& cycle, const char* sig) noexcept;

rc_t Create_pidfile(const std::string& path, const LogPtr& log) noexcept;
void Delete_pidfile(const CyclePtr& cycle) noexcept;

extern CyclePtr Cur_cycle;

extern Module Core_module;
extern bool Opt_test_config;
extern bool Opt_quiet_mode;

}

#endif
