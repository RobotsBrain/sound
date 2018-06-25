#include <string.h>
#include <pthread.h>

#include "base/StaticAssert.h"
#include "base/Mutex.h"


namespace Base {

struct MutexInternal
{
	pthread_mutex_t mtx;
};

template<class T, size_t COUNT>
inline MutexInternal* toMutexInternal(T(&p)[COUNT])
{
	BASE_STATIC_ASSERT(sizeof(MutexInternal) < sizeof(T) * COUNT);
	return (MutexInternal*)p;
}

CMutex::CMutex()
{
	memset(mInternal, 0, sizeof(mInternal));

	MutexInternal* mi = toMutexInternal(mInternal);
	pthread_mutex_init(&mi->mtx, NULL);
}

CMutex::~CMutex()
{
	MutexInternal* mi = toMutexInternal(mInternal);
	pthread_mutex_destroy(&mi->mtx);
}

void CMutex::Lock()
{
	MutexInternal* mi = toMutexInternal(mInternal);
	pthread_mutex_lock(&mi->mtx);
}

void CMutex::Unlock()
{
	MutexInternal* mi = toMutexInternal(mInternal);
	pthread_mutex_unlock(&mi->mtx);
}

////////////////////////////////////////////////////////////////////////////////

CRecursiveMutex::CRecursiveMutex()
{
	memset(mInternal, 0, sizeof(mInternal));
	MutexInternal* mi = toMutexInternal(mInternal);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mi->mtx, &attr);
    pthread_mutexattr_destroy(&attr);
}

CRecursiveMutex::~CRecursiveMutex()
{
	MutexInternal* mi = toMutexInternal(mInternal);
	pthread_mutex_destroy(&mi->mtx);
}

void CRecursiveMutex::Lock()
{
	MutexInternal* mi = toMutexInternal(mInternal);
	pthread_mutex_lock(&mi->mtx);
}

void CRecursiveMutex::Unlock()
{
	MutexInternal* mi = toMutexInternal(mInternal);
	pthread_mutex_unlock(&mi->mtx);
}


} // namespace Base

