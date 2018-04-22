#ifndef _LINGX_CORE_OS_IO_H
#define _LINGX_CORE_OS_IO_H

#include <lingx/core/common.h>

namespace lnx {

typedef ssize_t (*Recv_pt)(Connection* c, void* buf, size_t size);
typedef ssize_t (*Send_pt)(Connection* c, const void* buf, size_t size);

struct Os_io_t {
    Recv_pt  recv;
    Send_pt  send;
    int      flags;
};

ssize_t Unix_recv(Connection* c, void* buf, size_t size) noexcept;
ssize_t Unix_send(Connection* c, const void* buf, size_t size) noexcept;

extern Os_io_t Os_io;

}

#endif
