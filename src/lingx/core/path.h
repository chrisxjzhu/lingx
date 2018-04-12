#ifndef _LINGX_CORE_PATH_H
#define _LINGX_CORE_PATH_H

#include <string>

namespace lnx {

class Path {
public:
    static constexpr char Separator = '/';

    Path() = default;

    explicit Path(const char* path)
        : path_(path)
    { }

    explicit Path(const std::string& path)
        : path_(path)
    { }

    Path parent_path() const;

    const char* c_str() const noexcept
    { return path_.c_str(); }

    std::string string() const
    { return path_; }

    bool empty() const noexcept
    { return path_.empty(); }

    bool is_absolute() const noexcept
    { return !empty() && path_[0] == Separator; }

    bool is_relative() const noexcept
    { return !is_absolute(); }

    static void Tail_separator(std::string& path);

    static std::string Get_full_name(const std::string& prefix,
                                     const std::string& name);

private:
    const std::string path_;
};

}

#endif
