#include <lingx/core/conf_file.h>
#include <lingx/core/module.h>
#include <lingx/core/cycle.h>
#include <lingx/core/strings.h>
#include <lingx/core/error.h>
#include <lingx/core/log.h>
#include <lingx/core/buf.h>
#include <cstdarg>
#include <cassert>
#include <fcntl.h>

namespace lnx {

const size_t MAX_CONF_BUFFER = 4096;

int Argument_number_[] = {
    CONF_NOARGS,
    CONF_TAKE1,
    CONF_TAKE2,
    CONF_TAKE3,
    CONF_TAKE4,
    CONF_TAKE5,
    CONF_TAKE6,
    CONF_TAKE7
};

enum {
    CONF_BLOCK_START = 1,
    CONF_BLOCK_DONE  = 2,
    CONF_FILE_DONE   = 3
};

const char* Conf::param() noexcept
{
    std::string_view param = cycle_->conf_param();
    if (param.size() == 0)
        return CONF_OK;

    Buf b;
    b.start = const_cast<char*>(param.data());
    b.pos = b.start;
    b.last = b.start + param.size();
    b.end = b.last;
    b.temporary = 1;

    ConfFile conf_file;
    conf_file.buffer = &b;
    conf_file.line = 0;

    conf_file_ = &conf_file;

    const char* rv = parse("");

    conf_file_ = nullptr;

    return rv;
}

const char* Conf::parse(const std::string& filename) noexcept
{
    enum {
        parse_file = 0,
        parse_block,
        parse_param
    } type;

    ConfFile* prev = conf_file_;
    ConfFile  conf_file;

    Buf buf;
    std::unique_ptr<char[]> buffer;

    auto dtor = [this, prev](const char*) { conf_file_ = prev; };
    std::unique_ptr<const char, decltype(dtor)> cleanup("", dtor);

    if (!filename.empty()) {
        if (conf_file.file.open(filename.c_str(), O_RDONLY, 0) == -1) {
            Log_error(log_, Log::EMERG, errno,
                      "open() \"%s\" failed", filename.c_str());
            return CONF_ERROR;
        }

        conf_file_ = &conf_file;

        if (conf_file_->file.stat() == -1) {
            Log_error(log_, Log::EMERG, errno,
                      "fstat() \"%s\" failed", filename.c_str());
            return CONF_ERROR;
        }

        buffer = std::make_unique<char[]>(MAX_CONF_BUFFER);

        buf.start = buffer.get();
        buf.pos = buf.start;
        buf.last = buf.start;
        buf.end = buf.last + MAX_CONF_BUFFER;
        buf.temporary = 1;

        conf_file_->buffer = &buf;
        conf_file_->line = 1;

        type = parse_file;

    } else if (conf_file_->file.fd() != -1) {
        type = parse_block;
    } else {
        type = parse_param;
    }

    for ( ;; ) {
        int rc = read_token_();

        /*
         * read_token_() may return
         *
         *    ERROR             there is error
         *    OK                the token terminated by ";" was found
         *    CONF_BLOCK_START  the token terminated by "{" was found
         *    CONF_BLOCK_DONE   the "}" was found
         *    CONF_FILE_DONE    the configuration file is done
         */

        if (rc == ERROR)
            return CONF_ERROR;

        if (rc == CONF_BLOCK_DONE) {
            if (type != parse_block) {
                log_error(Log::EMERG, "unexpected \"}\"");
                return CONF_ERROR;
            }

            return CONF_OK;
        }

        if (rc == CONF_FILE_DONE) {
            if (type == parse_block) {
                log_error(Log::EMERG, "unexpected end of file, expecting \"}\"");
                return CONF_ERROR;
            }

            return CONF_OK;
        }

        if (rc == CONF_BLOCK_START) {
            if (type == parse_param) {
                log_error(Log::EMERG, "block directives are not supported "
                                      "in -g option");
                return CONF_ERROR;
            }
        }

        /* rc == NGX_OK || rc == NGX_CONF_BLOCK_START */

        /* TODO: if (cf->handler) */

        if (handle_(rc) == ERROR)
            return CONF_ERROR;
    }

    return CONF_OK;
}

int Conf::read_token_() noexcept
{
    char    *start, ch;
    off_t    file_size;
    bool     found, need_space, last_space, sharp_comment, variable;
    bool     quoted, s_quoted, d_quoted;
    uint     start_line;
    Buf     *b;

    found = false;
    need_space = false;
    last_space = true;
    sharp_comment = false;
    variable = false;
    quoted = false;
    s_quoted = false;
    d_quoted = false;

    args_.clear();

    b = conf_file_->buffer;
    start = b->pos;
    start_line = conf_file_->line;

    file_size = conf_file_->file.info().st_size;

    for (;;) {
        if (b->pos >= b->last) {
            if (conf_file_->file.offset() >= file_size) {
                if (args_.size() || !last_space) {
                    if (conf_file_->file.fd() == -1) {
                        log_error(Log::EMERG, "unexpected end of parameter, "
                                              "expecting \";\"");
                        return ERROR;
                    }

                    log_error(Log::EMERG, "unexpected end of file, "
                                          "expecting \";\" or \"}\"");
                    return ERROR;
                }

                return CONF_FILE_DONE;
            }

            size_t len = b->pos - start;

            if (len == MAX_CONF_BUFFER) {
                conf_file_->line = start_line;

                if (d_quoted)
                    ch = '"';
                else if (s_quoted)
                    ch = '\'';
                else {
                    log_error(Log::EMERG, "too long parameter \"%.*s...\" "
                                          "started", 10, start);
                    return ERROR;
                }

                log_error(Log::EMERG, "too long parameter, probably missing "
                                      "terminating \"%c\" character", ch);
                return ERROR;
            }

            if (len)
                std::memmove(b->start, start, len);

            ssize_t size = (ssize_t) (file_size - conf_file_->file.offset());

            if (size > b->end - (b->start + len))
                size = b->end - (b->start + len);

            ssize_t n = conf_file_->file.read(b->start + len, size,
                                              conf_file_->file.offset());
            if (n == -1) {
                Log_error(log_, Log::EMERG, errno, "read() \"%s\" failed",
                                            conf_file_->file.name().c_str());
                return ERROR;
            }

            if (n != size) {
                log_error(Log::EMERG, "read() returned only %z "
                                      "bytes instead of %z", n, size);
                return ERROR;
            }

            b->pos = b->start + len;
            b->last = b->pos + n;
            start = b->start;
        }

        ch = *b->pos++;

        if (ch == '\n') {
            conf_file_->line++;

            if (sharp_comment)
                sharp_comment = false;
        }

        if (sharp_comment)
            continue;

        if (quoted) {
            quoted = false;
            continue;
        }

        if (need_space) {
            if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
                last_space = true;
                need_space = false;
                continue;
            }

            if (ch == ';')
                return OK;

            if (ch == '{')
                return CONF_BLOCK_START;

            if (ch == ')') {
                last_space = true;
                need_space = false;
            } else {
                log_error(Log::EMERG, "unexpected \"%c\"", ch);
                return ERROR;
            }
        }

        if (last_space) {
            if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
                continue;

            start = b->pos - 1;
            start_line = conf_file_->line;

            switch (ch) {
            case ';':
            case '{':
                if (args_.empty()) {
                    log_error(Log::EMERG, "unexpected \"%c\"", ch);
                    return ERROR;
                }

                if (ch == '{')
                    return CONF_BLOCK_START;

                return OK;

            case '}':
                if (args_.size()) {
                    log_error(Log::EMERG, "unexpected \"}\"");
                    return ERROR;
                }

                return CONF_BLOCK_DONE;

            case '#':
                sharp_comment = true;
                continue;

            case '\\':
                quoted = true;
                last_space = false;
                continue;

            case '"':
                start++;
                d_quoted = true;
                last_space = false;
                continue;

            case '\'':
                start++;
                s_quoted = true;
                last_space = false;
                continue;

            default:
                last_space = false;
            }
        } else {
            if (ch == '{' && variable)
                continue;

            variable = false;

            if (ch == '\\') {
                quoted = true;
                continue;
            }

            if (ch == '$') {
                variable = true;
                continue;
            }

            if (d_quoted) {
                if (ch == '"') {
                    d_quoted = false;
                    need_space = true;
                    found = true;
                }
            } else if (s_quoted) {
                if (ch == '\'') {
                    s_quoted = false;
                    need_space = true;
                    found = true;
                }
            } else if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || 
                       ch == ';' || ch == '{')
            {
                last_space = true;
                found = true;
            }

            if (found) {
                std::string word(b->pos - 1 - start, '\0');
                auto dst = word.begin();

                for (char* src = start; src < b->pos - 1; ) {
                    if (*src == '\\') {
                        switch (src[1]) {
                        case '"':
                        case '\'':
                        case '\\':
                            src++;
                            break;

                        case 't':
                            *dst++ = '\t';
                            src += 2;
                            continue;

                        case 'r':
                            *dst++ = '\r';
                            src += 2;
                            continue;

                        case 'n':
                            *dst++ = '\n';
                            src += 2;
                            continue;
                        }
                    }

                    *dst++ = *src++;
                }

                args_.push_back(std::move(word));

                if (ch == ';')
                    return OK;

                if (ch == '{')
                    return CONF_BLOCK_START;

                found = false;
            }
        }
    }
}

rc_t Conf::handle_(int last) noexcept
{
    assert(last == OK || last == CONF_BLOCK_START);
    assert(args_.size());

    std::string_view name = args_[0];

    bool found = false;

    for (const Module& mod : cycle_->modules()) {
        for (const Command& cmd : mod.commands()) {
            if (name != cmd.name)
                continue;

            found = true;

            if (mod.type() != module_type_)
                continue;

            /* is the directive's location right ? */
            if (!(cmd.type & cmd_type_))
                continue;

            if (!(cmd.type & CONF_BLOCK) && last == CONF_BLOCK_START) {
                log_error(Log::EMERG, "directive \"%s\" is not terminated "
                                      "by \";\"", name.data());
                return ERROR;
            }

            if ((cmd.type & CONF_BLOCK) && last != CONF_BLOCK_START) {
                log_error(Log::EMERG,
                         "directive \"%s\" has no opening \"{\"", name.data());
                return ERROR;
            }

            /* is the directive's argument count right ? */
            if (!(cmd.type & CONF_ANY)) {
                if (cmd.type & CONF_FLAG) {
                    if (args_.size() != 2)
                        goto invalid;
                } else if (cmd.type & CONF_1MORE) {
                    if (args_.size() < 2)
                        goto invalid;
                } else if (cmd.type & CONF_2MORE) {
                    if (args_.size() < 3)
                        goto invalid;
                } else if (args_.size() > CONF_MAX_ARGS) {
                    goto invalid;
                } else if (!(cmd.type & Argument_number_[args_.size() - 1])) {
                    goto invalid;
                }
            }

            /* set up the directive's configuration context */

            MConfPtr dummy;

            uint idx = (cmd.type & MAIN_CONF) ? mod.index()
                                              : mod.ctx_index();
            MConfPtr& conf = ctxs_ ? (*ctxs_)[idx] : dummy;

            /* TODO: catch exceptions? */
            const char* rv = cmd.set(*this, cmd, conf);

            if (rv == CONF_OK)
                return OK;

            if (rv == CONF_ERROR)
                return ERROR;

            log_error(Log::EMERG, "\"%s\" directive %s", name.data(), rv);

            return ERROR;
        }
    }

    if (found) {
        log_error(Log::EMERG, "\"%s\" directive is not allowed here",
                               name.data());
        return ERROR;
    }

    log_error(Log::EMERG, "unknown directive \"%s\"", name.data());

    return ERROR;

invalid:
    log_error(Log::EMERG, "invalid number of arguments in \"%s\" directive",
                           name.data());
    return ERROR;
}

void Conf::log_error(Log::Level lvl, const char* fmt, ...) const noexcept
{
    char errstr[MAX_CONF_ERRSTR];
    const char* const last = errstr + sizeof(errstr);
    char* p = errstr;

    va_list args;
    va_start(args, fmt);
    p = Vslprintf(p, last, fmt, args);
    va_end(args);

    if (!conf_file_) {
        Log_error(log_, lvl, 0, "%.*s", p - errstr, errstr);
        return;
    }

    if (conf_file_->file.fd() == -1) {
        Log_error(log_, lvl, 0, "%.*s in command line", p - errstr, errstr);
        return;
    }

    Log_error(log_, lvl, 0, "%.*s in %s:%u", p - errstr, errstr,
              conf_file_->file.name().c_str(), conf_file_->line);
}

const char* Conf::parse_bool_arg(const char* cmd, bool* val) const noexcept
{
    if (*val)
        return "is duplicate";

    assert(args_.size() == 2);

    const char* arg = args_[1].c_str();

    if (std::strcmp(arg, "on") == 0)
        *val = true;
    else if (std::strcmp(arg, "off") == 0)
        *val = false;
    else {
        log_error(Log::EMERG, "invalid value \"%s\" in \"%s\" directive, "
                              "it must be \"on\" or \"off\"", arg, cmd);
        return CONF_ERROR;
    }

    return CONF_OK;
}

const char* Set_bool_slot(const Conf& cf, const Command& cmd, MConfPtr& conf)
{
    char* p  = (char*) conf.get();
    bool* bp = (bool*) (p + cmd.offset);

    const char* rv = cf.parse_bool_arg(cmd.name.data(), bp);
    if (rv != CONF_OK)
        return rv;

    /* TODO: cmd->post */
    return rv;
}

}
