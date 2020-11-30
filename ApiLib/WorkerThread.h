#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <vector>

struct ThreadMessage;
struct TimerContext;

class CThostSpiMessage;

template <class T>
class WorkerThread
{
public:
	WorkerThread();
	~WorkerThread();

	bool CreateThread();
	void ExitThread();

	std::thread::id CreateTimer(int timer_id, int delay);
	void ExitTimer(int timer_id);
	void ExitTimer(std::thread::id thread_id);

	std::thread::id GetThreadId();

	void PostMessage(T* msg);

	virtual void OnProcessMsg(std::shared_ptr<T> msg) = 0;
	virtual void OnTimer(int timer_id) = 0;

private:
	WorkerThread(const WorkerThread&) = delete;
	WorkerThread& operator=(const WorkerThread&) = delete;

	void Process();
	void SpawnTimerThread();
	void TimerThread(bool* running, int timer_id, int delay);

	std::queue<std::shared_ptr<TimerContext>> idle_timer_threads_;
	std::unique_ptr<std::thread> timer_thread_;
	std::unique_ptr<std::thread> thread_;
	std::mutex mutex_;
	std::queue<std::shared_ptr<ThreadMessage>> queue_;
	std::condition_variable cv_;
	std::atomic<bool> exit_;

	std::vector<std::shared_ptr<TimerContext>> timer_threads_;
};

#endif
