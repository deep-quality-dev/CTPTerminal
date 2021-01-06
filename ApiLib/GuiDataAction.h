#pragma once

#include "DataTypes/DataTypeDefs.h"
#include <set>

typedef enum API_EVENT {
	ApiEvent_MdConnectTimeout,
	ApiEvent_MdConnectSuccess,
	ApiEvent_MdDisconnected,
	ApiEvent_TradeConnectTimeout,
	ApiEvent_TradeConnectSuccess,
	ApiEvent_TradeDisconnected,
	ApiEvent_AuthenticationFailed,
	ApiEvent_AuthenticationSuccess,
	ApiEvent_MdLoginFailed,
	ApiEvent_MdLoginSuccess,
	ApiEvent_MdLogoutSuccess,
	ApiEvent_TradeLoginFailed,
	ApiEvent_TradeLoginSuccess,
	ApiEvent_TradeLogoutSuccess,
	ApiEvent_SubscribeMarketData,
	ApiEvent_UnsubscribeMarketData,
	ApiEvent_QryOrderFailed,
	ApiEvent_QryOrderSuccess,
	ApiEvent_QryTradeFailed,
	ApiEvent_QryTradeSuccess,
	ApiEvent_QryPositionFailed,
	ApiEvent_QryPositionSuccess,
	ApiEvent_QryPositionDetailFailed,
	ApiEvent_QryPositionDetailSuccess,
	ApiEvent_QryTradingAccountFailed,
	ApiEvent_QryTradingAccountSuccess,
	ApiEvent_QryInstrumentFailed,
	ApiEvent_QryInstrumentSuccess,
	ApiEvent_QryInstrumentMarginRateFailed,
	ApiEvent_QryInstrumentMarginRateSuccess,
	ApiEvent_QrySettlementInfoConfirmFailed,
	ApiEvent_QrySettlementInfoConfirmSuccess,
	ApiEvent_QrySettlementInfoFailed,
	ApiEvent_QrySettlementInfoSuccess,
	ApiEvent_SettlementInfoFailed,
	ApiEvent_SettlementInfoSuccess,

} ApiEvent;

class IGuiDataAction {
public:
	virtual void OnLoginProcess(ApiEvent api_event, const char* content = NULL, int error_id = 0, const char* error_msg = NULL) = 0;

	virtual void RefreshAccount(const TradingAccount& account) = 0;

	virtual void RefreshQuotes(const std::set<Quote>& quotes) = 0;

	virtual void RefreshInstruments(const std::set<Instrument>& instruments) = 0;

	virtual void RefreshInstrumentMarginRate(const InstrumentMarginRate& margin_rate) = 0;

	virtual void RefreshPositions(const std::set<Position>& positions) = 0;
	virtual void RefreshPositionDetails(const std::set<PositionDetail>& positions) = 0;

	virtual void RefreshOrders(const std::set<Order>& orders) = 0;

	virtual void RefreshTrades(const std::set<Trade>& trades) = 0;

	virtual void RefreshOrder(const Order& order) = 0;

	virtual void RefreshTrade(const Trade& trade) = 0;
};