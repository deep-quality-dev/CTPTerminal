﻿#pragma once

#include "DataTypes/GuiDataAction.h"

class CQuoteService;

class CQuoteServerHandler : public IGuiDataAction
{
public:
	CQuoteServerHandler(CQuoteService* quote_service);
	~CQuoteServerHandler();

	virtual void OnLoginProcess(ApiEvent api_event, const char* content = NULL, int error_id = 0, const char* error_msg = NULL);

	virtual void RefreshAccount(const TradingAccount& account);

	virtual void RefreshQuotes(const std::set<Quote>& quotes);

	virtual void RefreshInstruments(const std::set<Instrument>& instruments);

	virtual void RefreshPositions(const std::set<Position>& positions);

	virtual void RefreshOrders(const std::set<Order>& orders);

	virtual void RefreshTrades(const std::set<Trade>& trades);

	virtual void RefreshOrder(const Order& order);

	virtual void RefreshTrade(const Trade& trade);

private:
	CQuoteService* quote_service_;
};

