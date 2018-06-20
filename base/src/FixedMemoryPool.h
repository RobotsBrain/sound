
#ifndef __DEVICE_MEMORY_POOL_H__
#define __DEVICE_MEMORY_POOL_H__


#include <stddef.h>
#include <pthread.h>


namespace Base {
namespace Internal {


class CFixedMemoryPool
{
    CFixedMemoryPool(CFixedMemoryPool const&);
    CFixedMemoryPool& operator=(CFixedMemoryPool const&);
    CFixedMemoryPool();
    ~CFixedMemoryPool();

public:
    /// singleton instance
    static CFixedMemoryPool* Instance();

    /// malloc from pool
    void* Alloc(size_t bytes, size_t& alloc_bytes);

    /// free memory
    void Free(void* ptr, size_t alloc_bytes);

private:
    struct FreeNode
    {
        FreeNode* next;
    };

    struct NodeList
    {
        FreeNode* header;
    };

private:
    void free_list_all_node(FreeNode* header);

private:
    size_t  m_page_bytes;
    size_t  m_max_bytes;

    NodeList*       m_lists;
    int             m_lists_count;
    pthread_mutex_t m_mutex;

    size_t  m_total_bytes;
};


} // namespace Internal
} // namespace Base

#endif // __DEVICE_MEMORY_POOL_H__
