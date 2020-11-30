#pragma once

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

	virtual void OnProcessMsg(std::shared_ptr<CThostSpiMessage> msg);
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
	void OnRspUserLogin(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspUserLogout(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspQryOrder(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspQryTrade(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspQryInvestorPosition(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspQryTradingAccount(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspQryInstrument(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspQryDepthMarketData(std::shared_ptr<CThostSpiMessage> msg);
	void OnRspQryInvestorPositionDetail(std::shared_ptr<CThostSpiMessage> msg);

private:
	CThostFtdcTraderApi* trader_api_;
	std::shared_ptr<CThostTraderSpiHandler> trader_spi_handler_;
	std::shared_ptr<CQryManager> qry_manager_;

	bool connected_, logined_;
	int login_times_;

	int connect_timer_id_; // 检查连接的定时ID

	std::set<Instrument> instruments_cache_;
};

