#pragma once

#include "DataTypes/DataTypeDefs.h"
#include <map>
#include <set>
#include <mutex>
#include <functional>

class ITradeApi;

class CDataCenter
{
public:
	CDataCenter();
	~CDataCenter();

	void set_trade_api(ITradeApi* trade_api) {
		trade_api_ = trade_api;
	}

	std::set<Position> positions() {
		return positions_;
	}
	std::set<PositionDetail> position_details() {
		return position_details_;
	}

	void SetOrderRef(int order_ref) {
		order_ref_ = order_ref;
	}

	typedef std::function<void(const Quote&)> QuoteCallback;
	void SetQuoteCallback(const QuoteCallback& callback) {
		quote_callback_ = callback;
	}
	typedef std::function<void(const Trade&)> OnTradeCallback;
	void SetOnTradeCallback(const OnTradeCallback& callback) {
		ontrade_callback_ = callback;
	}
	typedef std::function<void(const TradingAccount&)> OnTradeAccountCallback;
	void SetOnTradeAccountCallback(const OnTradeAccountCallback& callback) {
		onaccount_callback_ = callback;
	}

	void OnRtnInstruments(const std::set<Instrument>& instruments);
	Quote OnRtnQuote(const Quote& quote);
	void OnRspInstrumentMarginRate(const InstrumentMarginRate& margin_rate);
	void OnRspTradeAccount(const TradingAccount& account);
	void OnRtnPositions(const std::set<Position>& positions);
	std::set<PositionDetail> OnRtnPositionDetails(const std::set<PositionDetail>& positions);
	void OnRspQryOrders(const std::set<Order>& orders);
	void OnRspQryTrades(const std::set<Trade>& orders);
	void OnRtnOrder(const Order& order);
	void OnRtnTrade(const Trade& trade);

	void InsertOrder(const std::string& instrument_id, OffsetFlag offset_flag, Direction direction, double price, int volume);
	void InsertMarketOrder(const std::string& instrument_id, OffsetFlag offset_flag, Direction direction, int volume);

protected:
	void SaveQuote(const Quote& quote);

	double GetMarketPrice(const std::string& instrument_id, Direction direction);

	void CalcPositionProfit();

protected:
	ITradeApi* trade_api_;

	QuoteCallback quote_callback_;
	OnTradeCallback ontrade_callback_;
	OnTradeAccountCallback onaccount_callback_;

	std::set<Instrument> instruments_;
	std::map<std::string, QuoteDeque> quotes_;
	std::set<InstrumentMarginRate> margin_rates_;
	TradingAccount trading_account_;
	std::set<Position> positions_;
	std::set<PositionDetail> position_details_;

	std::map<int, Order> order_ref2order_;
	std::map<std::string, int> order_sysid2ref_;

	int order_ref_;

	__time64_t server_time_;

	std::mutex mutex_positions_;
};

