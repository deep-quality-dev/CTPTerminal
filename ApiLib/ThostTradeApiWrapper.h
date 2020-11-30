﻿#pragma once

#include "ThostBaseWrapper.h"
#include "TradeApi.h"

class CThostFtdcTraderApi;
class CThostTraderSpiHandler;
class CQryManager;

class CThostTradeApiWrapper : public CThostBaseWrapper, public ITradeApi
{
public:
	CThostTradeApiWrapper(CDataCenter* data_center, IGuiDataAction* gui_action);
	~CThostTradeApiWrapper();

	const int MaxConnectTimeout = 10000;
	const int MaxRequestTimeout = 10000;

	virtual void OnProcessMsg(CThostSpiMessage* msg);
	virtual void OnTimer(int timer_id);

	virtual void Initialize(const std::string& broker_id,
		const std::string& user_id,
		const std::string& password,
		const std::vector<std::string> fronts);

	void ReqConnect();
	void ReqUserLogin();
	int ReqQryTradingAccount();
	int ReqQryAllInstrument();
	int ReqQryOrder();
	int ReqQryTrade();
	int ReqQryPosition();
	int ReqQryPositionDetail();
	int ReqQryDepthMarketData();


protected:
	void OnRspUserLogin(CThostSpiMessage* msg);
	void OnRspUserLogout(CThostSpiMessage* msg);
	void OnRspQryOrder(CThostSpiMessage* msg);
	void OnRspQryTrade(CThostSpiMessage* msg);
	void OnRspQryInvestorPosition(CThostSpiMessage* msg);
	void OnRspQryTradingAccount(CThostSpiMessage* msg);
	void OnRspQryInstrument(CThostSpiMessage* msg);
	void OnRspQryDepthMarketData(CThostSpiMessage* msg);
	void OnRspQryInvestorPositionDetail(CThostSpiMessage* msg);

private:
	CThostFtdcTraderApi* trader_api_;
	std::shared_ptr<CThostTraderSpiHandler> trader_spi_handler_;
	std::shared_ptr<CQryManager> qry_manager_;

	bool connected_, logined_;
	int login_times_;

	int connect_timer_id_; // 检查连接的定时ID

	std::set<Instrument> instruments_cache_;
};

