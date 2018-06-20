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

/// ���캯��
/// \param [in] increase ����ÿ�ζ�̬�����Ĵ�С
CBuffer::CBuffer(int increase)
{
    memset(mInternal, 0, sizeof(mInternal));
    int inc = (increase > 4096) ? 4096 : increase;
    inc = (inc <= 0) ? 64 : inc;
    ToBufferInternal(mInternal).increase = inc;
}

/// ��������
CBuffer::~CBuffer()
{
    Clear();
}

/// Ԥ���仺������, ���ԭ������������, ������ڴ����·����Լ��ڴ渴�Ʋ���
/// \param capacity �µĻ�������
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

/// ������Ч���ݳ���, ���ԭ������������, ������ڴ����·����Լ��ڴ渴�Ʋ���
/// \param size �µ���Ч���ݳ���
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

/// ��β��׷������, ���ԭ������������, ������ڴ����·����Լ��ڴ渴�Ʋ���
/// \param buffer ׷�ӵ�����ָ��
/// \param length ׷�ӵ����ݳ���
/// \return ʵ��д�������
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

/// ȡ���ݵ�ַ, PutBuffer Resize Reserve ��������ɵ�ַʧЧ
void* CBuffer::GetBuffer() const
{
    BufferInternal const& internal = ToBufferInternal(mInternal);
    return (internal.smallsize >= 0) ? (void*)internal.storage.small.data : (void*)internal.storage.huge.data;
}

/// ȡ��Ч���ݳ���
int CBuffer::Size() const
{
    BufferInternal const& internal = ToBufferInternal(mInternal);
    return (internal.smallsize >= 0) ? internal.smallsize : internal.storage.huge.size;
}

/// ȡ��������
int CBuffer::Capacity() const
{
    BufferInternal const& internal = ToBufferInternal(mInternal);
    return (internal.smallsize >= 0) ? sizeof(internal.storage.small.data): internal.storage.huge.capacity;
}

/// �Ƿ�Ϊ��
bool CBuffer::Empty() const
{
    BufferInternal const& internal = ToBufferInternal(mInternal);
    return (internal.smallsize == 0) || ((internal.smallsize < 0) && (internal.storage.huge.size == 0));
}

/// �ͷŻ���, ����Ϊ�ն���
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

