﻿#pragma once

#include "ThostBaseWrapper.h"
#include "MarketDataApi.h"
#include <map>

class CThostFtdcMdApi;
class CThostMdSpiHandler;

class CThostMdApiWrapper : public CThostBaseWrapper, public IMarketDataApi
{
public:
	CThostMdApiWrapper(CDataCenter* data_center, IGuiDataAction* gui_action);
	~CThostMdApiWrapper();

	const int MaxConnectTimeout = 10000;
	const int MaxRequestTimeout = 10000;

	virtual void OnProcessMsg(std::shared_ptr<CThostSpiMessage> msg);
	virtual void OnTimer(int timer_id);

	virtual void Initialize(const std::string& broker_id,
		const std::string& user_id,
		const std::string& password,
		const std::vector<std::string> fronts);

	void ReqConnect();
	void ReqUserLogin();
	virtual void ReqSubscribeQuote(std::set<std::string> instruments);

protected:
	void CheckSubscribe();
	void Subscribe(std::set<std::string> instruments);

	void OnFrontConnected(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspUserLogin(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspUserLogout(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspSubMarketData(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspUnSubMarketData(std::shared_ptr<CThostSpiMessage> msg);
	void OnRtnDepthMarketData(std::shared_ptr<CThostSpiMessage> msg);

private:
	CThostFtdcMdApi* md_api_;
	std::shared_ptr<CThostMdSpiHandler> md_spi_handler_;

	bool connected_, logined_;
	int login_times_;

	int connect_timer_id_; // 检查连接的定时ID

	std::set<std::string> allocated_instruments_;

	std::set<std::string> need_subscribe_instruments_;
	std::set<std::string> subscribing_instruments_;
	std::set<std::string> subscribed_instruments_;

	std::map<int, std::string> timer_id2instrument_;
	std::map<std::string, int> instrument2timer_id_;
};

