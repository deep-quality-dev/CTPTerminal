#pragma once

class IStrategy
{
public:
	IStrategy(CDataCenter* data_center, ITradeApi* trade_api) : trade_api_(trade_api), data_center_(data_center), order_ref_(0) {}

	CDataCenter* data_center() {
		return data_center_;
	}
	void set_data_center(CDataCenter* data_center) {
		data_center_ = data_center;
	}

	virtual void Initialize() = 0;

	virtual void OnQuoteCallback(const Quote& quote) = 0;
	virtual void OnTradeCallback(int order_ref, const Trade& trade) = 0;
	virtual void OnTradeAccountCallback(const TradingAccount& account) = 0;

	// 如果开平成功，返回OrderRef，则失败，就返回-1
	int InsertOrder(const std::string& instrument_id, OffsetFlag offset_flag, Direction direction, double price, int volume);
	// 如果开平成功，返回OrderRef，则失败，就返回-1
	int InsertMarketOrder(const std::string& instrument_id, OffsetFlag offset_flag, Direction direction, int volume);

protected:
	// 返回该方向的持仓手数
	int HasSellPosition(const std::string& instrument_id);
	int HasBuyPosition(const std::string& instrument_id);

protected:
	CDataCenter* data_center_;
	ITradeApi* trade_api_;

	int order_ref_;

};