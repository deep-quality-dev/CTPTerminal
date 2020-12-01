#pragma once

#include "WorkerThread.h"
#include <vector>

class CThostSpiMessage;
class CThostSpiBaseHandler;
class CDataCenter;
class IGuiDataAction;

class CThostBaseWrapper : public WorkerThread<CThostSpiMessage>
{
public:
	CThostBaseWrapper(CDataCenter* data_center, IGuiDataAction* gui_action) {
		data_center_ = data_center;
		gui_action_ = gui_action;
		timer_id_ = 1024;
		request_id_ = 0;

		CreateThread();
	}
	~CThostBaseWrapper() {
		ExitThread();
	}

	int GetTimerId() {
		return timer_id_++;
	}

	int GetRequestId() {
		return request_id_++;
	}

	void AddSpiMsg(CThostSpiBaseHandler* spi_handler, CThostSpiMessage* spi_msg) {
		PostMessage(spi_msg);
	}

	const char* broker_id() const {
		return broker_id_.c_str();
	}

	const char* user_id() const {
		return user_id_.c_str();
	}

	const char* password() const {
		return password_.c_str();
	}

	virtual void Initialize(const std::string& broker_id,
		const std::string& user_id,
		const std::string& password,
		const std::vector<std::string>& fronts) {
		broker_id_ = broker_id;
		user_id_ = user_id;
		password_ = password;
		fronts_ = fronts;
	}

protected:
	CDataCenter* data_center_;
	IGuiDataAction* gui_action_;

	std::string broker_id_;
	std::string user_id_;
	std::string password_;
	std::vector<std::string> fronts_;

	int timer_id_;
	int request_id_;
};