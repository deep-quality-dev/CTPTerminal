#include "stdafx.h"
#include "DataCenter.h"
#include "Utils/Utils.h"


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

	// Push Quote
	if (quotes_[quote.instrument_id][0]) {
		delete quotes_[quote.instrument_id][0];
	}
	quotes_[quote.instrument_id][0] = quotes_[quote.instrument_id][1];
	Quote* pquote = new Quote(quote);
	quotes_[quote.instrument_id][1] = pquote;

	server_time_ = quote.last_time;
}
