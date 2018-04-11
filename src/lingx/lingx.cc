#include <lingx/lingx.h>
#include <lingx/config.h>
#include <lingx/core/times.h>
#include <lingx/core/files.h>
#include <lingx/core/log.h>
#include <lingx/core/cycle.h>
#include <lingx/core/module.h>
#include <lingx/core/process_cycle.h>
#include <unistd.h>  // getpid()

namespace lnx {
namespace {

void Show_version_info_() noexcept;
rc_t Get_options_(int argc, const char *const argv[]) noexcept;
rc_t Process_options_(Cycle& cycle);

MConfPtr Core_module_create_conf_(const Cycle& cycle);
const char* Core_module_init_conf_(const Cycle& cycle, const MConfPtr& conf);

bool Opt_show_help_ = false;
bool Opt_show_version_ = false;
const char* Opt_prefix_ = LNX_PREFIX;
const char* Opt_conf_file_ = LNX_CONF_PATH;
const char* Opt_conf_param_ = "";
const char* Opt_signal_ = "";

std::vector<Command> Core_commands_ {
    Command { "daemon", MAIN_CONF }
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
    CORE_MODULE
};

}

int main(int argc, const char* const argv[])
{
    using namespace lnx;

    if (Get_options_(argc, argv) != LNX_OK)
        return 1;

    if (Opt_show_version_) {
        Show_version_info_();

        if (!Opt_test_config)
            return 0;
    }

    Time_init();

    Pid = ::getpid();

    LogPtr log = std::make_shared<Log>(Opt_prefix_);

    Cycle init_cycle;
    pCycle = &init_cycle;

    init_cycle.set_log(log);

    if (Process_options_(init_cycle) != LNX_OK)
        return 1;

    Preinit_modules();

    Cycle cycle;
    if (Init_new_cycle(init_cycle, cycle) != LNX_OK)
        return 1;

    return 0;
}

namespace lnx {
namespace {

void Show_version_info_() noexcept
{
    Write_stderr("lingx version: " LINGX_VER "\n");

    if (Opt_show_help_) {
        Write_stderr(
            "Usage: lingx [-?hvt] [-s signal] [-c filename] "
                         "[-p prefix]\n"
                         "\n"
            "Options:\n"
            "  -?,-h         : this help\n"
            "  -v            : show version and exit\n"
            "  -t            : test configuration and exit\n"
            "  -s signal     : send signal to a master process: "
                               "stop, quit, reopen, reload\n"
            "  -p prefix     : set prefix path (default: " LNX_PREFIX ")\n"
            "  -c filename   : set configuration file (default: " LNX_CONF_PATH
                               ")\n"
            "\n"
        );
    }
}

rc_t Get_options_(int argc, const char *const argv[]) noexcept
{
    for (int i = 1; i < argc; ++i) {
        const char* p = argv[i];

        if (*p++ != '-') {
            Log::Printf(0, "invalid option: \"%s\"", argv[i]);
            return LNX_ERROR;
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

            case 't':
                Opt_test_config = true;
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
                return LNX_ERROR;

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
                return LNX_ERROR;

            case 's':
                if (*p) {
                    Opt_signal_ = p;
                } else if (argv[++i]) {
                    Opt_signal_ = argv[i];
                } else {
                    Log::Printf(0, "option \"-s\" requires parameter");
                    return LNX_ERROR;
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
                return LNX_ERROR;

            default:
                Log::Printf(0, "invalid option: \"%c\"", *(p - 1));
                return LNX_ERROR;
            }
        }

    next:
        continue;
    }

    return LNX_OK;
}

rc_t Process_options_(Cycle& cycle)
{
    cycle.set_prefix(Opt_prefix_);
    cycle.set_conf_file(Opt_conf_file_);
    cycle.set_conf_param(Opt_conf_param_);

    if (Opt_test_config)
        cycle.log()->set_level(Log::Level::INFO);

    return LNX_OK;
}

MConfPtr Core_module_create_conf_(const Cycle& cycle)
{
    std::shared_ptr<CoreConf> ccf = std::make_shared<CoreConf>();
    return ccf;
}

const char* Core_module_init_conf_(const Cycle& cycle, const MConfPtr& conf)
{
    return LNX_CONF_OK;
}

}
}
