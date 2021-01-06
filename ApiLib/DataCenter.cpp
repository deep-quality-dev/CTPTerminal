#include "stdafx.h"
#include "DataCenter.h"
#include "GuiDataAction.h"
#include "TimeRegular.h"
#include "Utils/Logger.h"
#include "Utils/Utils.h"
#include <sstream>


CDataCenter::CDataCenter() : gui_action_(NULL)
{
	order_ref_ = 0;
}


CDataCenter::~CDataCenter()
{
}

void CDataCenter::OnRtnInstruments(const std::set<Instrument>& instruments)
{
	instruments_ = instruments;
}

Quote CDataCenter::OnRtnQuote(const Quote& squote)
{
	Quote quote = squote;

	auto it_instrument = instruments_.find(quote.instrument_id);
	if (it_instrument == instruments_.end()) {
		return Quote("");
	}

	if (it_instrument->exchange_id == SHFE ||	// 上期所
		it_instrument->exchange_id == DCE ||	// 大商所
		it_instrument->exchange_id == CFFEX) {	// 中金所
		// 郑商所的行情数据是没有乘的。
		quote.average_price = it_instrument->volume_multiple == 0 ? 0 : quote.average_price / it_instrument->volume_multiple;
		// 郑商所的夜盘日期是当天。
		// 其他交易所的夜盘日期是下一个交易日。
		SYSTEMTIME systime = Utils::GetSystemTime(quote.last_time);
		if (systime.wHour >= 21) {
			if (systime.wDayOfWeek == 1) { // monday
				systime = Utils::AddTime(systime, 0, -3);
			}
			else {
				systime = Utils::AddTime(systime, 0, -1);
			}
		}
		quote.last_time = Utils::CalcTimestampMilli(systime);

		if (quote.average_price <= 1e-4) {
			quote.average_price = quote.last_price;
		}
	}

// 	std::stringstream ss;
// 	ss << GetTimeString(quote.last_time) << ", "
// 		<< quote.instrument_id << ", "
// 		<< "最新价=" << quote.last_price << ", "
// 		<< "卖价=" << quote.bid_price1 << ", "
// 		<< "买价=" << quote.ask_price1;
// 	Utils::Log(ss.str());

	// Push Quote
	if (quotes_[quote.instrument_id][0]) {
		delete quotes_[quote.instrument_id][0];
	}
	quotes_[quote.instrument_id][0] = quotes_[quote.instrument_id][1];
	Quote* pquote = new Quote(quote);
	quotes_[quote.instrument_id][1] = pquote;

	server_time_ = quote.last_time;

	CalcPositionProfit();

	if (quote_callback_) {
		quote_callback_(quote);
	}

// 	// Write Quote
// 	SaveQuote(quote);

	return quote;
}

void CDataCenter::OnRspInstrumentMarginRate(const InstrumentMarginRate& margin_rate)
{
	margin_rates_.insert(margin_rate);
}

void CDataCenter::OnRspTradeAccount(const TradingAccount& account)
{
	trading_account_ = account;
	if (onaccount_callback_) {
		onaccount_callback_(trading_account_);
	}
}

void CDataCenter::OnRtnPositions(const std::set<Position>& positions)
{
	positions_ = positions;
}

std::set<PositionDetail> CDataCenter::OnRtnPositionDetails(const std::set<PositionDetail>& positions)
{
	{
		std::unique_lock<std::mutex> lk(mutex_positions_);
		position_details_ = positions;
	}
	// TODO, 从持仓明细合计持仓

	CalcPositionProfit();

	return position_details_;
}

void CDataCenter::OnRspQryOrders(const std::set<Order>& orders)
{

}

void CDataCenter::OnRspQryTrades(const std::set<Trade>& orders)
{

}

void CDataCenter::OnRtnOrder(const Order& order)
{
	order_ref2order_[order.GetKey().order_ref] = order;
	order_sysid2ref_[order.order_sys_id] = order.GetKey().order_ref;
	if (order.status == Status_Error || order.status == Status_Canceled) {
		if (order.offset_flag != Open) {
// 			// 如果正在等待报单回报，TODO
// 
// 			// 如果没有报单，在持仓列表里面删除
// 			for (auto it_position = positions_.begin(); it_position != positions_.end(); it_position++) {
// 				if (it_position->instrument_id == order.instrument_id &&
// 					it_position->direction == order.direction) {
// 					Position position = *it_position;
// 					int v = min(order.volume_remained, position.yesterday_volume);
// 					position.yesterday_volume -= v;
// 					position.today_volume += v;
// 					positions_.insert(position);
// 					break;
// 				}
// 			}
		}

// 		if (order.request_id > 0 && order.volume_remained > 0) {
// 			// 反复重行报单，最多9次
// 			double price = GetMarketPrice(order.instrument_id, order.direction);
// 			InsertOrder(order.instrument_id, order.offset_flag, order.direction, price, order.volume_remained);
// 		}
	}

	if (onorder_callback_) {
		onorder_callback_(order);
	}
}

void CDataCenter::OnRtnTrade(const Trade& trade)
{
	auto it = order_sysid2ref_.find(trade.order_sys_id);
	if (it != order_sysid2ref_.end()) {
		int order_ref = it->second;
		auto it_order = order_ref2order_.find(order_ref);
		if (it_order != order_ref2order_.end()) {
			if (trade.offset_flag == Open) {
				bool found = false;
				for (auto it_position = positions_.begin();
					it_position != positions_.end(); it_position++) {
					if (it_position->instrument_id == trade.instrument_id && it_position->direction == trade.direction) {
						Position position = *it_position;
						position.position_cost = (position.position_cost * position.volume() + trade.volume * trade.price) / (position.volume() + trade.volume);
						position.today_volume += trade.volume;
						positions_.erase(it_position);
						positions_.insert(position);
						found = true;
						break;
					}
				}

				if (!found) {
					Position position(trade.instrument_id, trade.direction);
					position.exchange_id = trade.exchange_id;
					position.today_volume = trade.volume;
					position.position_cost = trade.price;
					positions_.insert(position);
				}
			}
			else {
				int volume_traded = trade.volume;
				while (volume_traded > 0) {
					for (auto it_position = positions_.begin();
						it_position != positions_.end(); it_position++) {
						if (it_position->instrument_id == trade.instrument_id &&
							it_position->direction != trade.direction) {
							Position pos = *it_position;

							int v = min(volume_traded, pos.yesterday_volume);
							volume_traded -= v;
							pos.yesterday_volume -= v;

							v = min(volume_traded, pos.today_volume);
							volume_traded -= v;
							pos.today_volume -= v;

							if (pos.today_volume + pos.yesterday_volume == 0) {
								positions_.erase(pos);
								break;
							}
							else {
								positions_.erase(it_position);
								positions_.insert(pos);
							}
						}
					}
				}
			}

			CalcPositionProfit();

			if (ontrade_callback_) {
				ontrade_callback_(order_ref, trade);
			}
		}
	}
}

void CDataCenter::SaveQuote(const Quote& quote)
{
	// 如果在交易时间内，保存行情
	auto it_inst = instruments_.find(quote.instrument_id);
	if (it_inst == instruments_.end())
		return;

	if (!CTimeRegular::GetInstance().WithIn(it_inst->exchange_id, it_inst->product_id, quote.last_time))
		return;

	// 4点之前的行情都放在一起
	__time64_t time = quote.last_time;
	SYSTEMTIME systime = Utils::GetSystemTime(time);
	if (systime.wHour < 4) {
		systime = Utils::AddTime(systime, 0, -1);
	}
	std::string date = Utils::GetTimeString(systime, 0);

	std::stringstream ss;
	ss << "quote//" << quote.instrument_id << "_" << Utils::replace_all(date, "-", "") << ".csv";

	std::stringstream line;
	line << Utils::GetTimeString(quote.last_time) << ","
		<< quote.instrument_id << ","
		<< quote.last_price << ","
		<< quote.ask_price1 << ","
		<< quote.ask_volume1 << ","
		<< quote.bid_price1 << ","
		<< quote.bid_volume1 << ","
		<< quote.open_price << ","
		<< quote.close_price << ","
		<< quote.settlement_price << ","
		<< quote.highest_price << ","
		<< quote.lowest_price << ","
		<< quote.pre_settlement_price << ","
		<< quote.average_price << ","
		<< quote.trade_volume << ","
		<< quote.position_volume << std::endl;

	Utils::SaveFile(Utils::GetRelativePath(ss.str().c_str()), line.str().c_str());
}

double CDataCenter::GetMarketPrice(const std::string& instrument_id, Direction direction)
{
	auto it_quote = quotes_.find(instrument_id);
	if (instruments_.find(instrument_id) != instruments_.end() &&
		it_quote != quotes_.end() &&
		(it_quote->second[0] || it_quote->second[1])) {
		Quote* quote = it_quote->second[1] ? it_quote->second[1] : it_quote->second[0];
		return direction == Sell ? quote->lower_limit_price : quote->upper_limit_price;
	}
	return 0;
}

void CDataCenter::CalcPositionProfit()
{
	std::unique_lock<std::mutex> lk(mutex_positions_);

	std::set<Position> positions_bk;
	for (auto it_position = positions_.begin();
		it_position != positions_.end(); it_position++) {
		auto it_quote = quotes_.find(it_position->instrument_id);
		if (it_quote == quotes_.end()) {
			positions_bk.insert(*it_position); // ERROR
			continue;
		}

		Quote* quote = it_quote->second[1];
		if (quote == NULL) {
			positions_bk.insert(*it_position); // ERROR
			continue;
		}

		auto it_instrument = instruments_.find(quote->instrument_id);
		if (it_position->instrument_id == quote->instrument_id && it_instrument != instruments_.end()) {
			Position position = *it_position;
			int signal = it_position->direction == Buy ? 1 : -1;
			position.profit = signal * (quote->last_price - it_position->position_cost) * (it_position->today_volume + it_position->yesterday_volume) * it_instrument->volume_multiple;
			positions_bk.insert(position);
		}
		else {
			positions_bk.insert(*it_position); // ERROR
		}
	}
	positions_ = positions_bk;

	std::set<PositionDetail> position_details_bk;
	for (auto it_position = position_details_.begin();
		it_position != position_details_.end(); it_position++) {

		auto it_quote = quotes_.find(it_position->instrument_id);
		if (it_quote == quotes_.end()) {
			position_details_bk.insert(*it_position); // ERROR
			continue;
		}

		Quote* quote = it_quote->second[1];
		if (quote == NULL) {
			position_details_bk.insert(*it_position); // ERROR
			continue;
		}

		auto it_instrument = instruments_.find(quote->instrument_id);
		if (it_position->instrument_id == quote->instrument_id && it_instrument != instruments_.end()) {
			PositionDetail position_detail = *it_position;
			int signal = it_position->direction == Buy ? 1 : -1;
			position_detail.profit = signal * (quote->last_price - it_position->open_price) * it_position->volume * it_instrument->volume_multiple;
			position_details_bk.insert(position_detail);
		}
		else {
			position_details_bk.insert(*it_position); // ERROR
		}
	}
	position_details_ = position_details_bk;
}
