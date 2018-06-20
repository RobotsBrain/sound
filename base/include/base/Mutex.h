
#ifndef __BASE_MUTEX_H__
#define __BASE_MUTEX_H__


#include <stdint.h>
#include "Guard.h"


namespace Base {


/// mutex class
class CMutex
{
	CMutex(CMutex const&);
	CMutex& operator=(CMutex const&);

public:
	typedef TLockGuard<CMutex>	ScopedLock;

	CMutex();
	~CMutex();
	void Lock();
	void Unlock();

private:
	intptr_t mInternal[8];
};


/// recursive mutex class, avoid to use, see http://blog.chinaunix.net/uid-26983585-id-3316794.html
class CRecursiveMutex
{
	CRecursiveMutex(CRecursiveMutex const&);
	CRecursiveMutex& operator=(CRecursiveMutex const&);

public:
	typedef TLockGuard<CRecursiveMutex>	ScopedLock;

	CRecursiveMutex();
	~CRecursiveMutex();
	void Lock();
	void Unlock();

private:
	intptr_t mInternal[8];
};


} // namespace Base


#endif // __BASE_MUTEX_H__

