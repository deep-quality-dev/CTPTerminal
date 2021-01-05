#include "stdafx.h"
#include "QryManager.h"
#include "Utils/Logger.h"
#include <sstream>


CQryManager::CQryManager() : timer_id_(1024), current_query_timer_id_(99)
{
}


CQryManager::~CQryManager()
{
}

void CQryManager::AddQuery(const std::function<int()>& func, const std::string keyword)
{
	std::unique_lock<std::mutex> lk(mutex_);

	if (query_tasks_.empty() && quering_tasks_.empty()) {
		int timer_id = GetTimerId();
		query_tasks_.push_back(QueryTask(timer_id, func, keyword));
		callback_timer_ids_.insert(timer_id);
		CreateTimer(timer_id, QueryRetryInterval);
	}
	else {
		query_tasks_.push_back(QueryTask(0, func, keyword));
	}
}

void CQryManager::CheckQuery(int request_id, int error_id)
{
	std::unique_lock<std::mutex> lk(mutex_);

	if (!quering_tasks_.empty() && quering_tasks_.front() == request_id) {
		if (query_tasks_.front().request_id == request_id) {
			ExitTimer(current_query_timer_id_);

			if (error_id) { // FAILED to CALL Query
				// RETRY to CALL
				QueryTask& task = query_tasks_.front();
				if (task.times < MaxRetryCount) {
					callback_timer_ids_.insert(task.timer_id);
					CreateTimer(task.timer_id, QueryRetryInterval);
				}
			}
			else {
				query_tasks_.pop_front();
				quering_tasks_.erase(quering_tasks_.begin());
			}
		}
	}
	if (quering_tasks_.empty() && !query_tasks_.empty()) {
		int timer_id = GetTimerId();
		query_tasks_.front().timer_id = timer_id;
		callback_timer_ids_.insert(timer_id);
		CreateTimer(timer_id, QueryRetryInterval);
	}
}

void CQryManager::OnProcessMsg(QueryTask* msg)
{

}

void CQryManager::OnTimer(int timer_id)
{
	auto it_timer = callback_timer_ids_.find(timer_id);
	if (it_timer != callback_timer_ids_.end()) {
		callback_timer_ids_.erase(it_timer);
		OnQueringTask(timer_id);
	}

	if (timer_id == current_query_timer_id_) {
		OnCurrentQuering(timer_id);
	}
}

void CQryManager::OnQueringTask(int timer_id)
{
	std::unique_lock<std::mutex> lk(mutex_);

	ExitTimer(timer_id);

	if (!query_tasks_.empty()) {
		QueryTask& task = query_tasks_.front();
		if (timer_id == task.timer_id) {
			ASSERT_TRUE(quering_tasks_.empty());

			task.request_id = task.func();
			if (task.request_id > 0) {
				current_query_timer_id_ = GetTimerId();
				CreateTimer(current_query_timer_id_, MaxQueryTimeout);
			}
			else if (task.request_id == -1) { // -1，表示网络连接失败；
				current_query_timer_id_ = GetTimerId();
				CreateTimer(current_query_timer_id_, QueryRetryInterval);
				return;
			}
			else if (task.request_id == -2) { // -2，表示未处理请求超过许可数；
				current_query_timer_id_ = GetTimerId();
				CreateTimer(current_query_timer_id_, MaxQueryDelay);
				return;
			}
			else if (task.request_id == -3) { // -3，表示每秒发送请求数超过许可数。
				current_query_timer_id_ = GetTimerId();
				CreateTimer(current_query_timer_id_, QueryRetryInterval);
				return;
			}

			quering_tasks_.push_back(task.request_id);
		}
	}
}

void CQryManager::OnCurrentQuering(int timer_id)
{
	std::unique_lock<std::mutex> lk(mutex_);

	ExitTimer(timer_id);

	if (!quering_tasks_.empty())
		quering_tasks_.erase(quering_tasks_.begin());

	ASSERT_TRUE(quering_tasks_.empty());

	QueryTask& task = query_tasks_.front();
	if (!query_tasks_.empty()) {
		ASSERT_TRUE(quering_tasks_.empty());

		int request_id = task.func();
		if (request_id > 0) {
			current_query_timer_id_ = GetTimerId();
			CreateTimer(current_query_timer_id_, MaxQueryTimeout);
		}
		else if (request_id == -1) { // -1，表示网络连接失败；
			current_query_timer_id_ = GetTimerId();
			CreateTimer(current_query_timer_id_, QueryRetryInterval);
			return;
		}
		else if (request_id == -2) { // -2，表示未处理请求超过许可数；
			current_query_timer_id_ = GetTimerId();
			CreateTimer(current_query_timer_id_, MaxQueryDelay);
			return;
		}
		else if (request_id == -3) { // -3，表示每秒发送请求数超过许可数。
			current_query_timer_id_ = GetTimerId();
			CreateTimer(current_query_timer_id_, QueryRetryInterval);
			return;
		}

		quering_tasks_.push_back(request_id);
		task.times++;
	}
}
