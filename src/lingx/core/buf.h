#ifndef _LINGX_CORE_BUF_H
#define _LINGX_CORE_BUF_H

namespace lnx {

class File;

struct Buf {
    char*     pos;
    char*     last;
    off_t     file_pos;
    off_t     file_last;

    char*     start;
    char*     end;
    void*     tag;
    File*     file;
    Buf*      shadow;

    /* the buf's content could be changed */
    unsigned  temporary:1;

    /*
     * the buf's content is in a memory cache or in a read only memory
     * and must not be changed
     */
    unsigned  memory:1;

    /* the buf's content is mmap()ed and must not be changed */
    unsigned  mmap:1;

    unsigned  recycled:1;
    unsigned  in_file:1;
    unsigned  flush:1;
    unsigned  sync:1;
    unsigned  last_buf:1;
    unsigned  last_in_chain:1;

    unsigned  last_shadow:1;
    unsigned  temp_file:1;

    /* STUB */
    int       num;
};

}

#endif
