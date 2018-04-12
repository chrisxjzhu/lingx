#include <lingx/core/path.h>

namespace lnx {

Path Path::parent_path() const
{
    std::string path = path_;

    size_t pos = path_.rfind(Separator);
    if (pos == std::string::npos)
        return Path("");

    while (pos != 0 && path_[pos] == Separator)
        --pos;

    return Path(path_.substr(0, pos + 1));
}

void Path::Tail_separator(std::string& path)
{
    if (!path.empty() && path.back() != Separator)
        path += Separator;
}

std::string Path::Get_full_name(const std::string& prefix,
                                const std::string& name)
{
    if (name.c_str()[0] == Separator)
        return name;

    std::string fullname = prefix;
    Tail_separator(fullname);
    fullname += name;

    return fullname;
}

}
