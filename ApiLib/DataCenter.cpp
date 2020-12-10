#include "stdafx.h"
#include "DataCenter.h"
#include "TimeRegular.h"
#include "Utils/Utils.h"
#include "Utils/Logger.h"
#include <sstream>


CDataCenter::CDataCenter()
{
}


CDataCenter::~CDataCenter()
{
}

void CDataCenter::OnRtnInstruments(const std::set<Instrument>& instruments)
{
	instruments_ = instruments;
}

void CDataCenter::OnRtnQuote(const Quote& squote)
{
	Quote quote = squote;

	auto it_instrument = instruments_.find(quote.instrument_id);
	if (it_instrument == instruments_.end()) {
		return;
	}

	if (it_instrument->exchange_id == SHFE ||	// 上期所
		it_instrument->exchange_id == DCE ||	// 大商所
		it_instrument->exchange_id == CFFEX) {	// 中金所
		// 郑商所的行情数据是没有乘的。
		quote.average_price = it_instrument->volume_multiple == 0 ? 0 : quote.average_price / it_instrument->volume_multiple;
		// 郑商所的夜盘日期是当天。
		// 其他交易所的夜盘日期是下一个交易日。
		SYSTEMTIME systime = GetSystemTime(quote.last_time);
		if (systime.wHour >= 21) {
			if (systime.wDayOfWeek == 1) { // monday
				systime = AddTime(systime, 0, -3);
			}
			else {
				systime = AddTime(systime, 0, -1);
			}
		}
		quote.last_time = CalcTimestampMilli(systime);

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

	// Write Quote
	SaveQuote(quote);
}

void CDataCenter::OnRtnPositions(const std::set<Position>& positions)
{
	positions_ = positions;
}

void CDataCenter::OnRspQryOrders(const std::set<Order>& orders)
{

}

void CDataCenter::OnRspQryTrades(const std::set<Trade>& orders)
{

}

void CDataCenter::OnRtnOrder(const Order& order)
{

}

void CDataCenter::OnRtnTrade(const Trade& trade)
{

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
	SYSTEMTIME systime = GetSystemTime(time);
	if (systime.wHour < 4) {
		systime = AddTime(systime, 0, -1);
	}
	std::string date = GetTimeString(systime, 0);

	std::stringstream ss;
	ss << "quote//" << quote.instrument_id << "_" << replace_all(date, "-", "") << ".csv";

	std::stringstream line;
	line << GetTimeString(quote.last_time) << ","
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

	SaveFile(GetRelativePath(ss.str().c_str()), line.str().c_str());
}
