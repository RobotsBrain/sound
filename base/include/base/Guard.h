
#ifndef __BASE_LOCK_GUARD_H__
#define __BASE_LOCK_GUARD_H__


namespace Base {


/// Lock Guard template
template<class Mutex>
class TLockGuard
{
	TLockGuard(TLockGuard const&);
	TLockGuard& operator=(TLockGuard const&);

public:
	TLockGuard(Mutex& mutex)
		: mMutex(mutex)
	{
		mMutex.Lock();
	}

	~TLockGuard()
	{
		mMutex.Unlock();
	}

private:
	Mutex&	mMutex;
};


} // namespace Base


#endif // __BASE_LOCK_GUARD_H__

