#include <lingx/core/errno.h>
#include <lingx/core/strings.h>  // Strncpy()
#include <system_error>

namespace lnx {

const std::error_category& Error_category_ = std::system_category();

char* Strerror(int err, char* estr, size_t size) noexcept
{
    const std::string& msg = Error_category_.default_error_condition(err)
                                            .message();

    return Strncpy(estr, msg.c_str(), size);
}

}
