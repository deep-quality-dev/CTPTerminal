#pragma once

#include "WorkerThread.h"
#include <functional>

struct QueryTask {
	int request_id;
	int timer_id;
	std::function<int()> func;
	std::string keyword;
	int times;

	QueryTask(int timer_id, std::function<int()> func, std::string keyword)	{
		this->request_id = -1;
		this->timer_id = timer_id;
		this->func = func;
		this->keyword = keyword;
		this->times = 0;
	}
};

class CQryManager : public WorkerThread<QueryTask>
{
public:
	CQryManager();
	~CQryManager();

	const int QueryRetryInterval = 1000;
	const int MaxQueryTimeout = 30000;
	const int MaxQueryDelay = 30000;
	const int MaxRetryCount = 10;

	int GetTimerId() {
		return timer_id_++;
	}

	void AddQuery(const std::function<int()>& func, const std::string keyword);
	void CheckQuery(int request_id, int error_id);

	virtual void OnProcessMsg(std::shared_ptr<QueryTask> msg);
	virtual void OnTimer(int timer_id);

protected:
	void OnQueringTask(int timer_id);
	void OnCurrentQuering(int timer_id);

private:
	std::set<int> timer_ids_;

	int timer_id_;
	int current_query_timer_id_;

	std::deque<QueryTask> query_tasks_;
	std::vector<int> quering_tasks_;
	std::set<int> callback_timer_ids_;
};

