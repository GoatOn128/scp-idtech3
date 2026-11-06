#include "cmdlib.h"
#include "mutex.h"
#include "threads.h"

void idQ3MapMutex::Lock() {
	mutex.lock();
}

void idQ3MapMutex::Unlock() {
	mutex.unlock();
}

void MutexLock( mutex_t *m ) {
	if ( m != nullptr ) {
		m->Lock();
	}
}

void MutexUnlock( mutex_t *m ) {
	if ( m != nullptr ) {
		m->Unlock();
	}
}

mutex_t *MutexAlloc() {
	return numthreads == 1 ? nullptr : new idQ3MapMutex();
}
