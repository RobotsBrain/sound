#ifndef __BASE_MEMORY_POOL_H__
#define __BASE_MEMORY_POOL_H__


#include <cstddef>


#ifndef PREVENT_MACRO_SUBSTITUTION
#define PREVENT_MACRO_SUBSTITUTION
#endif


namespace Base {


/// \class CMemoryPool 非线程安全的内存池类
class CMemoryPool
{
public:
	typedef size_t size_type;
	typedef std::ptrdiff_t difference_type;

public:
    CMemoryPool(unsigned int sz, unsigned int inc);
    ~CMemoryPool();
    void* malloc PREVENT_MACRO_SUBSTITUTION ();
    void free PREVENT_MACRO_SUBSTITUTION (void* ptr);
    bool release_memory();

private:
    CMemoryPool(CMemoryPool const&);            // copy protection
    CMemoryPool& operator=(CMemoryPool const&);	// assign protection

private:
    struct Impl;
    Impl *m_impl;
};


} // namespace Base

#endif // __BASE_MEMORY_POOL_H__


