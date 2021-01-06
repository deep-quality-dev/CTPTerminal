#include "stdafx.h"
#include "StepStrategy.h"
#include "DataTypes/Formatter.h"
#include "Utils/Utils.h"
#include "Utils/Logger.h"
#include <sstream>


#define REGISTER_OPEN_SIGNAL_FUNC(direction, name, func, instrument_id1, instrument_id2, param1, param2, param3) \
	open_signals_[std::make_pair(direction, name)] = std::bind(&CStepStrategy::func, this, instrument_id1, instrument_id2, param1, param2, param3);
#define REGISTER_CLOSE_SIGNAL_FUNC(direction, name, func, instrument_id1, instrument_id2, param1, param2, param3) \
	close_signals_[std::make_pair(direction, name)] = std::bind(&CStepStrategy::func, this, instrument_id1, instrument_id2, param1, param2, param3);

CStepStrategy::CStepStrategy(CDataCenter* data_center, ITradeApi* trade_api) : IStrategy(data_center, trade_api), volume_(0), ma_period_(0)
{
}


CStepStrategy::~CStepStrategy()
{
}

void CStepStrategy::Initialize()
{
	order_ref_ = data_center()->order_ref();

	REGISTER_OPEN_SIGNAL_FUNC(Direction::Buy, "MA上穿现价", MABuyCross, main_instrument_id(), sub_instrument_id(), ma_period(), 0, 0);
	REGISTER_OPEN_SIGNAL_FUNC(Direction::Sell, "MA下穿现价", MASellCross, main_instrument_id(), sub_instrument_id(), ma_period(), 0, 0);

	REGISTER_CLOSE_SIGNAL_FUNC(Direction::Buy, "MA上穿现价", MABuyCross, main_instrument_id(), sub_instrument_id(), ma_period(), 0, 0);
	REGISTER_CLOSE_SIGNAL_FUNC(Direction::Sell, "MA下穿现价", MASellCross, main_instrument_id(), sub_instrument_id(), ma_period(), 0, 0);
}

void CStepStrategy::CheckForceSettle()
{
	// 如果平仓盈亏达到70%以上的资金，立即强制并停止运行
	double profit = account_.close_profit + account_.position_profit;

	if (profit < 0 && account_.available * 0.25 <= abs(profit)) {
		Utils::Log("分控强平");

		int pos_volume = 0, order_ref = -1;
		if ((pos_volume = HasSellPosition(main_instrument_id())) > 0) {
			order_ref = InsertMarketOrder(main_instrument_id(), OffsetFlag::CloseToday, Direction::Buy, pos_volume);
		}
		else if ((pos_volume = HasBuyPosition(main_instrument_id())) > 0) {
			order_ref = InsertMarketOrder(main_instrument_id(), OffsetFlag::CloseToday, Direction::Sell, pos_volume);
		}
		if (order_ref > 0) {
			pending_order_refs_.insert(order_ref);
		}
		set_enable_trade(false);
	}
}

void CStepStrategy::OnQuoteCallback(const Quote& quote)
{
	if (!PushQuote(quote)) {
		return;
	}

	if (pending_order_refs_.size() > 0) {
		return;
	}

	CheckForceSettle();

	int pos_volume = 0, order_ref = -1;
	if ((pos_volume = HasSellPosition(main_instrument_id())) > 0) {
		bool has_signal = true;
		for (auto it_func = close_signals_.begin(); it_func != close_signals_.end(); it_func++) {
			if (it_func->first.first == Direction::Buy) {
				has_signal &= it_func->second();
			}
		}
		if (has_signal) {
			order_ref = InsertMarketOrder(main_instrument_id(), OffsetFlag::CloseToday, Direction::Buy, pos_volume);
		}
	}
	else if ((pos_volume = HasBuyPosition(main_instrument_id())) > 0) {
		bool has_signal = true;
		for (auto it_func = close_signals_.begin(); it_func != close_signals_.end(); it_func++) {
			if (it_func->first.first == Direction::Sell) {
				has_signal &= it_func->second();
			}
		}
		if (has_signal) {
			order_ref = InsertMarketOrder(main_instrument_id(), OffsetFlag::CloseToday, Direction::Sell, pos_volume);
		}
	}
	else if (is_enable_trade()) {
		bool has_sell_signal = true;
		for (auto it_func = open_signals_.begin(); it_func != open_signals_.end(); it_func++) {
			if (it_func->first.first == Direction::Sell) {
				has_sell_signal &= it_func->second();
			}
		}

		bool has_buy_signal = true;
		for (auto it_func = open_signals_.begin(); it_func != open_signals_.end(); it_func++) {
			if (it_func->first.first == Direction::Buy) {
				has_buy_signal &= it_func->second();
			}
		}

		if (has_sell_signal) {
			order_ref = InsertMarketOrder(main_instrument_id(), OffsetFlag::Open, Direction::Sell, volume());
		}
		else if (has_buy_signal) {
			order_ref = InsertMarketOrder(main_instrument_id(), OffsetFlag::Open, Direction::Buy, volume());
		}
	}

	if (order_ref > 0) {
		pending_order_refs_.insert(order_ref);
	}
}

void CStepStrategy::OnOrderCallback(const Order& order)
{
	if (order.status == Status_Error || order.status == Status_Canceled) {
		int order_ref = order.GetKey().order_ref;
		auto it = pending_order_refs_.find(order_ref);
		if (it != pending_order_refs_.end()) {
			pending_order_refs_.erase(order_ref);
		}
	}
}

void CStepStrategy::OnTradeCallback(int order_ref, const Trade& trade)
{
	auto it = pending_order_refs_.find(order_ref);
	if (it != pending_order_refs_.end()) {
		pending_order_refs_.erase(order_ref);
	}

	std::set<Position> positions = data_center()->positions();
	Utils::Log("查询投资者持仓 >>");
	for (auto it_position = positions.begin(); it_position != positions.end(); it_position++) {
		std::stringstream ss;
		ss << "===============" << std::endl
			<< "交易所代码, " << CFormatter::GetInstance().ExchangeID2string(it_position->exchange_id) << std::endl
			<< "合约名称, " << it_position->instrument_id << std::endl
			<< "买卖方向, " << CFormatter::GetInstance().Direction2string(it_position->direction) << std::endl
			<< "上日持仓, " << it_position->yesterday_volume << std::endl
			<< "今日持仓, " << it_position->today_volume << std::endl
			<< "价格, " << it_position->position_cost << std::endl
			<< "手续费, " << it_position->commission << std::endl
			<< "持仓盈亏, " << it_position->profit << std::endl;

		Utils::Log(ss.str());
	}
	Utils::Log("查询投资者持仓 <<");
}

void CStepStrategy::OnTradeAccountCallback(const TradingAccount& account)
{
	account_ = account;
}

bool CStepStrategy::PushQuote(const Quote& quote)
{
	if (quote.instrument_id != main_instrument_id_ && quote.instrument_id != sub_instrument_id_) {
		return false;
	}

	for (auto it_quotes = quotes_map_.begin(); it_quotes != quotes_map_.end(); it_quotes++) {
		if (it_quotes->first == quote.instrument_id) {
			it_quotes->second.push_back(quote);

			if (it_quotes->second.size() >= MaxQuoteLimitSize) {
				it_quotes->second.pop_front();
			}
			return true;
		}
	}

	std::deque<Quote>& quotes = quotes_map_[quote.instrument_id];
	quotes.push_back(quote);

	return true;
}

bool CStepStrategy::MASellCross(const std::string& main_instrument_id, const std::string& sub_instrument_id, int param1, int param2, int param3)
{
	int ma_period = param1;
	if (ma_period < 1) {
		return false;
	}

	std::deque<Quote>* main_quotes = NULL;
	for (auto it_quotes = quotes_map_.begin(); it_quotes != quotes_map_.end(); it_quotes++) {
		if (it_quotes->first == main_instrument_id) {
			main_quotes = &it_quotes->second;
		}
	}
	if (main_quotes == NULL) {
		return false;
	}

	double sum = 0;
	size_t size = main_quotes->size();
	for (size_t idx = size - 1; idx >= 0 && ma_period > 0; idx--, ma_period--) {
		Quote main_quote = (*main_quotes)[idx];
		sum += main_quote.bid_price1;
	}
	sum /= param1;

	Quote main_quote = (*main_quotes)[size - 1];
	Quote main_prev_quote = (*main_quotes)[size - 2];

	if (Utils::CompareDouble(main_quote.ask_price1, sum) < 0 && Utils::CompareDouble(sum, main_prev_quote.ask_price1) < 0) {
		return true;
	}

	return false;
}

bool CStepStrategy::MABuyCross(const std::string& main_instrument_id, const std::string& sub_instrument_id, int param1, int param2, int param3)
{
	int ma_period = param1;
	if (ma_period < 1) {
		return false;
	}

	std::deque<Quote>* main_quotes = NULL;
	for (auto it_quotes = quotes_map_.begin(); it_quotes != quotes_map_.end(); it_quotes++) {
		if (it_quotes->first == main_instrument_id) {
			main_quotes = &it_quotes->second;
		}
	}
	if (main_quotes == NULL) {
		return false;
	}

	double sum = 0;
	size_t size = main_quotes->size();
	for (size_t idx = size - 1; idx >= 0 && ma_period > 0; idx--, ma_period--) {
		Quote main_quote = (*main_quotes)[idx];

		sum += main_quote.ask_price1;
	}
	sum /= param1;

	Quote main_quote = (*main_quotes)[size - 1];
	Quote main_prev_quote = (*main_quotes)[size - 2];

	if (Utils::CompareDouble(main_quote.bid_price1, sum) < 0 && Utils::CompareDouble(sum, main_prev_quote.bid_price1) < 0) {
		return true;
	}

	return false;
}
