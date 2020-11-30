#include "stdafx.h"
#include "WorkerThread.h"
#include "ThostSpiMessage.h"
#include <iostream>
#include <memory>

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
WorkerThread<T>::WorkerThread()
{
}

template <class T>
WorkerThread<T>::~WorkerThread()
{
}

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
	context->thread_id = timer_thread.get_id();
	context->timer_id = timer_id;
	context->delay = delay;
	context->running = true;

	std::thread timer_thread(&WorkerThread::TimerThread, this, &context->running, timer_id, delay, func);
	context->thread = timer_thread;

	timer_threads_.push(context);
	return context->thread_id;
}

template <class T>
void WorkerThread<T>::ExitTimer(int timer_id)
{
	for (auto it = timer_threads_.begin(); it != timer_threads_.end(); it++) {
		if (it->first->timer_id == timer_id) {
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
			OnTimer(threadmsg->msgData.get());
		}
		else {
			OnProcessMsg(threadmsg->msgData);
		}
	}
}

template <class T>
void WorkerThread<T>::SpawnTimerThread()
{
	while (idle_timer_threads_.size()) {
		std::shared_ptr<TimerContext> context = idle_timer_threads_.pop();
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
		queue_.push(std::shared_ptr<ThreadMessage>(MSG_TIMER, std::shared_ptr<int>(timer_id)));
		cv_.notify_one();
	}
}
