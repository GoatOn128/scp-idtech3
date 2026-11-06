
#ifndef Q3MAP_MUTEX_H
#define Q3MAP_MUTEX_H

#include <mutex>

class idQ3MapMutex {
public:
	idQ3MapMutex() = default;
	idQ3MapMutex( const idQ3MapMutex & ) = delete;
	idQ3MapMutex &operator=( const idQ3MapMutex & ) = delete;

	void Lock();
	void Unlock();

private:
	friend void MutexLock( idQ3MapMutex *mutex );
	friend void MutexUnlock( idQ3MapMutex *mutex );
	std::mutex mutex;
};

typedef idQ3MapMutex mutex_t;

void MutexLock (mutex_t *m);
void MutexUnlock (mutex_t *m);
mutex_t *MutexAlloc(void);

#endif
