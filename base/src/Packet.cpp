#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "base/MemoryBlock.h"
#include "base/Packet.h"
#include "FixedMemoryPool.h"


typedef Base::CPacketFactory::MemoryAllocFunc MemoryAllocFunc;
typedef Base::CPacketFactory::MemoryFreeFunc MemoryFreeFunc;


namespace Base {
namespace Internal {


////////////////////////////////////////////////////////////////////////////////

struct FixedPoolBlockPriv
{
    MemoryFreeFunc free_func;
    long ref_count;
    size_t alloc_bytes;
    size_t info_size;
    size_t data_capacity;
    int data_begin;
    int data_end;
};

// block memory struct
// +------------+------+------+------------------------------------------------+
// | block_func | priv | info | data                                           |
// +------------+------+------+------------------------------------------------+
//

static IMemoryBlock* FixedPoolBlock_create_priv(MemoryAllocFunc allocfunc, MemoryFreeFunc freefunc, size_t capacity, size_t info_size)
{
    size_t block_func_bytes = sizeof(IMemoryBlock) - sizeof(((IMemoryBlock*)0)->priv);
    size_t aligned_priv_bytes = (sizeof(FixedPoolBlockPriv) + 0xF) & (~0xF);
    size_t aligned_info_bytes = (info_size + 0xF) & (~0xF);
    size_t aligned_capacity = (capacity + 0xF) & (~0xF);
    size_t bytes = block_func_bytes + aligned_priv_bytes + aligned_info_bytes + aligned_capacity;
    size_t alloc_bytes = 0;
    IMemoryBlock* block = (IMemoryBlock*)allocfunc(bytes, alloc_bytes);
    if (block)
    {
        memset(block, 0, block_func_bytes + aligned_priv_bytes);
        FixedPoolBlockPriv* priv = (FixedPoolBlockPriv*)block->priv;
        priv->free_func     = freefunc;
        priv->ref_count     = 1;
        priv->alloc_bytes   = alloc_bytes;
        priv->info_size     = info_size;
        priv->data_capacity = alloc_bytes - (block_func_bytes + aligned_priv_bytes + aligned_info_bytes);
        priv->data_begin    = 0;
        priv->data_end      = 0;
    }

    return block;
}

static FixedPoolBlockPriv* FixedPoolBlock_priv(IMemoryBlock* thiz)
{
    assert(thiz);
    return (FixedPoolBlockPriv*)thiz->priv;
}

static int FixedPoolBlock_addref(IMemoryBlock* thiz)
{
    assert(thiz);
    FixedPoolBlockPriv* priv = FixedPoolBlock_priv(thiz);
    return __sync_add_and_fetch(&priv->ref_count, 1);
}

static int FixedPoolBlock_release(IMemoryBlock* thiz)
{
    assert(thiz);
    FixedPoolBlockPriv* priv = FixedPoolBlock_priv(thiz);
    int ref_count = __sync_add_and_fetch(&priv->ref_count, -1);
    if (ref_count == 0)
    {
        MemoryFreeFunc freefunc = priv->free_func;
        freefunc(thiz, priv->alloc_bytes);
    }

    return ref_count;
}

static void* FixedPoolBlock_info(struct IMemoryBlock* thiz)
{
    assert(thiz);
    size_t aligned_priv_bytes = (sizeof(FixedPoolBlockPriv) + 0xF) & (~0xF);
    return ((char*)thiz->priv) + aligned_priv_bytes;
}

static void* FixedPoolBlock_data(struct IMemoryBlock* thiz)
{
    assert(thiz);
    FixedPoolBlockPriv* priv = FixedPoolBlock_priv(thiz);
    size_t aligned_priv_bytes = (sizeof(FixedPoolBlockPriv) + 0xF) & (~0xF);
    size_t aligned_info_bytes = (priv->info_size + 0xF) & (~0xF);
    char* buffer = ((char*)thiz->priv) + aligned_priv_bytes + aligned_info_bytes;
    return buffer + priv->data_begin;
}

static int FixedPoolBlock_capacity(struct IMemoryBlock* thiz)
{
    assert(thiz);
    FixedPoolBlockPriv* priv = FixedPoolBlock_priv(thiz);
    return priv->data_capacity;
}

/// 设置有效数据区偏移区间
static int FixedPoolBlock_set_data_offset(struct IMemoryBlock* thiz, int begin, int end)
{
    assert(thiz);
    FixedPoolBlockPriv* priv = FixedPoolBlock_priv(thiz);

    begin = (begin < 0) ? priv->data_begin : begin;
    end = (end < 0) ? priv->data_end : end;

    if (begin > end || end > FixedPoolBlock_capacity(thiz))
    {
        return -1;
    }

    priv->data_begin = begin;
    priv->data_end = end;

    return 0;
}

/// 获取有效数据区间
static int FixedPoolBlock_get_data_offset(struct IMemoryBlock* thiz, int* begin, int* end)
{
    assert(thiz);
    FixedPoolBlockPriv* priv = FixedPoolBlock_priv(thiz);
    if (begin) *begin = priv->data_begin;
    if (end) *end = priv->data_end;
    return 0;
}

static int FixedPoolBlock_data_size(struct IMemoryBlock* thiz)
{
    FixedPoolBlockPriv* priv = FixedPoolBlock_priv(thiz);
    return priv->data_end - priv->data_begin;
}

static int FixedPoolBlock_info_size(struct IMemoryBlock* thiz)
{
    FixedPoolBlockPriv* priv = FixedPoolBlock_priv(thiz);
    return priv->info_size;
}

} // namespace Internal

////////////////////////////////////////////////////////////////////////////////


IMemoryBlock* GeneralMemoryBlockCreate(MemoryAllocFunc allocfunc, MemoryFreeFunc freefunc, size_t capacity, size_t info_size)
{
    IMemoryBlock* block = Base::Internal::FixedPoolBlock_create_priv(allocfunc, freefunc, capacity, info_size);
    if (block)
    {
        block->AddRef       = Base::Internal::FixedPoolBlock_addref;
        block->Release      = Base::Internal::FixedPoolBlock_release;
        block->Capacity     = Base::Internal::FixedPoolBlock_capacity;
        block->SetDataRange = Base::Internal::FixedPoolBlock_set_data_offset;
        block->GetDataRange = Base::Internal::FixedPoolBlock_get_data_offset;
        block->Data         = Base::Internal::FixedPoolBlock_data;
        block->DataSize     = Base::Internal::FixedPoolBlock_data_size;
        block->Info         = Base::Internal::FixedPoolBlock_info;
        block->InfoSize     = Base::Internal::FixedPoolBlock_info_size;
    }

    return block;
}


CPacket CPacketFactory::Create(size_t capacity, size_t info_size, MemoryAllocFunc allocfunc, MemoryFreeFunc freefunc)
{
    CPacket packet;
    packet.m_block = GeneralMemoryBlockCreate(allocfunc, freefunc, capacity, info_size);
    return packet;
}


} // namespace Base

////////////////////////////////////////////////////////////////////////////////

