#ifndef __BASE_MEMORY_PACKET_H__
#define __BASE_MEMORY_PACKET_H__


#include "MemoryBlock.h"


namespace Base {




class CPacket
{
public:
    CPacket() : m_block(NULL)
    {
    }

    CPacket(CPacket const& rhs)
        : m_block(rhs.m_block)
    {
        if (m_block) m_block->AddRef(m_block);
    }

    ~CPacket()
    {
        if (m_block) m_block->Release(m_block);
    }

    bool Empty() const
    {
        return m_block == NULL || m_block->Data(m_block) == NULL;
    }

    void* Data() const
    {
        return m_block ? m_block->Data(m_block) : NULL;
    }

    int DataSize() const
    {
        return m_block ? m_block->DataSize(m_block) : 0;
    }

    void* Info() const
    {
        return m_block ? m_block->Info(m_block) : NULL;
    }

    int InfoSize() const
    {
        return m_block ? m_block->InfoSize(m_block) : 0;
    }

    int Capacity() const
    {
        return m_block ? m_block->Capacity(m_block) : 0;
    }

    bool GetDataRange(int& begin, int& end) const
    {
        return m_block && m_block->GetDataRange(m_block, &begin, &end);
    }

    bool SetDataRange(int begin, int end)
    {
        return m_block && m_block->SetDataRange(m_block, begin, end);
    }

    void Swap(CPacket& rhs)
    {
        IMemoryBlock* t = rhs.m_block;
        rhs.m_block = m_block;
        m_block = t;
    }

    CPacket& operator=(CPacket const& rhs)
    {
        if (&rhs != this)
        {
            CPacket(rhs).Swap(*this);
        }

        return *this;
    }

private:
    IMemoryBlock* m_block;

    friend class CPacketFactory;
};


class CPacketFactory
{
public:
    typedef void* (*MemoryAllocFunc)(size_t bytes, size_t& alloc_bytes);
    typedef void (*MemoryFreeFunc)(void* ptr, size_t alloc_bytes);
    static CPacket Create(size_t capacity, size_t info_size, MemoryAllocFunc allocfunc, MemoryFreeFunc freefunc);

    template <class Deleter, class T1, class T2>
    static CPacket CreateInBuffer(void* buffer, size_t capacity, size_t databytes, T1, T2);
};

} // namespace Base


#endif // __BASE_MEMORY_PACKET_H__

