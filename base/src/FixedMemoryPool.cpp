#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "FixedMemoryPool.h"



#define CAMERA_DEBUG

#ifdef CAMERA_DEBUG
#define DBG(x)  x
#else
#define DBG(x)
#endif // CAMERA_DEBG



namespace Base {
namespace Internal {


/// singleton instance
CFixedMemoryPool* CFixedMemoryPool::Instance()
{
   static CFixedMemoryPool inst;
   return &inst;
}

CFixedMemoryPool::CFixedMemoryPool()
    : m_page_bytes(4*1024)      // 4K
    , m_max_bytes(4*1024*1024)  // 4M
    , m_lists(NULL)
    , m_lists_count(m_max_bytes / m_page_bytes)
    , m_total_bytes(0)
{
    pthread_mutex_init(&m_mutex, NULL);

    m_lists = (NodeList*)malloc(sizeof(NodeList) * m_lists_count);
    memset(m_lists, 0, sizeof(NodeList) * m_lists_count);
}

CFixedMemoryPool::~CFixedMemoryPool()
{
    for (int i = 0; i < m_lists_count; ++i)
    {
        if (m_lists[i].header != NULL)
        {
            DBG(printf("node_index(%d) node_bytes(%dK)\n", i, (i+1)*(m_page_bytes/1024)));
            free_list_all_node(m_lists[i].header);
            m_lists[i].header = NULL;
        }
    }
    free(m_lists);
    pthread_mutex_destroy(&m_mutex);
}

void CFixedMemoryPool::free_list_all_node(FreeNode* header)
{
    FreeNode* node = header;
    while (node != NULL)
    {
        void* ptr = node;
        node = node->next;
        DBG(printf("free memory(%p)\n", ptr));
        free(ptr);
    }
}

/// malloc from pool
void* CFixedMemoryPool::Alloc(size_t bytes, size_t& alloc_bytes)
{
    int list_index = (bytes + m_page_bytes - 1) / m_page_bytes - 1;
    if (list_index < 0 || list_index >= m_lists_count)
    {
        printf("alloc bytes out range! bytes(%d)\n", alloc_bytes);
        alloc_bytes = bytes;
        return malloc(bytes);
    }

    alloc_bytes = (list_index + 1) * m_page_bytes;
    NodeList& list = m_lists[list_index];
    if (list.header == NULL)
    {
        void* ptr = malloc(alloc_bytes);
        m_total_bytes += alloc_bytes;
        DBG(printf("alloc memory(%p), alloc_bytes(%dK), total_bytes(%dK)\n", ptr, alloc_bytes/1024, m_total_bytes/1024));
        return ptr;
    }

    pthread_mutex_lock(&m_mutex);
    void* ptr = list.header;
    list.header = list.header->next;
    pthread_mutex_unlock(&m_mutex);

    return ptr;
}

/// free memory
void CFixedMemoryPool::Free(void* ptr, size_t alloc_bytes)
{
    if (ptr == NULL)
    {
        return;
    }

    int list_index = (alloc_bytes + m_page_bytes - 1) / m_page_bytes - 1;
    if (list_index < 0 || list_index >= m_lists_count)
    {
        printf("alloc bytes out range!\n");
        free(ptr);
        return;
    }

    NodeList& list = m_lists[list_index];

    pthread_mutex_lock(&m_mutex);
    FreeNode* node = (FreeNode*)ptr;
    node->next = list.header;
    list.header = node;
    pthread_mutex_unlock(&m_mutex);
}

} // namespace Internal
} // namespace Base

////////////////////////////////////////////////////////////////////////////////


/// allocate memory from fixed pool
extern "C" void* FixedPoolMemoryAlloc(size_t bytes, size_t& alloc_bytes)
{
    return Base::Internal::CFixedMemoryPool::Instance()->Alloc(bytes, alloc_bytes);
}

/// free memory allocated from fixed pool
extern "C" void FixedPoolMemoryFree(void* ptr, size_t alloc_bytes)
{
    Base::Internal::CFixedMemoryPool::Instance()->Free(ptr, alloc_bytes);
}


