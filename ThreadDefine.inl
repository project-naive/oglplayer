//#include "ThreadDefine.h"

inline mutex::mutex(uint32_t spin_count) noexcept
{
	InitializeCriticalSectionEx(&handle, spin_count, 0/* debug info */);
}

inline mutex::~mutex() noexcept
{
	DeleteCriticalSection(&handle);
}

inline void mutex::lock() noexcept
{
	EnterCriticalSection(&handle);
}

inline bool mutex::try_lock() noexcept
{
	return TryEnterCriticalSection(&handle);
}

inline void mutex::unlock() noexcept
{
	LeaveCriticalSection(&handle);
}

inline CRITICAL_SECTION* mutex::native_handle() noexcept
{
	return &handle;
}

inline rwlock::rwlock() noexcept
{
	InitializeSRWLock(&handle);
}

inline void rwlock::start_write() noexcept
{
	AcquireSRWLockExclusive(&handle);
}

inline void rwlock::end_write() noexcept
{
	ReleaseSRWLockExclusive(&handle);
}

inline void rwlock::start_read() noexcept
{
	AcquireSRWLockShared(&handle);
}

inline void rwlock::end_read() noexcept
{
	ReleaseSRWLockShared(&handle);
}

inline bool rwlock::try_write() noexcept
{
	return TryAcquireSRWLockExclusive(&handle);
}

inline bool rwlock::try_read() noexcept
{
	return TryAcquireSRWLockShared(&handle);
}

template<typename lockable>
inline lock_guard<lockable>::lock_guard(lockable& mtx) noexcept:m_mtx(mtx)
{
	m_mtx.lock();
}

template<typename lockable>
inline lock_guard<lockable>::~lock_guard() noexcept
{
	m_mtx.unlock();
}

inline lock_guard_rwl_r::lock_guard_rwl_r(rwlock& mtx) noexcept:m_mtx(mtx)
{
	m_mtx.start_read();
}

inline lock_guard_rwl_r::~lock_guard_rwl_r() noexcept
{
	m_mtx.end_read();
}

inline lock_guard_rwl_w::lock_guard_rwl_w(rwlock& mtx) noexcept:m_mtx(mtx)
{
	m_mtx.start_write();
}

inline lock_guard_rwl_w::~lock_guard_rwl_w() noexcept
{
	m_mtx.end_write();
}

inline condition::condition() noexcept
{
	InitializeConditionVariable(&hcond);
}

inline void condition::wait(lock_guard<mutex>& lck) noexcept
{
	SleepConditionVariableCS(&hcond, &lck.m_mtx.handle, INFINITE);
}

inline void condition::wait(lock_guard<mutex>& lck, bool(*pred)()) noexcept
{
	while (!pred()) {
		SleepConditionVariableCS(&hcond, &lck.m_mtx.handle, INFINITE);
	}
}

inline bool condition::wait_for(lock_guard<mutex>& lck, float second) noexcept
{
	return SleepConditionVariableCS(&hcond, &lck.m_mtx.handle, second * 1000);
}

inline bool condition::wait_for(lock_guard<mutex>& lck, float second, bool(*pred)()) noexcept
{
	auto start = std::chrono::system_clock::now();
	bool eval = pred();
	while (!eval && (std::chrono::duration<float>(std::chrono::system_clock::now()-start) > std::chrono::duration<float>(second))
		&& !SleepConditionVariableCS(&hcond, &lck.m_mtx.handle, second * 1000)) {
		eval = pred();
	};
	return eval;
}

inline void condition::wait_read(lock_guard_rwl_r& lck) noexcept
{
	SleepConditionVariableSRW(&hcond, &lck.m_mtx.handle, INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED);
}

inline void condition::wait_read(lock_guard_rwl_r& lck, bool(*pred)()) noexcept
{
	while (!pred()) {
		SleepConditionVariableSRW(&hcond, &lck.m_mtx.handle, INFINITE, CONDITION_VARIABLE_LOCKMODE_SHARED);
	}
}

inline bool condition::wait_read_for(lock_guard_rwl_r& lck, float second) noexcept
{
	return SleepConditionVariableSRW(&hcond, &lck.m_mtx.handle, second * 1000, CONDITION_VARIABLE_LOCKMODE_SHARED);
}

inline bool condition::wait_read_for(lock_guard_rwl_r& lck, float  second, bool(*pred)()) noexcept
{
	auto start = std::chrono::system_clock::now();
	bool eval = pred();
	while (!eval && (std::chrono::duration<float>(std::chrono::system_clock::now() - start) > std::chrono::duration<float>(second))
		&& !SleepConditionVariableSRW(&hcond, &lck.m_mtx.handle, second * 1000, CONDITION_VARIABLE_LOCKMODE_SHARED)) {
		eval = pred();
	};
	return eval;
}

inline void condition::wait_write(lock_guard_rwl_r& lck) noexcept
{
	SleepConditionVariableSRW(&hcond, &lck.m_mtx.handle, INFINITE, 0);
}

inline void condition::wait_write(lock_guard_rwl_r& lck, bool(*pred)()) noexcept
{
	while (!pred()) {
		SleepConditionVariableSRW(&hcond, &lck.m_mtx.handle, INFINITE, 0);
	}
}

inline bool condition::wait_write_for(lock_guard_rwl_r& lck, float second) noexcept
{
	return SleepConditionVariableSRW(&hcond, &lck.m_mtx.handle, second * 1000, 0);
}

inline bool condition::wait_write_for(lock_guard_rwl_r& lck, float second, bool(*pred)()) noexcept
{
	auto start = std::chrono::system_clock::now();
	bool eval = pred();
	while (!eval && (std::chrono::duration<float>(std::chrono::system_clock::now() - start) > std::chrono::duration<float>(second))
		&& !SleepConditionVariableSRW(&hcond, &lck.m_mtx.handle, second * 1000, 0)) {
		eval = pred();
	};
	return eval;
}
