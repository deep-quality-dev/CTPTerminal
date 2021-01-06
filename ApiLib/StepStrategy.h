#pragma once

#include "DataCenter.h"
#include "TradeApi.h"
#include "Strategy.h"
#include <functional>
#include <deque>
#include <map>

class CStepStrategy : public IStrategy
{
public:
	CStepStrategy(CDataCenter* data_center, ITradeApi* trade_api);
	~CStepStrategy();

	const int MaxQuoteLimitSize = 200;

	typedef std::pair<Direction, std::string> SignalFuncKey;

	std::string main_instrument_id() {
		return main_instrument_id_;
	}
	void set_main_instrument_id(const std::string& id) {
		main_instrument_id_ = id;
	}

	std::string sub_instrument_id() {
		return sub_instrument_id_;
	}
	void set_sub_instrument_id(const std::string& id) {
		sub_instrument_id_ = id;
	}

	int volume() {
		return volume_;
	}
	void set_volume(int volume) {
		volume_ = volume;
	}

	int ma_period() {
		return ma_period_;
	}
	void set_ma_period(int period) {
		ma_period_ = period;
	}

	virtual void Initialize() override;

	virtual void CheckForceSettle() override;

	virtual void OnQuoteCallback(const Quote& quote) override;
	virtual void OnOrderCallback(const Order& order) override;
	virtual void OnTradeCallback(int order_ref, const Trade& trade) override;
	virtual void OnTradeAccountCallback(const TradingAccount& account) override;

protected:
	bool PushQuote(const Quote& quote);

	// MA下穿现价
	bool MASellCross(const std::string& main_instrument_id, const std::string& sub_instrument_id,
		int param1, int param2, int param3);
	// MA上穿现价
	bool MABuyCross(const std::string& main_instrument_id, const std::string& sub_instrument_id,
		int param1, int param2, int param3);

private:
	std::string main_instrument_id_;
	std::string sub_instrument_id_;
	int volume_;
	int ma_period_;

	std::map<std::string, std::deque<Quote>> quotes_map_;

	TradingAccount account_;

	std::map<SignalFuncKey, std::function<bool()>> open_signals_;
	std::map<SignalFuncKey, std::function<bool()>> close_signals_;

	std::set<int> pending_order_refs_;
};

