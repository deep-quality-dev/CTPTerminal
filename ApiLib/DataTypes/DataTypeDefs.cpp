#include "stdafx.h"
#include "DataTypes/DataTypeDefs.h"
#include "FtdcTranslator.h"
#include "Utils/Utils.h"


TimeDuration::TimeDuration(const std::string& product_id, __time64_t start_time, __time64_t end_time)
{
	this->product_id = product_id;
	this->start_time = start_time;
	this->end_time = end_time;
}

TimeDuration TimeDuration::MakeTimeDuration(const std::string& product_id, int week_day, int start_hour, int start_minute, int end_hour, int end_minute)
{
	SYSTEMTIME systime = { 0 };
	systime.wYear = 2001;
	systime.wMonth = 1;
	systime.wDay = week_day;
	systime.wHour = start_hour;
	systime.wMinute = start_minute;
	systime.wSecond = 0;

	__time64_t start_time = CalcTimestampMilli(systime);

	systime.wHour = end_hour;
	systime.wMinute = end_minute;
	systime.wSecond = 0;

	__time64_t end_time = CalcTimestampMilli(systime);

	return TimeDuration(product_id, start_time, end_time);
}

TradingAccount::TradingAccount()
{
	pre_mortgage = pre_credit = pre_deposit = pre_balance = pre_margin = interest = deposit =
		withdraw = frozen_margin = frozen_cash = frozen_commission = curr_margin = cash_in = commission =
		close_profit = position_profit = balance = available = withdraw_quota = reserve = credit =
		mortgage = exchange_margin = delivery_margin = exchange_delivery_margin = 0;
}

TradingAccount::TradingAccount(CThostFtdcTradingAccountField& field)
{
	///经纪公司代码
	broker_id = field.BrokerID;
	///投资者帐号
	account_id = field.AccountID;
	///上次质押金额
	pre_mortgage = field.PreMortgage;
	///上次信用额度
	pre_credit = field.PreCredit;
	///上次存款额
	pre_deposit = field.PreDeposit;
	///上次结算准备金
	pre_balance = field.PreBalance;
	///上次占用的保证金
	pre_margin = field.PreMargin;
	///利息收入
	interest = field.Interest;
	///入金金额
	deposit = field.Deposit;
	///出金金额
	withdraw = field.Withdraw;
	///冻结的保证金
	frozen_margin = field.FrozenMargin;
	///冻结的资金
	frozen_cash = field.FrozenCash;
	///冻结的手续费
	frozen_commission = field.FrozenCommission;
	///当前保证金总额
	curr_margin = field.CurrMargin;
	///资金差额
	cash_in = field.CashIn;
	///手续费
	commission = field.Commission;
	///平仓盈亏
	close_profit = field.CloseProfit;
	///持仓盈亏
	position_profit = field.PositionProfit;
	///期货结算准备金
	balance = field.Balance;
	///可用资金
	available = field.Available;
	///可取资金
	withdraw_quota = field.WithdrawQuota;
	///基本准备金
	reserve = field.Reserve;
	///交易日
	trading_day = field.TradingDay;
	///信用额度
	credit = field.Credit;
	///质押金额
	mortgage = field.Mortgage;
	///交易所保证金
	exchange_margin = field.ExchangeMargin;
	///投资者交割保证金
	delivery_margin = field.DeliveryMargin;
	///交易所交割保证金
	exchange_delivery_margin = field.ExchangeDeliveryMargin;
}

double TradingAccount::StaticBalance()
{
	return pre_balance - pre_credit - pre_mortgage + mortgage + deposit - withdraw;
}

double TradingAccount::DynamicBalance() const
{
	return balance - credit;
}

double TradingAccount::InitBalance() const
{
	return pre_balance - withdraw + deposit;
}

Quote::Quote(const CThostFtdcDepthMarketDataField& field)
{
	trading_day = field.TradingDay;
	if (trading_day.length() < 1) {
		trading_day = GetCurrentDate();
	}

	SetFromFtdcExchangeID(exchange_id, field.ExchangeID);
	instrument_id = field.InstrumentID;

	if (field.AskPrice1 >= 0 && field.AskPrice1 <= 1e+10) {
		ask_price1 = field.AskPrice1;
	}
	else {
		ask_price1 = 0;
	}

	if (field.BidPrice1 >= 0 && field.BidPrice1 <= 1e+10) {
		bid_price1 = field.BidPrice1;
	}
	else {
		bid_price1 = 0;
	}

	if (field.LastPrice >= 0 && field.LastPrice <= 1e+10) {
		last_price = field.LastPrice;
	}
	else {
		last_price = 0;
	}

	if (field.OpenPrice >= 0 && field.OpenPrice <= 1e+10) {
		open_price = field.OpenPrice;
	}
	else {
		open_price = 0;
	}

	if (field.ClosePrice >= 0 && field.ClosePrice <= 1e+10) {
		close_price = field.ClosePrice;
	}
	else {
		close_price = 0;
	}

	if (field.SettlementPrice >= 0 && field.SettlementPrice <= 1e+10) {
		settlement_price = field.SettlementPrice;
	}
	else {
		settlement_price = 0;
	}

	if (field.HighestPrice >= 0 && field.HighestPrice <= 1e+10) {
		highest_price = field.HighestPrice;
	}
	else {
		highest_price = 0;
	}

	if (field.LowestPrice >= 0 && field.LowestPrice <= 1e+10) {
		lowest_price = field.LowestPrice;
	}
	else {
		lowest_price = 0;
	}

	if (field.PreSettlementPrice >= 0 && field.PreSettlementPrice <= 1e+10) {
		pre_settlement_price = field.PreSettlementPrice;
	}
	else {
		pre_settlement_price = 0;
	}

	if (field.AveragePrice >= 0 && field.AveragePrice <= 1e+10) {
		average_price = field.AveragePrice;
	}
	else {
		average_price = 0;
	}

	if (field.UpperLimitPrice >= 0 && field.UpperLimitPrice <= 1e+10) {
		upper_limit_price = field.UpperLimitPrice;
	}
	else {
		upper_limit_price = 0;
	}

	if (field.LowerLimitPrice >= 0 && field.LowerLimitPrice <= 1e+10) {
		lower_limit_price = field.LowerLimitPrice;
	}
	else {
		lower_limit_price = 0;
	}

	if (ask_price1 == 0) {
		if (CompareDouble(field.LastPrice, field.UpperLimitPrice) == 0)
			ask_price1 = field.LastPrice;
		else if (CompareDouble(field.LastPrice, field.LowerLimitPrice) == 0)
			ask_price1 = field.LastPrice;
	}
	if (bid_price1 == 0) {
		if (CompareDouble(field.LastPrice, field.UpperLimitPrice) == 0)
			bid_price1 = field.LastPrice;
		else if (CompareDouble(field.LastPrice, field.LowerLimitPrice) == 0)
			bid_price1 = field.LastPrice;
	}

	trade_volume = field.Volume;
	ask_volume1 = field.AskVolume1;
	bid_volume1 = field.BidVolume1;
	position_volume = field.OpenInterest;

	std::string updatetime = field.UpdateTime;
	if (updatetime == "") 
		updatetime = "00:00:00";
	std::string strtime = trading_day.substr(0, 4) + "-" + trading_day.substr(4, 2) + "-" + trading_day.substr(6, 2)
		+ " " + updatetime;

	last_time = CalcTimestamp(strtime);
	if (field.UpdateMillisec) {
		last_time += field.UpdateMillisec;
	}
}

Quote::Quote(const std::string& instrument_id)
{
	this->instrument_id = instrument_id;
}

bool Quote::operator<(const Quote& quote) const
{
	return instrument_id < quote.instrument_id ||
		(instrument_id == quote.instrument_id && last_time < quote.last_time);
}

Quote& Quote::operator=(const Quote& quote)
{
	trading_day = quote.trading_day;
	exchange_id = quote.exchange_id;
	instrument_id = quote.instrument_id;
	ask_price1 = quote.ask_price1;
	bid_price1 = quote.bid_price1;
	last_price = quote.last_price;

	open_price = quote.open_price;
	close_price = quote.close_price;
	settlement_price = quote.settlement_price;
	highest_price = quote.highest_price;
	lowest_price = quote.lowest_price;

	pre_settlement_price = quote.pre_settlement_price;
	average_price = quote.average_price;

	trade_volume = quote.trade_volume;
	position_volume = quote.position_volume;
	ask_volume1 = quote.ask_volume1;
	bid_volume1 = quote.bid_volume1;

	upper_limit_price = quote.upper_limit_price;
	lower_limit_price = quote.lower_limit_price;

	last_time = quote.last_time;
	return *this;
}

Instrument::Instrument(const CThostFtdcInstrumentField& field)
{
	SetFromFtdcExchangeID(exchange_id, field.ExchangeID);
	instrument_id = field.InstrumentID;
	instrument_name = field.InstrumentName;
	price_tick = field.PriceTick;
	decs = 0;
	double tpt = price_tick;
	for (int i = 0; tpt > 0 && tpt < 1; ++i) {
		++decs;
		tpt *= 10;
	}
	volume_multiple = field.VolumeMultiple;
	product_id = field.ProductID;
	allow_market_order = exchange_id != SHFE;
}

Instrument::Instrument(const Instrument &ins)
{
	exchange_id = ins.exchange_id;
	instrument_id = ins.instrument_id;
	instrument_name = ins.instrument_name;
	price_tick = ins.price_tick;
	decs = ins.decs;
	volume_multiple = ins.volume_multiple;
	product_id = ins.product_id;
	allow_market_order = ins.allow_market_order;
}

Instrument::Instrument(const std::string& id) : instrument_id(id)
{

}

Instrument::Instrument(const ExchangeID exid, const std::string& id) : exchange_id(exid), instrument_id(id)
{

}

bool Instrument::operator<(const Instrument& Instrument) const
{
	return instrument_id < Instrument.instrument_id;
}

InstrumentMarginRate::InstrumentMarginRate()
{
	margin_ratio_by_money = margin_ratio_by_volume = 0;
}

bool InstrumentMarginRate::operator<(const InstrumentMarginRate& rate) const
{
	return instrument_id < rate.instrument_id || direction < rate.direction;
}

bool FrontSession::operator==(const FrontSession& ov) const
{
	return front_id == ov.front_id && session_id == ov.session_id;
}

bool FrontSession::operator<(const FrontSession& ov) const
{
	return front_id < ov.front_id || session_id < ov.session_id;
}

bool OrderKey::operator<(const OrderKey& ov) const
{
	return order_ref < ov.order_ref || FrontSession::operator<(ov);
}

Order::Order()
{
	direction = Buy;
	offset_flag = Open;
	request_id = 0;
	price = 0;
	volume = 0;
	volume_traded = 0;
	volume_remained = 0;
	broker_order_seq = 0;
}

Order::Order(CThostFtdcOrderField& field)
{
	key = OrderKey(field.FrontID, field.SessionID, atoi(field.OrderRef));
	instrument_id = field.InstrumentID;
	order_sys_id = field.OrderSysID;
	SetFromFtdcDirection(direction, field.Direction);
	SetFromFtdcOffsetflag(offset_flag, field.CombOffsetFlag[0]);
	price = field.LimitPrice;
	volume = field.VolumeTotalOriginal;
	volume_traded = field.VolumeTraded;
	volume_remained = field.VolumeTotal;
	broker_order_seq = field.BrokerOrderSeq;
	SetFromFtdcOrderStatus(status, field.OrderStatus);
	order_day = field.InsertDate;
	order_time = field.InsertTime;
	status_msg = field.StatusMsg;
	request_id = field.RequestID;
}

Order::Order(OrderKey& ref) : key(ref)
{

}

bool Order::operator<(const Order& order) const
{
	return key < order.key;
}

Trade::Trade(CThostFtdcTradeField& field)
{
	SetFromFtdcExchangeID(exchange_id, field.ExchangeID);
	instrument_id = field.InstrumentID;
	order_sys_id = field.OrderSysID;
	SetFromFtdcDirection(direction, field.Direction);
	SetFromFtdcOffsetflag(offset_flag, field.OffsetFlag);
	price = field.Price;
	volume = field.Volume;
	trade_id = field.TradeID;
	trade_date = field.TradeDate;
	trade_time = field.TradeTime;
}

bool Trade::operator<(const Trade& trade) const
{
	return trade_id < trade.trade_id;
}

Position::Position(const CThostFtdcInvestorPositionField& field)
{
	SetFromFtdcExchangeID(exchange_id, field.ExchangeID);
	instrument_id = field.InstrumentID;
	SetFromFtdcPosiDirection(direction, field.PosiDirection);
	yesterday_volume = field.Position - field.TodayPosition;
	today_volume = field.TodayPosition;
	position_cost = field.PositionCost;
	commission = field.Commission;
	profit = field.PositionProfit;
}

Position::Position(const std::string& instrument_id, Direction direction)
{
	this->instrument_id = instrument_id;
	this->direction = direction;
	yesterday_volume = 0;
	today_volume = 0;
	position_cost = 0;
	commission = 0;
	profit = 0;
}

Position::Position(const CThostFtdcInvestorPositionDetailField& field)
{
	SetFromFtdcExchangeID(exchange_id, field.ExchangeID);
	instrument_id = field.InstrumentID;
	SetFromFtdcPosiDirection(direction, field.Direction);
	yesterday_volume = 0;
	today_volume = field.Volume;
	position_cost = field.OpenPrice;
	commission = 0;
	profit = 0;
}

bool Position::operator<(const Position& position) const
{
	return instrument_id < position.instrument_id ||
		direction < position.direction;
}
