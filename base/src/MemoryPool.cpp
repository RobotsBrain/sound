#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "base/MemoryPool.h"


//#define BASE_DEBUG

#ifdef BASE_DEBUG
#define DBG(x)  x
#else
#define DBG(x)
#endif // BASE_DEBUG



namespace {

#ifdef BASE_DEBUG
static void dumphex(void* ptr, int bytes)
{
    unsigned char* p = (unsigned char*)ptr;
    for (int i = 0; i < bytes; ++i)
    {
        printf("%02X ", p[i]);
        if (i % 16 == 15)
        {
            if (i % 32 == 31)
            {
                printf("\n");
            }
            else
            {
                printf(" ");
            }
        }
    }
}
#else
#define dumphex(...)
#endif

} // namespace noname


namespace Base {

typedef long AtomicInt;

int AtomicAdd(AtomicInt* c, int v)
{
    return __sync_add_and_fetch(c, v);
}

static Base::AtomicInt s_all_pool_bytes = 0;

struct CMemoryPool::Impl
{
private:
    int m_cell_size;
    void* m_freelist;
    int m_max_freecount;
    int m_freecount;
    int m_total_bytes;

    void*& nextof(void* cur_cell)
    {
        void** ptr = (void**)cur_cell;
        return *ptr;
    }

    void* alloc_cells()
    {
        int bytes = m_cell_size;
        char* block = (char*)::malloc(bytes);
        m_total_bytes += bytes;
        AtomicAdd(&s_all_pool_bytes, bytes);
        printf("this(%p) cell_size(%d) pool_bytes(%d) all_bytes(%d)\n", this, m_cell_size, m_total_bytes, s_all_pool_bytes);

        nextof(block) = NULL;
        dumphex(block, 512);
        return block;
    }

public:
    Impl(int sz, int inc)
        : m_cell_size(sz)
        , m_freelist(NULL)
        , m_max_freecount(inc)
        , m_freecount(0)
        , m_total_bytes(0)
    {
    }

    void* malloc PREVENT_MACRO_SUBSTITUTION ()
    {
        if (m_freelist == NULL)
        {
            m_freelist = alloc_cells();
            ++m_freecount;
        }

        void* ptr = m_freelist;
        m_freelist = nextof(m_freelist);
        --m_freecount;

        return ptr;
    }

    void free PREVENT_MACRO_SUBSTITUTION (void* ptr)
    {
        if (m_max_freecount > 0 && m_freecount >= m_max_freecount)
        {
            ::free(ptr);
            m_total_bytes -= m_cell_size;
            AtomicAdd(&s_all_pool_bytes, -m_cell_size);
            printf("this(%p) cell_size(%d) pool_bytes(%d) all_bytes(%d)\n", this, m_cell_size, m_total_bytes, s_all_pool_bytes);
        }
        else
        {
            nextof(ptr) = m_freelist;
            m_freelist = ptr;
            ++m_freecount;
        }
    }

    bool release_memory()
    {
        int released_bytes = 0;
        while (m_freelist != NULL)
        {
            void* ptr = m_freelist;
            m_freelist = nextof(m_freelist);
            ::free(ptr);
            released_bytes += m_cell_size;
        }

        if (released_bytes > 0)
        {
            m_total_bytes -= released_bytes;
            AtomicAdd(&s_all_pool_bytes, -released_bytes);
            printf("this(%p) cell_size(%d) pool_bytes(%d) all_bytes(%d)\n", this, m_cell_size, m_total_bytes, s_all_pool_bytes);
            return true;
        }

        return false;
    }
};


CMemoryPool::CMemoryPool(unsigned int sz, unsigned int inc)
{
    m_impl = new Impl(sz, inc);
}

CMemoryPool::~CMemoryPool()
{
    delete m_impl;
}

void* CMemoryPool::malloc PREVENT_MACRO_SUBSTITUTION ()
{
    return m_impl->malloc();
}

void CMemoryPool::free PREVENT_MACRO_SUBSTITUTION (void* ptr)
{
    m_impl->free(ptr);
}

bool CMemoryPool::release_memory()
{
    return m_impl->release_memory();
}

} // namespace Base


