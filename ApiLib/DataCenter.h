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

	void OnRtnInstruments(const std::set<Instrument>& instruments);
	void OnRtnQuote(const Quote& quote);
	void OnRtnPositions(const std::set<Position>& positions);
	void OnRspQryOrders(const std::set<Order>& orders);
	void OnRspQryTrades(const std::set<Trade>& orders);
	void OnRtnOrder(const Order& order);
	void OnRtnTrade(const Trade& trade);

protected:
	void SaveQuote(const Quote& quote);

protected:
	std::set<Instrument> instruments_;
	std::map<std::string, QuoteDeque> quotes_;
	std::set<Position> positions_;

	__time64_t server_time_;
};

