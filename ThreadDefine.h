#pragma once

#ifndef _THREAD_DEFINE_H_
#define _THREAD_DEFINE_H_

#include <atomic>
#include <chrono>

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef min;
#undef max;
//Using C++11 is most efficent and easy
using std::atomic;
typedef SRWLOCK _naive_rwlock;
typedef CRITICAL_SECTION _naive_mtx;

#else
//broken
using std::atomic;
using std::mutex;
typedef mutex _naive_mtx;
typedef SRWLOCK _naive_rwlock;
using condition = std::condition_variable;
#endif

class condition;
class mutex {
protected:
	friend condition;
	_naive_mtx handle;
public:
	//extension
	mutex(uint32_t spin_count = 0x1000) noexcept;
	~mutex() noexcept;
	void lock() noexcept;
	bool try_lock() noexcept;
	void unlock() noexcept;
	_naive_mtx* native_handle() noexcept;
	mutex(const mutex&) = delete;
	mutex& operator=(const mutex&) = delete;
};

class rwlock {
protected:
	friend condition;
	_naive_rwlock handle;
public:
	rwlock() noexcept;
	~rwlock() noexcept {}
	void start_write() noexcept;
	void end_write() noexcept;
	void start_read() noexcept;
	void end_read() noexcept;
	bool try_write() noexcept;
	bool try_read() noexcept;
	rwlock(const rwlock&) = delete;
	rwlock& operator=(const rwlock&) = delete;
};
typedef CONDITION_VARIABLE _naive_hcondition;

template<typename lockable = mutex>
class lock_guard {
public:
	lock_guard(lockable& mtx) noexcept;
	~lock_guard() noexcept;
	lock_guard() = delete;
	lock_guard(const lock_guard&) = delete;
	lock_guard& operator=(const lock_guard&) = delete;
	lock_guard(lock_guard&&) = delete;
	lock_guard& operator=(lock_guard&&) = delete;
protected:
	friend condition;
	lockable& m_mtx;
};

class lock_guard_rwl_r {
public:
	lock_guard_rwl_r(rwlock& mtx) noexcept;
	~lock_guard_rwl_r() noexcept;
	lock_guard_rwl_r() = delete;
	lock_guard_rwl_r(const lock_guard_rwl_r&) = delete;
	lock_guard_rwl_r& operator=(const lock_guard_rwl_r&) = delete;
	lock_guard_rwl_r(lock_guard_rwl_r&&) = delete;
	lock_guard_rwl_r& operator=(lock_guard_rwl_r&&) = delete;
protected:
	friend condition;
	rwlock& m_mtx;
};

class lock_guard_rwl_w {
public:
	lock_guard_rwl_w(rwlock& mtx) noexcept;
	~lock_guard_rwl_w() noexcept;
	lock_guard_rwl_w() = delete;
	lock_guard_rwl_w(const lock_guard_rwl_w&) = delete;
	lock_guard_rwl_w& operator=(const lock_guard_rwl_w&) = delete;
	lock_guard_rwl_w(lock_guard_rwl_w&&) = delete;
	lock_guard_rwl_w& operator=(lock_guard_rwl_w&&) = delete;
protected:
	friend condition;
	rwlock& m_mtx;
};

class condition {
	_naive_hcondition hcond;
public:
	condition() noexcept;
	~condition() noexcept {};
	condition(const condition&) = delete;
	condition& operator=(const condition&) = delete;
	void wait(lock_guard<mutex>& lck) noexcept;
	void wait(lock_guard<mutex>& lck, bool(*pred)()) noexcept;
	bool wait_for(lock_guard<mutex>& lck, float second) noexcept;
	bool wait_for(lock_guard<mutex>& lck, float second, bool(*pred)()) noexcept;
	void wait_read(lock_guard_rwl_r& lck) noexcept;
	void wait_read(lock_guard_rwl_r& lck, bool(*pred)()) noexcept;
	bool wait_read_for(lock_guard_rwl_r& lck, float second) noexcept;
	bool wait_read_for(lock_guard_rwl_r& lck, float second, bool(*pred)()) noexcept;
	void wait_write(lock_guard_rwl_r& lck) noexcept;
	void wait_write(lock_guard_rwl_r& lck, bool(*pred)()) noexcept;
	bool wait_write_for(lock_guard_rwl_r& lck, float second) noexcept;
	bool wait_write_for(lock_guard_rwl_r& lck, float second, bool(*pred)()) noexcept;
	void notify_one() noexcept {
		WakeConditionVariable(&hcond);
	}
	void notify_all() noexcept {
		WakeAllConditionVariable(&hcond);
	}
	_naive_hcondition* native_handle() noexcept {
		return &hcond;
	};
};

class semaphore {
	atomic<uint64_t> count{0};
	mutex mtx;
	condition cond;
public:
	void acquire(uint64_t needed)
	{
		while(true) {
			uint64_t cur(count.load(std::memory_order_acquire));
			if (cur >= needed) {
				//begin critical
				lock_guard<mutex> guard(mtx);
				if (cur >= needed) {
					count.fetch_sub(needed, std::memory_order_relaxed);
					//end critical
					return;
				}
				//end critical
			}
			lock_guard<mutex> guard(mtx);
			cond.wait(guard);
		}
	}
	void release(uint64_t released)
	{
		//increment or decrement and check
		count.fetch_add(released, std::memory_order_release);
		cond.notify_all();
	}
};



#include "ThreadDefine.inl"

#endif

