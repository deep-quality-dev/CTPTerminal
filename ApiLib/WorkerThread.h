#ifndef WORKER_THREAD_H_
#define WORKER_THREAD_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <vector>

#define MSG_DATA	1
#define MSG_TIMER	2
#define MSG_EXIT	3

struct ThreadMessage
{
	ThreadMessage(int msgType, std::shared_ptr<void> msgData) {
		this->msgType = msgType;
		this->msgData = msgData;
	}

	int msgType;
	std::shared_ptr<void> msgData;
};

struct TimerContext
{
	std::thread::id thread_id;
	int timer_id;
	int delay;
	bool running;

	std::thread thread;
};

template <class T>
class WorkerThread
{
public:
	WorkerThread() {}
	~WorkerThread() {}

	bool CreateThread();
	void ExitThread();

	std::thread::id CreateTimer(int timer_id, int delay);
	void ExitTimer(int timer_id);
	void ExitTimer(std::thread::id thread_id);

	std::thread::id GetThreadId();

	void PostMessage(T* msg);

	virtual void OnProcessMsg(T* msg) = 0;
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

template <class T>
bool WorkerThread<T>::CreateThread()
{
	if (!thread_) {
		thread_ = std::unique_ptr<std::thread>(new std::thread(&WorkerThread::Process, this));
	}
	return true;
}

template <class T>
void WorkerThread<T>::ExitThread()
{
	if (!thread_)
		return;

	std::shared_ptr<ThreadMessage> msg(new ThreadMessage(MSG_EXIT, 0));
	{
		std::unique_lock<std::mutex> lk(mutex_);
		queue_.push(msg);
		cv_.notify_one();
	}

	thread_->join();
	thread_ = nullptr;

	if (timer_thread_) {
		timer_thread_->join();
	}
}

template <class T>
std::thread::id WorkerThread<T>::CreateTimer(int timer_id, int delay)
{
	std::shared_ptr<TimerContext> context(new TimerContext());
	context->thread_id = timer_thread_->get_id();
	context->timer_id = timer_id;
	context->delay = delay;
	context->running = true;

	// std::thread timer_thread(&WorkerThread::TimerThread, this, &context->running, timer_id, delay);
	context->thread = std::thread(&WorkerThread::TimerThread, this, &context->running, timer_id, delay);

	timer_threads_.push_back(context);
	return context->thread_id;
}

template <class T>
void WorkerThread<T>::ExitTimer(int timer_id)
{
	for (auto it = timer_threads_.begin(); it != timer_threads_.end(); it++) {
		if ((*it)->timer_id == timer_id) {
			(*it)->running = false;
			timer_threads_.erase(it);
			idle_timer_threads_.push(*it);

			if (!timer_thread_) {
				timer_thread_ = std::unique_ptr<std::thread>(new std::thread(&WorkerThread::SpawnTimerThread, this));
			}
			break;
		}
	}
}

template <class T>
void WorkerThread<T>::ExitTimer(std::thread::id thread_id)
{
	for (auto it = timer_threads_.begin(); it != timer_threads_.end(); it++) {
		if (it->first->thread_id == thread_id) {
			it->first->running = false;
			timer_threads_.erase(it);
			idle_timer_threads_.push(*it);

			if (!timer_thread_) {
				timer_thread_ = std::unique_ptr<std::thread>(new std::thread(&WorkerThread::SpawnTimerThread, this));
			}
			break;
		}
	}
}

template <class T>
std::thread::id WorkerThread<T>::GetThreadId()
{
	return thread_->get_id();
}

template <class T>
void WorkerThread<T>::PostMessage(T* msg)
{
	if (!thread_)
		return;

	{
		std::unique_lock<std::mutex> lk(mutex_);
		queue_.push(std::make_shared<ThreadMessage>(ThreadMessage(MSG_DATA, std::shared_ptr<T>(msg))));
		cv_.notify_one();
	}
}

template <class T>
void WorkerThread<T>::Process()
{
	while (true) {
		std::shared_ptr<ThreadMessage> threadmsg;
		{
			std::unique_lock<std::mutex> lk(mutex_);
			while (queue_.empty())
				cv_.wait(lk);

			if (queue_.size() < 1)
				continue;

			threadmsg = queue_.front();
			queue_.pop();
		}

		if (threadmsg->msgType == MSG_EXIT) {
			return;
		}

		if (threadmsg->msgType == MSG_TIMER) {
			TimerContext* timer_context = (TimerContext*)threadmsg->msgData.get();
			OnTimer(timer_context->timer_id);
		}
		else {
			T* msg_context = (T*)threadmsg->msgData.get();
			OnProcessMsg(msg_context);
		}
	}
}

template <class T>
void WorkerThread<T>::SpawnTimerThread()
{
	while (idle_timer_threads_.size()) {
		std::shared_ptr<TimerContext> context = idle_timer_threads_.front();
		idle_timer_threads_.pop();
		context->thread.join();
	}
}

template <class T>
void WorkerThread<T>::TimerThread(bool* running, int timer_id, int delay)
{
	while (*running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));

		if (!*running)
			break;

		std::unique_lock<std::mutex> lk(mutex_);
		queue_.push(std::make_shared<ThreadMessage>(ThreadMessage(MSG_TIMER, std::make_shared<int>(timer_id))));
		cv_.notify_one();
	}
}


#endif
