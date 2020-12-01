#include "stdafx.h"
#include "DataCenter.h"
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
			if (systime.wDayOfWeek == 0) { // month
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

	std::stringstream ss;
	ss << GetTimeString(quote.last_time) << ", "
		<< quote.instrument_id << ", "
		<< "最新价=" << quote.last_price << ", "
		<< "卖价=" << quote.bid_price1 << ", "
		<< "买价=" << quote.ask_price1;
	Utils::Log(ss.str());

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

void CDataCenter::SaveQuote(const Quote& quote)
{
	std::stringstream ss;
	ss << "quote//" << quote.instrument_id << "_" << replace(GetCurrentDate(), "-", "") << ".csv";

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
		<< quote.position_volume;

	SaveFile(GetRelativePath(ss.str().c_str()), line.str().c_str());
}
