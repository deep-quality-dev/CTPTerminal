#pragma once

#include <wtypes.h>
#include "DataCenter.h"
#include "GuiDataAction.h"
#include "QuoteServerHandler.h"
#include "MarketDataApi.h"
#include "TradeApi.h"
#include <vector>
#include <memory>

class CQuoteService : public CQuoteServerHandler
{
public:
	CQuoteService();
	~CQuoteService();

	const int MaxLoginTimeout = 30000;
	const int MaxInsertOrderTimeout = 30000;
	const int MaxAccountTimeout = 30000;
	const int MaxPositionListTimeout = 30000;

	CDataCenter* data_center() {
		return data_center_;
	}
	void set_data_center(CDataCenter* data_center) {
		data_center_ = data_center;
	}

	IGuiDataAction* gui_action() {
		return gui_action_;
	}
	void set_gui_action(IGuiDataAction* gui_action) {
		gui_action_ = gui_action;
	}

	ITradeApi* trade_api() {
		return trade_api_;
	}

	void Initialize(const std::vector<MarketDataServerConfig>& quote_configs,
		const TradeServerConfig& trade_config);
	void Deinitialize();
	
	int Login();
	int Logout();

	void SetInstruments(const std::set<Instrument>& instruments);

	void SetSubscribeProducts(const std::set<std::string>& products);
	void SetSubscribeInstruments(const std::set<std::string>& instrument_ids);

	void ReqQryTradingAccount();
	void ReqQryPositionDetail();

protected:
	void InitMdWrappers();
	void InitTradeWrappers();

	void CheckSubscribes();

	virtual void OnLoginProcess(ApiEvent api_event, const char* content = NULL, int error_id = 0, const char* error_msg = NULL);
	virtual void RefreshAccount(const TradingAccount& account);
	virtual void RefreshQuotes(const std::set<Quote>& quotes);
	virtual void RefreshInstruments(const std::set<Instrument>& instruments);
	virtual void RefreshInstrumentMarginRate(const InstrumentMarginRate& margin_rate);
	virtual void RefreshPositions(const std::set<Position>& positions);
	virtual void RefreshPositionDetails(const std::set<PositionDetail>& positions);
	virtual void RefreshOrders(const std::set<Order>& orders);
	virtual void RefreshTrades(const std::set<Trade>& trades);
	virtual void RefreshOrder(const Order& order);
	virtual void RefreshTrade(const Trade& trade);

protected:
	HANDLE login_handle;
	HANDLE account_handle;
	HANDLE insert_order_handle;
	HANDLE position_list_handle;

	bool logined_;
	int md_logined_; // 0 : waiting to login, -1 : failed to login, 1: successfully logined
	int trade_logined_; // 0 : waiting to login, -1 : failed to login, 1: successfully logined
	bool instrument_inited_;
	bool position_list_inited_;

	bool trader_inited_, md_inited_;

	CDataCenter* data_center_;
	IGuiDataAction* gui_action_;

	std::vector<MarketDataServerConfig> quote_server_configs_;
	TradeServerConfig trade_server_config_;

	std::set<IMarketDataApi*> md_apis_;
	ITradeApi* trade_api_;

	std::set<Instrument> instruments_;
	std::set<std::string> subscribe_products_;
	std::set<std::string> subscribe_instrument_ids_;
};

