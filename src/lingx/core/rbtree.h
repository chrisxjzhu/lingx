#ifndef _LINGX_CORE_RBTREE_H
#define _LINGX_CORE_RBTREE_H

#include <set> 

namespace lnx {

typedef ulong rbkey_t;

struct RBNode {
    rbkey_t  key;
};

inline bool Rbnode_ptr_cmp(const RBNode* a, const RBNode* b) noexcept
{ return a->key < b->key; };

using RBTree = std::set<RBNode*, decltype(Rbnode_ptr_cmp)*>;

}

#endif
