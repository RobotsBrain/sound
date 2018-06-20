//
//	Description:
//	�ο�"The C++ Programming Laguage (Special Edition)", by Bjarne Stroustrup��
//  ʵ�ֵ��ڴ�ģʽ�ڴ������
//


#ifndef __BASE_POOL_ALLOCATOR_H__
#define __BASE_POOL_ALLOCATOR_H__

#include <cassert>
#include <new>
#include "Mutex"
#include "SingletonPool.h"
#include "MemoryPool.h"

// �ж�STL�Ƿ���ȷʵ���� deallocate
#if defined(_RWSTD_VER) || defined(__SGI_STL_PORT) || defined(__BORLANDC__)
 #define NO_PROPER_STL_DEALLOCATE
#endif



namespace Base {


////////////////////////////////////////////////////////////////////////////////

namespace Pool {


typedef CMutex DefaultMutex;

struct DefaultSingletonTag {};

} // namespace Pool

////////////////////////////////////////////////////////////////////////////////


/// \brief ����std:: alloctor������ڴ�ģʽ�ڴ������
///
/// - ֻ֧��list����, ��Ϊlist����ÿ��ֻ������ͷ�һ��һ�ֹ��Ľڵ�. ����deque,
/// ��Ԫ�ع��С��4Kʱ, ��Ĭ�ϵ�������Ծ��൱���ڴ��, ���ڵ���4Kʱ, ��������
/// �����͵Ĺ��, һ����Ԫ��ָ��, һ������ĸ������ò��÷�������, ��һ����Ԫ��,
/// ��ʱ�ſ��Կ����ڴ��.  ��vectorʹ�óع���ʵ��ûʲô����, ���ڴ�������Ҳ
/// �Ѿ������˶�̬���������, �����Ҫ���ڴ��ʵ��, ���������ش���.
template <typename T,
	typename PoolType = CMemoryPool,        // ָ���ڴ������
	typename Mutex = Pool::DefaultMutex,    // ָ��������, Detial::CMemPool �ڲ��Ѿ������������� NullMutex
	typename SingletonTag = Pool::DefaultSingletonTag,
	unsigned nextSize = 32,                 // ÿ�η����������
	unsigned maxSize = 0>
class TPoolAllocator;


/// \brief TPoolAllocator��ƫ�ػ��汾
template<
	typename PoolType,
	typename Mutex,
	typename SingletonTag,
	unsigned nextSize,
	unsigned maxSize>
class TPoolAllocator<void, PoolType, Mutex, SingletonTag, nextSize, maxSize>
{
public:
	typedef void*       pointer;
	typedef const void* const_pointer;
	typedef void        value_type;
	template <class U> struct rebind {
		typedef TPoolAllocator<U, PoolType, Mutex, SingletonTag, nextSize, maxSize> other;
	};
};


/// \brief ����std:: alloctor������ڴ�ģʽ�ڴ������
/// �����ڴ�ʹ��CMemoryPool���������Ǿ�̬���������Գ����������������뵽��ϵͳ�ڴ�һֱ�����ͷ�
template <typename T,
	typename PoolType,
	typename Mutex,
	typename SingletonTag,
	unsigned nextSize,
	unsigned maxSize>
class TPoolAllocator
{
public:
	typedef Mutex mutex_type;
	typedef PoolType pool_type;
	static const unsigned next_size = nextSize;

	typedef T value_type;
	typedef T * pointer;
	typedef const T * const_pointer;
	typedef T & reference;
	typedef const T & const_reference;
	typedef size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template <typename U>
	struct rebind
	{
		typedef TPoolAllocator<U, PoolType, Mutex, SingletonTag, nextSize, maxSize> other;
	};

	TPoolAllocator()
	{}

	template <class U>
	TPoolAllocator(TPoolAllocator<U, PoolType, Mutex, SingletonTag, nextSize, maxSize> const&)
	{}

	~TPoolAllocator()
	{}

	pointer address(reference x) const
	{ return &x; }

	const_pointer address(const_reference x) const
	{ return x; }

	bool operator==(const TPoolAllocator &) const
	{ return true; }

	bool operator!=(const TPoolAllocator &) const
	{ return false; }

	static pointer allocate(const size_type n)
	{
		typedef TSingletonPool<SingletonTag, sizeof(T), PoolType, Mutex, nextSize, maxSize> SingletonPool;
		assert(n == 1);
		const pointer ret = static_cast<pointer>(SingletonPool::malloc());
		if (ret == 0)
		{
			throw std::bad_alloc();
		}
		return ret;
	}

	static pointer allocate(const size_type n, const void * const)
	{ return allocate(n); }

	static void deallocate(const pointer ptr, const size_type n)
	{
		typedef TSingletonPool<SingletonTag, sizeof(T), PoolType, Mutex, nextSize, maxSize> SingletonPool;
#ifdef NO_PROPER_STL_DEALLOCATE
		if (ptr == 0 || n == 0) return;
#endif
		assert(n == 1);
		SingletonPool::free(ptr);
	}

	size_type max_size() const
	{
		return static_cast<size_type>(-1) / sizeof(T);
	}

	void construct(pointer ptr, const value_type& x)
	{
		new(ptr) T(x);
	}

	void destroy(pointer ptr)
	{
		ptr->~T();
		(void) ptr; // avoid unused variable warning
	}
};

} // namespace Base

#endif // __BASE_POOL_ALLOCATOR_H__

