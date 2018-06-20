#ifndef _BASE_MEMORY_SINGLETON_POOL_H__
#define _BASE_MEMORY_SINGLETON_POOL_H__


// std::less, std::less_equal, std::greater
#include <functional>
// std::size_t, std::ptrdiff_t
#include <cstddef>
// std::max
#include <algorithm>

#include "Singleton.h"


#ifndef PREVENT_MACRO_SUBSTITUTION
#define PREVENT_MACRO_SUBSTITUTION
#endif


namespace Base {

////////////////////////////////////////////////////////////////////////////////


struct NullMutex
{
	void Lock() {}
	void Unlock() {}
};


////////////////////////////////////////////////////////////////////////////////

/// 单件内存池, 线程安全性取决于Mutex类型
template <typename Tag, unsigned requestedSize,
	typename Pool,
	typename Mutex,
	unsigned nextSize = 32,
	unsigned maxSize = 0 >
struct TSingletonPool
{
public:
	typedef Tag tag;
	typedef Pool pool_type;
	typedef Mutex mutex_type;
	typedef size_t size_type;
	typedef std::ptrdiff_t difference_type;

	static const unsigned requested_size = requestedSize;
	static const unsigned next_size = nextSize;

private:
	struct mem_pool : Mutex
	{
		pool_type p;
		mem_pool() : p(requestedSize, nextSize) {}
	};

	struct guard
	{
		Mutex& m;
		guard(Mutex& mutex) : m(mutex) {m.Lock();}
		~guard() {m.Unlock();}
	};

	typedef TSingletonDefault<mem_pool> Singleton;

	TSingletonPool();

public:
	static void * malloc PREVENT_MACRO_SUBSTITUTION()
	{
		mem_pool & p = Singleton::Instance();
		guard g(p);
		return (p.p.malloc)();
	}
	static void free PREVENT_MACRO_SUBSTITUTION(void * const ptr)
	{
		mem_pool & p = Singleton::Instance();
		guard g(p);
		(p.p.free)(ptr);
	}
    static bool release_memory()
    {   //! Equivalent to SingletonPool::p.release_memory(); synchronized.
        mem_pool & p = Singleton::Instance();
        guard g(p);
        return p.p.release_memory();
    }
};


} // namespace Base

#endif // _BASE_MEMORY_SINGLETON_POOL_H__
