#pragma once

#include "DataTypes/DataTypeDefs.h"
#include <map>
#include <set>

class CDataCenter
{
public:
	CDataCenter();
	~CDataCenter();

	std::set<Position> positions() {
		return positions_;
	}

	void SetOrderRef(int order_ref) {
		order_ref_ = order_ref;
	}

	void OnRtnInstruments(const std::set<Instrument>& instruments);
	void OnRtnQuote(const Quote& quote);
	void OnRspTradeAccount(const TradingAccount& account);
	void OnRtnPositions(const std::set<Position>& positions);
	void OnRspQryOrders(const std::set<Order>& orders);
	void OnRspQryTrades(const std::set<Trade>& orders);
	void OnRtnOrder(const Order& order);
	void OnRtnTrade(const Trade& trade);

	void InsertOrder(const std::string& instrument_id, OffsetFlag offset_flag, Direction direction, double price, int volume);

protected:
	void SaveQuote(const Quote& quote);

	double GetMarketPrice(const std::string& instrument_id, Direction direction);

protected:
	std::set<Instrument> instruments_;
	std::map<std::string, QuoteDeque> quotes_;
	TradingAccount trading_account_;
	std::set<Position> positions_;

	std::map<int, Order> order_ref2order_;
	std::map<std::string, int> order_sysid2ref_;

	int order_ref_;

	__time64_t server_time_;
};

