#include "stdafx.h"
#include "ArbitrageStrategy.h"
#include "Utils/Utils.h"


#define REGISTER_OPEN_SIGNAL_FUNC(direction, name, func, instrument_id1, instrument_id2, param1, param2, param3) \
	open_signals_[std::make_pair(direction, name)] = std::bind(&CArbitrageStrategy::func, this, instrument_id1, instrument_id2, param1, param2, param3);
#define REGISTER_CLOSE_SIGNAL_FUNC(direction, name, func, instrument_id1, instrument_id2, param1, param2, param3) \
	close_signals_[std::make_pair(direction, name)] = std::bind(&CArbitrageStrategy::func, this, instrument_id1, instrument_id2, param1, param2, param3);

CArbitrageStrategy::CArbitrageStrategy(CDataCenter* data_center, ITradeApi* trade_api) : IStrategy(data_center, trade_api), volume_(0), ma_period_(0)
{
}


CArbitrageStrategy::~CArbitrageStrategy()
{
}

void CArbitrageStrategy::Initialize()
{
	order_ref_ = data_center()->init_order_ref();

	REGISTER_OPEN_SIGNAL_FUNC(Direction::Buy, "MA上穿现价", MABuyCross, main_instrument_id(), sub_instrument_id(), ma_period(), 0, 0);
	REGISTER_OPEN_SIGNAL_FUNC(Direction::Sell, "MA下穿现价", MASellCross, main_instrument_id(), sub_instrument_id(), ma_period(), 0, 0);

	REGISTER_CLOSE_SIGNAL_FUNC(Direction::Buy, "MA上穿现价", MABuyCross, main_instrument_id(), sub_instrument_id(), ma_period(), 0, 0);
	REGISTER_CLOSE_SIGNAL_FUNC(Direction::Sell, "MA下穿现价", MASellCross, main_instrument_id(), sub_instrument_id(), ma_period(), 0, 0);
}

void CArbitrageStrategy::OnQuoteCallback(const Quote& quote)
{
	IStrategy::OnQuoteCallback(quote);

	if (!PushQuote(quote)) {
		return;
	}

	int order_ref = -1;
	int pos_volume = 0;
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
	else {
		bool has_sell_signal = true;
		for (auto it_func = open_signals_.begin(); it_func != open_signals_.end(); it_func++) {
			if (it_func->first.first == Direction::Sell) {
				has_sell_signal &= it_func->second();
			}
		}
		if (has_sell_signal) {
			order_ref = InsertMarketOrder(main_instrument_id(), OffsetFlag::Open, Direction::Sell, volume());
		}
		else {
			bool has_buy_signal = true;
			for (auto it_func = open_signals_.begin(); it_func != open_signals_.end(); it_func++) {
				if (it_func->first.first == Direction::Buy) {
					has_buy_signal &= it_func->second();
				}
			}
			if (has_buy_signal) {
				order_ref = InsertMarketOrder(main_instrument_id(), OffsetFlag::Open, Direction::Buy, volume());
			}
		}
	}

	if (order_ref > 0) {
		RollbackOrderKey(order_ref);
	}
}

void CArbitrageStrategy::OnOrderCallback(const Order& order)
{
	IStrategy::OnOrderCallback(order);
}

void CArbitrageStrategy::OnTradeCallback(int order_ref, const Trade& trade)
{
	IStrategy::OnTradeCallback(order_ref, trade);
}

void CArbitrageStrategy::OnTradeAccountCallback(const TradingAccount& account)
{
	IStrategy::OnTradeAccountCallback(account);
}

bool CArbitrageStrategy::PushQuote(const Quote& quote)
{
	if (quote.instrument_id != main_instrument_id_ && quote.instrument_id != sub_instrument_id_) {
		return false;
	}

	// 把主力和副主力合约的行情时间保持一致
	std::deque<Quote>* main_quotes = NULL, *sub_quotes = NULL;
	for (auto it_quotes = quotes_map_.begin(); it_quotes != quotes_map_.end(); it_quotes++) {
		if (it_quotes->first == quote.instrument_id) {
			main_quotes = &it_quotes->second;
		}
		else if (it_quotes->first == quote.instrument_id) {
			sub_quotes = &it_quotes->second;
		}
	}
	if (main_quotes == NULL) {
		quotes_map_[main_instrument_id()] = std::deque<Quote>();
		main_quotes = &quotes_map_[main_instrument_id()];
	}
	if (sub_quotes == NULL) {
		quotes_map_[sub_instrument_id()] = std::deque<Quote>();
		sub_quotes = &quotes_map_[sub_instrument_id()];
	}

	if (quote.instrument_id == main_instrument_id()) {
		Quote last_quote = sub_quotes->size() > 0 ? sub_quotes->back() : Quote(sub_instrument_id());
		last_quote.last_time = quote.last_time;
		main_quotes->push_back(quote);
		sub_quotes->push_back(quote);
	}
	else if (quote.instrument_id == sub_instrument_id()) {
		Quote last_quote = main_quotes->size() > 0 ? main_quotes->back() : Quote(main_instrument_id());
		last_quote.last_time = quote.last_time;
		sub_quotes->push_back(quote);
		main_quotes->push_back(quote);
	}

	if (main_quotes->size() >= MaxQuoteLimitSize) {
		main_quotes->pop_front();
	}
	if (sub_quotes->size() >= MaxQuoteLimitSize) {
		sub_quotes->pop_front();
	}
	return true;
}

bool CArbitrageStrategy::MASellCross(const std::string& main_instrument_id, const std::string& sub_instrument_id, int param1, int param2, int param3)
{
	int ma_period = param1;
	if (ma_period < 1) {
		return false;
	}

	std::deque<Quote>* main_quotes = NULL, *sub_quotes = NULL;
	for (auto it_quotes = quotes_map_.begin(); it_quotes != quotes_map_.end(); it_quotes++) {
		if (it_quotes->first == main_instrument_id) {
			main_quotes = &it_quotes->second;
		}
		else if (it_quotes->first == sub_instrument_id) {
			sub_quotes = &it_quotes->second;
		}
	}
	if (main_quotes == NULL || sub_quotes == NULL) {
		return false;
	}

	double sum = 0;
	size_t size = main_quotes->size();
	for (size_t idx = size - 1; idx >= 0 && ma_period > 0; idx--, ma_period--) {
		Quote main_quote = (*main_quotes)[idx];
		Quote sub_quote = (*sub_quotes)[idx];

		sum += main_quote.bid_price1 - sub_quote.bid_price1;
	}
	sum /= param1;

	Quote main_quote = (*main_quotes)[size - 1];
	Quote sub_quote = (*sub_quotes)[size - 1];

	Quote main_prev_quote = (*main_quotes)[size - 2];
	Quote sub_prev_quote = (*sub_quotes)[size - 2];

	if (Utils::CompareDouble(main_quote.ask_price1 - sub_quote.ask_price1, sum) < 0 && Utils::CompareDouble(sum, main_prev_quote.ask_price1 - sub_prev_quote.ask_price1) < 0) {
		return true;
	}

	return false;
}

bool CArbitrageStrategy::MABuyCross(const std::string& main_instrument_id, const std::string& sub_instrument_id, int param1, int param2, int param3)
{
	int ma_period = param1;
	if (ma_period < 1) {
		return false;
	}

	std::deque<Quote>* main_quotes = NULL, *sub_quotes = NULL;
	for (auto it_quotes = quotes_map_.begin(); it_quotes != quotes_map_.end(); it_quotes++) {
		if (it_quotes->first == main_instrument_id) {
			main_quotes = &it_quotes->second;
		}
		else if (it_quotes->first == sub_instrument_id) {
			sub_quotes = &it_quotes->second;
		}
	}
	if (main_quotes == NULL || sub_quotes == NULL) {
		return false;
	}

	double sum = 0;
	size_t size = main_quotes->size();
	for (size_t idx = size - 1; idx >= 0 && ma_period > 0; idx--, ma_period--) {
		Quote main_quote = (*main_quotes)[idx];
		Quote sub_quote = (*sub_quotes)[idx];

		sum += main_quote.ask_price1 - sub_quote.ask_price1;
	}
	sum /= param1;

	Quote main_quote = (*main_quotes)[size - 1];
	Quote sub_quote = (*sub_quotes)[size - 1];

	Quote main_prev_quote = (*main_quotes)[size - 2];
	Quote sub_prev_quote = (*sub_quotes)[size - 2];

	if (Utils::CompareDouble(main_quote.bid_price1 - sub_quote.bid_price1, sum) < 0 && Utils::CompareDouble(sum, main_prev_quote.bid_price1 - sub_prev_quote.bid_price1) < 0) {
		return true;
	}

	return false;
}
