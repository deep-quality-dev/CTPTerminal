#pragma once

class IStrategy
{
public:
	IStrategy(CDataCenter* data_center, ITradeApi* trade_api) : 
		trade_api_(trade_api), data_center_(data_center), 
		order_ref_(0), order_count_(0), order_limit_(1), loss_limit_(5), order_interval_(5), last_order_time_(0),
		is_enable_trade_(false) {}

	CDataCenter* data_center() {
		return data_center_;
	}
	void set_data_center(CDataCenter* data_center) {
		data_center_ = data_center;
	}

	bool is_enable_trade() {
		return is_enable_trade_;
	}
	void set_enable_trade(bool enabled) {
		is_enable_trade_ = enabled;
	}

	int order_limit() {
		return order_limit_;
	}
	void set_order_limit(int limit) {
		order_limit_ = limit;
	}

	double loss_limit() {
		return loss_limit_;
	}
	void set_loss_limit(double limit) {
		loss_limit_ = limit;
	}

	int order_interval() {
		return order_interval_;
	}
	void set_order_interval(int interval) {
		order_interval_ = interval;
	}

	virtual void Initialize() = 0;

	virtual void CheckForceSettle() = 0;

	virtual void OnQuoteCallback(const Quote& quote) = 0;
	virtual void OnOrderCallback(const Order& order) = 0;
	virtual void OnTradeCallback(int order_ref, const Trade& trade) = 0;
	virtual void OnTradeAccountCallback(const TradingAccount& account) = 0;

	// 如果开平成功，返回OrderRef，如果失败，就返回-1，如果频繁报单返回-2
	int InsertOrder(const std::string& instrument_id, OffsetFlag offset_flag, Direction direction, double price, int volume);
	// 如果开平成功，返回OrderRef，如果失败，就返回-1，如果频繁报单返回-2
	int InsertMarketOrder(const std::string& instrument_id, OffsetFlag offset_flag, Direction direction, int volume);

protected:
	// 返回该方向的持仓手数
	int HasSellPosition(const std::string& instrument_id);
	int HasBuyPosition(const std::string& instrument_id);

protected:
	CDataCenter* data_center_;
	ITradeApi* trade_api_;

	int order_ref_;

	int order_count_; // 当前的报单次数
	int order_limit_; // 当日报单限制次数

	double loss_limit_;

	int order_interval_; // 报单之间的时间间隔，单位为秒
	__time64_t last_order_time_; // 最后报单时间

	bool is_enable_trade_;
};