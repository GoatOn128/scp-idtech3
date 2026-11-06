#include "cmdlib.h"
#include "threads.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

class idQ3MapThreadPool {
public:
	void Run( int workCount, qboolean showPacifier, void ( *function )( int ) );
	void Lock();
	void Unlock();

private:
	std::mutex mutex;
};

static idQ3MapThreadPool threadPool;
int numthreads = -1;
int dispatch = 0;
int workcount = 0;
int oldf = 0;
qboolean pacifier = qfalse;
qboolean threaded = qfalse;

void idQ3MapThreadPool::Lock() {
	mutex.lock();
}

void idQ3MapThreadPool::Unlock() {
	mutex.unlock();
}

int GetThreadWork() {
	threadPool.Lock();
	if ( dispatch == workcount ) {
		threadPool.Unlock();
		return -1;
	}
	const int work = dispatch++;
	const int fraction = workcount == 0 ? 10 : 10 * work / workcount;
	if ( fraction != oldf ) {
		oldf = fraction;
		if ( pacifier ) {
			_printf( "%i...", fraction );
		}
	}
	threadPool.Unlock();
	return work;
}

static void ThreadWorkerFunction( void ( *function )( int ) ) {
	for ( ;; ) {
		const int work = GetThreadWork();
		if ( work < 0 ) {
			return;
		}
		function( work );
	}
}

void ThreadSetDefault() {
	if ( numthreads < 0 ) {
		numthreads = static_cast<int>( std::thread::hardware_concurrency() );
		if ( numthreads < 1 || numthreads > 32 ) {
			numthreads = 1;
		}
	}
	qprintf( "%i threads\n", numthreads );
}

void ThreadLock() {
	if ( threaded ) {
		threadPool.Lock();
	}
}

void ThreadUnlock() {
	if ( threaded ) {
		threadPool.Unlock();
	}
}

void RunThreadsOn( int workcnt, qboolean showpacifier, void ( *function )( int ) ) {
	if ( numthreads < 0 ) {
		ThreadSetDefault();
	}
	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	threaded = qtrue;
	const double start = I_FloatTime();

	std::vector<std::thread> workers;
	if ( numthreads > 1 ) {
		workers.reserve( numthreads );
		for ( int i = 0; i < numthreads; ++i ) {
			workers.emplace_back( ThreadWorkerFunction, function );
		}
	} else {
		function( 0 );
	}
	for ( std::thread &worker : workers ) {
		worker.join();
	}
	threaded = qfalse;
	if ( pacifier ) {
		_printf( " (%i)\n", static_cast<int>( I_FloatTime() - start ) );
	}
}

void RunThreadsOnIndividual( int workcnt, qboolean showpacifier, void ( *function )( int ) ) {
	RunThreadsOn( workcnt, showpacifier, function );
}
