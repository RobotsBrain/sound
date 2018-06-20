#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "base/StaticAssert.h"
#include "base/Buffer.h"



//#define BASE_DEBUG

#ifdef BASE_DEBUG
#define DBG(x)  x
#else
#define DBG(x)
#endif // BASE_DEBUG




namespace Base {


enum {InternalSize = sizeof(intptr_t) * 8};


struct BufferInternal
{
    union {
        struct {
            char data[InternalSize - sizeof(intptr_t)];
        } small;

        struct {
            void* data;
            int capacity;
            int size;
        } huge;

    } storage;

    int16_t smallsize;  ///< negative means use huge storage
    int16_t increase;
};

template <size_t N>
inline BufferInternal& ToBufferInternal(intptr_t (&a)[N])
{
    BASE_STATIC_ASSERT(sizeof(a) >= sizeof(BufferInternal));
    BufferInternal* p = reinterpret_cast<BufferInternal*>((void*)&a);
    return *p;
}

template <size_t N>
inline BufferInternal const& ToBufferInternal(intptr_t const (&a)[N])
{
    BASE_STATIC_ASSERT(sizeof(a) >= sizeof(BufferInternal));
    BufferInternal const* p = reinterpret_cast<BufferInternal const*>((void*)&a);
    return *p;
}

/// 构造函数
/// \param [in] increase 设置每次动态增长的大小
CBuffer::CBuffer(int increase)
{
    memset(mInternal, 0, sizeof(mInternal));
    int inc = (increase > 4096) ? 4096 : increase;
    inc = (inc <= 0) ? 64 : inc;
    ToBufferInternal(mInternal).increase = inc;
}

/// 析构函数
CBuffer::~CBuffer()
{
    Clear();
}

/// 预分配缓存容量, 如果原缓存容量不够, 会造成内存重新分配以及内存复制操作
/// \param capacity 新的缓存容量
void CBuffer::Reserve(int capacity)
{
    if (capacity > Capacity())
    {
        BufferInternal& internal = ToBufferInternal(mInternal);
        if (capacity > (int)sizeof(internal.storage.small.data))
        {
            bool is_samll = (internal.smallsize >= 0);
            void* olddata = is_samll ? internal.storage.small.data : internal.storage.huge.data;
            int oldsize = is_samll ? internal.smallsize : internal.storage.huge.size;

            int newcapacity = (capacity + internal.increase - 1) / internal.increase * internal.increase;
            void* newdata = malloc(newcapacity);
            DBG(printf("malloc(%p)\n", newdata));
            memcpy(newdata, olddata, oldsize);

            if (!is_samll)
            {
                free(olddata);
            }

            internal.smallsize = -1;
            internal.storage.huge.data = newdata;
            internal.storage.huge.size = oldsize;
            internal.storage.huge.capacity = newcapacity;
        }
    }
}

/// 设置有效数据长度, 如果原缓存容量不够, 会造成内存重新分配以及内存复制操作
/// \param size 新的有效数据长度
void CBuffer::Resize(int bytes)
{
    Reserve(bytes);

    BufferInternal& internal = ToBufferInternal(mInternal);
    if (internal.smallsize < 0)
    {
        internal.storage.huge.size = bytes;
    }
    else
    {
        internal.smallsize = bytes;
    }
}

/// 向尾部追加数据, 如果原缓存容量不够, 会造成内存重新分配以及内存复制操作
/// \param buffer 追加的数据指针
/// \param length 追加的数据长度
/// \return 实际写入的数据
int CBuffer::PutBuffer(void const* buffer, int bytes)
{
    if (buffer == NULL || bytes <= 0)
    {
        return 0;
    }

    int oldsize = Size();
    int newsize = oldsize + bytes;
    DBG(printf("this(%p) data(%p) size(%d)\n", this, GetBuffer(), Size()));
    Resize(newsize);
    DBG(printf("this(%p) data(%p) size(%d)\n", this, GetBuffer(), Size()));
    memcpy((char*)GetBuffer() + oldsize, buffer, bytes);
    DBG(printf("this(%p) data(%p) size(%d)\n", this, GetBuffer(), Size()));

    return bytes;
}

/// 取数据地址, PutBuffer Resize Reserve 操作会造成地址失效
void* CBuffer::GetBuffer() const
{
    BufferInternal const& internal = ToBufferInternal(mInternal);
    return (internal.smallsize >= 0) ? (void*)internal.storage.small.data : (void*)internal.storage.huge.data;
}

/// 取有效数据长度
int CBuffer::Size() const
{
    BufferInternal const& internal = ToBufferInternal(mInternal);
    return (internal.smallsize >= 0) ? internal.smallsize : internal.storage.huge.size;
}

/// 取缓存容量
int CBuffer::Capacity() const
{
    BufferInternal const& internal = ToBufferInternal(mInternal);
    return (internal.smallsize >= 0) ? sizeof(internal.storage.small.data): internal.storage.huge.capacity;
}

/// 是否为空
bool CBuffer::Empty() const
{
    BufferInternal const& internal = ToBufferInternal(mInternal);
    return (internal.smallsize == 0) || ((internal.smallsize < 0) && (internal.storage.huge.size == 0));
}

/// 释放缓存, 重置为空对象
void CBuffer::Clear()
{
    BufferInternal& internal = ToBufferInternal(mInternal);
    if (internal.smallsize < 0)
    {
        DBG(printf("free(%p)\n", internal.storage.huge.data));
        free(internal.storage.huge.data);
        internal.smallsize = 0;
    }
}


} // namespace Base

