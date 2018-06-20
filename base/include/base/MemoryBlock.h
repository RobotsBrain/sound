#ifndef __PAL_MEMORY_BLOCK_INTERFACE_H__
#define __PAL_MEMORY_BLOCK_INTERFACE_H__

#include <stddef.h> // for size_t
#include <stdint.h> // intptr_t

#ifdef __cplusplus
extern "C" {
#endif


/// 内存块描述接口, 32个指针大小
typedef struct IMemoryBlock
{
    /// 增引用计数
    int (*AddRef)(struct IMemoryBlock* thiz);

    /// 减引用计数及释放自身
    int (*Release)(struct IMemoryBlock* thiz);

    /// 数据区总容量
    int (*Capacity)(struct IMemoryBlock* thiz);

    /// 有效数据区起始偏移
    int (*SetDataRange)(struct IMemoryBlock* thiz, int begin, int end);

    /// 获取有效数据区间
    int (*GetDataRange)(struct IMemoryBlock* thiz, int* begin, int* end);

    /// 有效数据区指针
    void* (*Data)(struct IMemoryBlock* thiz);

    /// 有效数据区字节数
    int (*DataSize)(struct IMemoryBlock* thiz);

    /// 信息区指针
    void* (*Info)(struct IMemoryBlock* thiz);

    /// 信息区字节数
    int (*InfoSize)(struct IMemoryBlock* thiz);

    /// 保留
    void* reserved[7];

    /// 内部私有数据
    intptr_t priv[16];

} IMemoryBlock;



#ifdef __cplusplus
} // extern "C"
#endif

#endif // __PAL_MEMORY_BLOCK_INTERFACE_H__

