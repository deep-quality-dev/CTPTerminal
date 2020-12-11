#pragma once

#include "DataTypes/DataTypeDefs.h"
#include <vector>

class ITradeApi
{
public:
	// 初始化
	virtual void Initialize(const std::string& broker_id,
		const std::string& user_id,
		const std::string& password,
		const std::vector<std::string> fronts,
		const std::string& user_product_info,
		const std::string& auth_code,
		const std::string& app_id) = 0;

	virtual int ReqQryTradingAccount() = 0;

	virtual int ReqQryAllInstrument() = 0;

	virtual int ReqQryOrder() = 0;

	virtual int ReqQryTrade() = 0;

	virtual int ReqQryPosition() = 0;

	virtual int ReqQryPositionDetail() = 0;

	virtual int ReqQryDepthMarketData() = 0;

	virtual void ReqInsertOrder(const OrderInsert& order_insert) = 0;

	virtual void ReqCancelOrder(const Order& order) = 0;

	// 查询保证金比率
	virtual void ReqQryMarginRate(const std::string& instrument_id) = 0;
};