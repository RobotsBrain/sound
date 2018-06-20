#ifndef __BASE_BUFFER_H__
#define __BASE_BUFFER_H__


#include <stdint.h>


namespace Base {


/// 可动态增长的数据缓存类, 支持小内存优化
class CBuffer
{
	/// 禁止复制构造和赋值操作
	CBuffer(CBuffer const&);
	CBuffer& operator=(CBuffer const&);

public:
	/// 构造函数
	/// \param [in] increase 设置每次动态增长的大小
	CBuffer(int increase = 64);

	/// 析构函数
	~CBuffer();

	/// 预分配缓存容量, 如果原缓存容量不够, 会造成内存重新分配以及内存复制操作
	/// \param capacity 新的缓存容量
	void Reserve(int capacity);

	/// 设置有效数据长度, 如果原缓存容量不够, 会造成内存重新分配以及内存复制操作
	/// \param size 新的有效数据长度
    void Resize(int bytes);

	/// 向尾部追加数据, 如果原缓存容量不够, 会造成内存重新分配以及内存复制操作
	/// \param buffer 追加的数据指针
	/// \param length 追加的数据长度
	/// \return 实际写入的数据
    int PutBuffer(void const* buffer, int bytes);

	/// 取数据地址, PutBuffer Resize Reserve 操作会造成地址失效
    void* GetBuffer() const;

	/// 取有效数据长度
    int Size() const;

	/// 取缓存容量
	int Capacity() const;

	/// 是否为空
	bool Empty() const;

	/// 释放缓存, 重置为空对象
	void Clear();

private:
    intptr_t mInternal[8];
};


} // namespace Base


#endif // __BASE_BUFFER_H__

