#include "base/Atomic.h"

namespace Base {


int AtomicAdd(AtomicInt* c, int v)
{
    return __sync_add_and_fetch(c, v);
}


}  // namespace Base

