﻿#pragma once

#include "DataTypeDefs.h"

typedef enum API_EVENT {
	ApiEvent_ConnectTimeout,
	ApiEvent_ConnectSuccess,
	ApiEvent_Disconnected,
	ApiEvent_AuthenticationFailed,
	ApiEvent_AuthenticationSuccess,
	ApiEvent_LoginFailed,
	ApiEvent_LoginSuccess,
	ApiEvent_SubscribeMarketData,
	ApiEvent_UnsubscribeMarketData,
	ApiEvent_QryOrderFailed,
	ApiEvent_QryOrderSuccess,
	ApiEvent_QryTradeFailed,
	ApiEvent_QryTradeSuccess,
	ApiEvent_QryPositionFailed,
	ApiEvent_QryPositionSuccess,
	ApiEvent_QryTradingAccountFailed,
	ApiEvent_QryTradingAccountSuccess,
	ApiEvent_QryInstrumentFailed,
	ApiEvent_QryInstrumentSuccess

} ApiEvent;

class IGuiDataAction {
public:
	virtual void OnLoginProcess(ApiEvent api_event, const char* content = NULL, int error_id = 0, const char* error_msg = NULL) = 0;

	virtual void RefreshAccount(const TradingAccount& account) = 0;

	virtual void RefreshQuotes(const std::set<Quote>& quotes) = 0;

	virtual void RefreshInstruments(const std::set<Instrument>& instruments) = 0;

	virtual void RefreshPositions(const std::set<Position>& positions) = 0;

	virtual void RefreshOrders(const std::set<Order>& order) = 0;

	virtual void RefreshTrades(const std::set<Trade>& trade) = 0;

	virtual void RefreshOrder(const Order& order) = 0;

	virtual void RefreshTrade(const Trade& trade) = 0;
};