#ifndef __BASE_ATOMIC_H__
#define __BASE_ATOMIC_H__


namespace Base {


typedef long AtomicInt;


int AtomicAdd(AtomicInt*, int val);


}  // namespace Base

#endif // __BASE_ATOMIC_H__

