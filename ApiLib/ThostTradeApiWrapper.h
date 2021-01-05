#pragma once

#include "ThostBaseWrapper.h"
#include "TradeApi.h"
#include <map>

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
		const std::vector<std::string>& fronts,
		const std::string& user_product_info,
		const std::string& auth_code,
		const std::string& app_id);
	virtual void Deinitialize();

	virtual void Login();
	virtual void Logout();

	void ReqConnect();
	void ReqAuthenticate();
	void ReqUserLogin();
	void ReqUserLogout();
	int ReqQrySettlementInfoConfirm();
	int ReqQrySettlementInfo(unsigned int date);
	virtual int ReqQryTradingAccount();
	virtual int ReqQryAllInstrument();
	virtual int ReqQryOrder();
	virtual int ReqQryTrade();
	virtual int ReqQryPosition();
	virtual int ReqQryPositionDetail();
	virtual int ReqQryDepthMarketData();
	virtual void ReqInsertOrder(const OrderInsert& order_insert);
	virtual void ReqQryMarginRate(const std::string& instrument_id);
	virtual void ReqCancelOrder(const Order& order);

protected:
	int ReqQryInstrumentMarginRate(const std::string& instrument_id);

	void OnRspConnected(CThostSpiMessage* msg);
	void OnRspDisconnected(CThostSpiMessage* msg);
	void OnRspAuthenticate(CThostSpiMessage* msg);
	void OnRspUserLogin(CThostSpiMessage* msg);
	void OnRspUserLogout(CThostSpiMessage* msg);
	void OnRtnOrder(CThostSpiMessage* msg);
	void OnRtnTrade(CThostSpiMessage* msg);
	void OnErrRtnOrderInsert(CThostSpiMessage* msg);
	void OnRspOrderInsert(CThostSpiMessage* msg);
	void OnRspQryOrder(CThostSpiMessage* msg);
	void OnRspQryTrade(CThostSpiMessage* msg);
	void OnRspQryInvestorPosition(CThostSpiMessage* msg);
	void OnRspQryTradingAccount(CThostSpiMessage* msg);
	void OnRspQryInstrumentMarginRate(CThostSpiMessage* msg);
	void OnRspQryInstrument(CThostSpiMessage* msg);
	void OnRspQryDepthMarketData(CThostSpiMessage* msg);
	void OnRspQryInvestorPositionDetail(CThostSpiMessage* msg);
	void OnRspQrySettlementInfoConfirm(CThostSpiMessage* msg);

private:
	std::string user_product_info_;	// 用户端产品信息
	std::string auth_code_;	// 认证码
	std::string app_id_;	// App代码

	CThostFtdcTraderApi* trader_api_;
	CThostTraderSpiHandler* trader_spi_handler_;
	CQryManager* qry_manager_;

	bool connected_, authenticated_, logined_;
	int login_times_;
	bool force_logout_;

	HANDLE logout_event_;

	int connect_timer_id_; // 检查连接的定时ID

	std::set<Instrument> instruments_cache_;
	std::set<Order> orders_cache_;
	std::set<Trade> trades_cache_;
	std::set<Position> positions_cache_;
	std::set<PositionDetail> position_details_cache_;

	std::map<OrderKey, std::string> orderkey2sysid_;
	std::map<std::string, OrderKey> sysid2orderkey_;
	std::set<Trade> rtn_trade_cache_;

	std::set<std::string> querying_margin_rates_;
	std::set<std::string> ready_margin_rates_;

	int order_ref_;
};

