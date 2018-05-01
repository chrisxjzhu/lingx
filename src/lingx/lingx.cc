#include <lingx/lingx.h>
#include <lingx/config.h>
#include <lingx/core/times.h>
#include <lingx/core/files.h>
#include <lingx/core/log.h>
#include <lingx/core/path.h>
#include <lingx/core/cycle.h>
#include <lingx/core/module.h>
#include <lingx/core/conf_file.h>
#include <lingx/core/os.h>  // Os_init()
#include <lingx/core/process.h>  // Init_signals()
#include <lingx/core/process_cycle.h>
#include <lingx/core/daemon.h>   // Daemonize()
#include <unistd.h>  // getpid()
#include <cstddef>   // offsetof()

namespace lnx {
namespace {

void Show_version_info_() noexcept;
rc_t Get_options_(int argc, const char *const argv[]) noexcept;
rc_t Process_options_(const CyclePtr& cycle);

MConfPtr Core_module_create_conf_(Cycle* cycle);
const char* Core_module_init_conf_(Cycle* cycle, MConf* conf);

bool Opt_show_help_      = false;
bool Opt_show_version_   = false;
bool Opt_show_configure_ = false;
const char* Opt_prefix_     = LNX_PREFIX;
const char* Opt_conf_file_  = LNX_CONF_PATH;
const char* Opt_conf_param_ = "";
const char* Opt_signal_     = "";

std::vector<Command> Core_commands_ {
    Command {
        "daemon",
         MAIN_CONF|CONF_FLAG,
         Set_flag_slot,
         offsetof(CoreConf, daemon)
    },
    Command {
        "master_process",
         MAIN_CONF|CONF_FLAG,
         Set_flag_slot,
         offsetof(CoreConf, master)
    }
};

CoreModuleCtx Core_module_ctx_ {
    "core",
    Core_module_create_conf_,
    Core_module_init_conf_
};

}

Module Core_module {
    "lnx_core_module",
    Core_module_ctx_,
    Core_commands_,
    CORE_MODULE,
    nullptr,
    nullptr
};

}

int main(int argc, const char* const argv[])
{
    using namespace lnx;

    if (Get_options_(argc, argv) != OK)
        return 1;

    if (Opt_show_version_) {
        Show_version_info_();

        if (!Opt_test_config)
            return 0;
    }

    Time_init();

    Pid = ::getpid();

    LogPtr log = Init_new_log(Opt_prefix_);

    CyclePtr init_cycle = std::make_shared<Cycle>();
    Cur_cycle = init_cycle;

    init_cycle->set_log(log.get());

    if (Process_options_(init_cycle) != OK)
        return 1;

    if (Os_init(log.get()) != OK)
        return 1;

    Preinit_modules();

    CyclePtr cycle = Init_new_cycle(init_cycle);
    if (cycle == nullptr)
        return 1;

    if (Opt_test_config) {
        if (!Opt_quiet_mode) {
            Log::Printf(0, "configuration file %s test is successful",
                        cycle->conf_file().c_str());
        }

        return 0;
    }

    if (Opt_signal_[0])
        return Signal_process(cycle, Opt_signal_);

    Os_status(cycle->log());

    Cur_cycle = cycle;

    std::shared_ptr<CoreConf> ccf = Get_conf(CoreConf, cycle, Core_module);

    if (ccf->master && Process_type == PROCESS_SINGLE)
        Process_type = PROCESS_MASTER;

    if (Init_signals(cycle->log()) != OK)
        return 1;

    if (ccf->daemon) {
        if (Daemonize(cycle->log()) != OK)
            return 1;

        Daemonized = true;
    }

    if (Create_pidfile(ccf->pid_path, cycle->log()) != OK)
        return 1;

    if (cycle->log_redirect_stderr() != OK)
        return 1;

    if (log->file()->fd() != STDERR_FILENO) {
        if (log->file()->close() == -1) {
            Log_error(cycle->log(), Log::ALERT, errno,
                      "close() built-in log failed");
        }
    }

    Use_stderr = false;

    if (Process_type == PROCESS_SINGLE)
        Single_process_cycle(cycle);
    else
        Master_process_cycle(cycle);

    return 0;
}

namespace lnx {
namespace {

void Show_version_info_() noexcept
{
    Write_stderr("lingx version: " LINGX_VER "\n");

    if (Opt_show_help_) {
        Write_stderr(
            "Usage: lingx [-?hvVtq] [-s signal] [-c filename] "
                         "[-p prefix] [-g directives]\n"
                         "\n"
            "Options:\n"
            "  -?,-h         : this help\n"
            "  -v            : show version and exit\n"
            "  -V            : show version and configure options then exit\n"
            "  -t            : test configuration and exit\n"
            "  -q            : suppress non-error messages "
                               "during configuration testing\n"
            "  -s signal     : send signal to a master process: "
                               "stop, quit, reopen, reload\n"
            "  -p prefix     : set prefix path (default: " LNX_PREFIX ")\n"
            "  -c filename   : set configuration file (default: " LNX_CONF_PATH
                               ")\n"
            "  -g directives : set global directives out of configuration file\n"
            "\n"
        );
    }

    if (Opt_show_configure_) {
        Write_stderr("built by " LNX_COMPILER "\n");
        Write_stderr("configure arguments:" LNX_CONFIGURE "\n");
    }
}

rc_t Get_options_(int argc, const char *const argv[]) noexcept
{
    for (int i = 1; i < argc; ++i) {
        const char* p = argv[i];

        if (*p++ != '-') {
            Log::Printf(0, "invalid option: \"%s\"", argv[i]);
            return ERROR;
        }

        while (*p) {
            switch (*p++) {
            case '?':
            case 'h':
                Opt_show_version_ = true;
                Opt_show_help_ = true;
                break;

            case 'v':
                Opt_show_version_ = true;
                break;

            case 'V':
                Opt_show_version_ = true;
                Opt_show_configure_ = true;
                break;

            case 't':
                Opt_test_config = true;
                break;

            case 'q':
                Opt_quiet_mode = true;
                break;

            case 'p':
                if (*p) {
                    Opt_prefix_ = p;
                    goto next;
                }

                if (argv[++i]) {
                    Opt_prefix_ = argv[i];
                    goto next;
                }

                Log::Printf(0, "option \"-p\" requires directory name");
                return ERROR;

            case 'c':
                if (*p) {
                    Opt_conf_file_ = p;
                    goto next;
                }

                if (argv[++i]) {
                    Opt_conf_file_ = argv[i];
                    goto next;
                }

                Log::Printf(0, "option \"-c\" requires file name");
                return ERROR;

            case 'g':
                if (*p) {
                    Opt_conf_param_ = p;
                    goto next;
                }

                if (argv[++i]) {
                    Opt_conf_param_ = argv[i];
                    goto next;
                }

                Log::Printf(0, "option \"-g\" requires parameter");
                return ERROR;

            case 's':
                if (*p) {
                    Opt_signal_ = p;
                } else if (argv[++i]) {
                    Opt_signal_ = argv[i];
                } else {
                    Log::Printf(0, "option \"-s\" requires parameter");
                    return ERROR;
                }

                if (std::strcmp(Opt_signal_, "stop") == 0
                    || std::strcmp(Opt_signal_, "quit") == 0
                    || std::strcmp(Opt_signal_, "reopen") == 0
                    || std::strcmp(Opt_signal_, "reload") == 0)
                {
                    Process_type = PROCESS_SIGNALLER;
                    goto next;
                }

                Log::Printf(0, "invalid option: \"-s %s\"", Opt_signal_);
                return ERROR;

            default:
                Log::Printf(0, "invalid option: \"%c\"", *(p - 1));
                return ERROR;
            }
        }

    next:
        continue;
    }

    return OK;
}

rc_t Process_options_(const CyclePtr& cycle)
{
    cycle->set_prefix(Opt_prefix_);
    cycle->set_conf_file(Path::Get_full_name(Opt_prefix_, Opt_conf_file_));
    cycle->set_conf_param(Opt_conf_param_);

    if (Opt_test_config)
        const_cast<Log*>(cycle->log())->set_level(Log::INFO);

    return OK;
}

MConfPtr Core_module_create_conf_(Cycle*)
{
    std::shared_ptr<CoreConf> ccf = std::make_shared<CoreConf>();

    ccf->daemon = UNSET;
    ccf->master = UNSET;

    return ccf;
}

const char* Core_module_init_conf_(Cycle* cycle, MConf* conf)
{
    CoreConf* ccf = static_cast<CoreConf*>(conf);

    Conf_init_value(ccf->daemon, ON);
    Conf_init_value(ccf->master, ON);

    if (ccf->pid_path.empty())
        ccf->pid_path = LNX_PID_PATH;

    ccf->pid_path = Path::Get_full_name(cycle->prefix(), ccf->pid_path);

    return CONF_OK;
}

}
}
