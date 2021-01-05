﻿#pragma once

#include "DataTypes/DataTypeDefs.h"
#include <map>
#include <set>
#include <mutex>
#include <functional>

class IGuiDataAction;

class CDataCenter
{
public:
	CDataCenter();
	~CDataCenter();

	IGuiDataAction* gui_action() {
		return gui_action_;
	}
	void set_gui_action(IGuiDataAction* gui_action) {
		gui_action_ = gui_action;
	}

	std::set<Position> positions() {
		return positions_;
	}
	std::set<PositionDetail> position_details() {
		return position_details_;
	}

	int order_ref() {
		return order_ref_;
	}
	void set_order_ref(int order_ref) {
		order_ref_ = order_ref;
	}

	typedef std::function<void(const Quote&)> OnQuoteCallback;
	void SetQuoteCallback(const OnQuoteCallback& callback) {
		quote_callback_ = callback;
	}
	typedef std::function<void(int, const Trade&)> OnTradeCallback;
	void SetOnTradeCallback(const OnTradeCallback& callback) {
		ontrade_callback_ = callback;
	}
	typedef std::function<void(const TradingAccount&)> OnTradeAccountCallback;
	void SetOnTradeAccountCallback(const OnTradeAccountCallback& callback) {
		onaccount_callback_ = callback;
	}

	double GetMarketPrice(const std::string& instrument_id, Direction direction);

	virtual void OnRtnInstruments(const std::set<Instrument>& instruments);
	virtual Quote OnRtnQuote(const Quote& quote);
	virtual void OnRspInstrumentMarginRate(const InstrumentMarginRate& margin_rate);
	virtual void OnRspTradeAccount(const TradingAccount& account);
	virtual void OnRtnPositions(const std::set<Position>& positions);
	virtual std::set<PositionDetail> OnRtnPositionDetails(const std::set<PositionDetail>& positions);
	virtual void OnRspQryOrders(const std::set<Order>& orders);
	virtual void OnRspQryTrades(const std::set<Trade>& orders);
	virtual void OnRtnOrder(const Order& order);
	virtual void OnRtnTrade(const Trade& trade);

protected:
	void SaveQuote(const Quote& quote);

	void CalcPositionProfit();

protected:
	IGuiDataAction* gui_action_;

	OnQuoteCallback quote_callback_;
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

