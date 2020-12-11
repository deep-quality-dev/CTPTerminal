#include "stdafx.h"
#include "QuoteServerHandler.h"
#include "QuoteService.h"
#include "DataTypes/Formatter.h"
#include "Utils/Logger.h"
#include <sstream>


CQuoteServerHandler::CQuoteServerHandler(CQuoteService* quote_service) : quote_service_(quote_service)
{
}


CQuoteServerHandler::~CQuoteServerHandler()
{
}

void CQuoteServerHandler::OnLoginProcess(ApiEvent api_event, const char* content /*= NULL*/, int error_id /*= 0*/, const char* error_msg /*= NULL*/)
{
	std::stringstream ss;
	ss << api_event
		<< ", " << (content == NULL ? "" : content) << ", "
		<< error_id << ", "
		<< (error_msg == NULL ? "," : error_msg);
	Utils::Log(ss.str());
}

void CQuoteServerHandler::RefreshAccount(const TradingAccount& account)
{
	std::stringstream ss;
	ss << "经纪公司代码: " << account.broker_id << std::endl
		<< "投资者帐号: " << account.account_id << std::endl
		<< "上次质押金额: " << account.pre_mortgage << std::endl
		<< "上次信用额度: " << account.pre_credit << std::endl
		<< "上次存款额: " << account.pre_deposit << std::endl
		<< "上次结算准备金: " << account.pre_balance << std::endl
		<< "上次占用的保证金: " << account.pre_margin << std::endl
		<< "利息收入: " << account.interest << std::endl
		<< "入金金额: " << account.deposit << std::endl
		<< "出金金额: " << account.withdraw << std::endl
		<< "冻结的保证金: " << account.frozen_margin << std::endl
		<< "投资者帐号冻结的资金 " << account.frozen_cash << std::endl
		<< "冻结的手续费: " << account.frozen_commission << std::endl
		<< "当前保证金总额: " << account.curr_margin << std::endl
		<< "资金差额: " << account.cash_in << std::endl
		<< "手续费: " << account.commission << std::endl
		<< "平仓盈亏: " << account.close_profit << std::endl
		<< "持仓盈亏: " << account.position_profit << std::endl
		<< "期货结算准备金: " << account.balance << std::endl
		<< "可用资金: " << account.available << std::endl
		<< "可取资金: " << account.withdraw_quota << std::endl
		<< "基本准备金: " << account.reserve << std::endl
		<< "交易日: " << account.trading_day << std::endl
		<< "信用额度: " << account.credit << std::endl
		<< "质押金额: " << account.mortgage << std::endl
		<< "交易所保证金: " << account.exchange_margin << std::endl
		<< "投资者交割保证金: " << account.delivery_margin << std::endl
		<< "交易所交割保证金: " << account.exchange_delivery_margin;
	Utils::Log(ss.str());
}

void CQuoteServerHandler::RefreshQuotes(const std::set<Quote>& quotes)
{

}

void CQuoteServerHandler::RefreshInstruments(const std::set<Instrument>& instruments)
{
	quote_service_->SetInstruments(instruments);
}

void CQuoteServerHandler::RefreshPositions(const std::set<Position>& positions)
{

}

void CQuoteServerHandler::RefreshOrders(const std::set<Order>& orders)
{
	Utils::Log("查询报单 >>");
	for (auto it_order = orders.begin(); it_order != orders.end(); it_order++) {
		std::stringstream ss;
		ss << "===============" << std::endl
			<< "报单编号, " << it_order->order_sys_id << std::endl
			<< "合约名称, " << it_order->instrument_id << std::endl
			<< "买卖方向, " << CFormatter::GetInstance().Direction2string(it_order->direction) << std::endl
			<< "组合开平标志, " << CFormatter::GetInstance().OffsetFlag2string(it_order->offset_flag) << std::endl
			<< "请求编号, " << it_order->request_id << std::endl
			<< "价格, " << it_order->price << std::endl
			<< "数量, " << it_order->volume << std::endl
			<< "今成交数量, " << it_order->volume_traded << std::endl
			<< "剩余数量, " << it_order->volume_remained << std::endl
			<< "报单状态, " << CFormatter::GetInstance().Status2String(it_order->status) << std::endl
			<< "报单日期, " << it_order->order_day << std::endl
			<< "委托时间, " << it_order->order_time << std::endl
			<< "状态信息, " << it_order->status_msg << std::endl;

		Utils::Log(ss.str());
	}
	Utils::Log("查询报单 <<");
}

void CQuoteServerHandler::RefreshTrades(const std::set<Trade>& trades)
{
	Utils::Log("查询成交 >>");
	for (auto it_trade = trades.begin(); it_trade != trades.end(); it_trade++) {
		std::stringstream ss;
		ss << "===============" << std::endl
			<< "交易所代码, " << CFormatter::GetInstance().ExchangeID2string(it_trade->exchange_id) << std::endl
			<< "合约名称, " << it_trade->instrument_id << std::endl
			<< "买卖方向, " << CFormatter::GetInstance().Direction2string(it_trade->direction) << std::endl
			<< "组合开平标志, " << CFormatter::GetInstance().OffsetFlag2string(it_trade->offset_flag) << std::endl
			<< "报单编号, " << it_trade->order_sys_id << std::endl
			<< "价格, " << it_trade->price << std::endl
			<< "数量, " << it_trade->volume << std::endl
			<< "成交编号, " << it_trade->trade_id << std::endl
			<< "成交时期, " << it_trade->trade_date << std::endl
			<< "成交时间, " << it_trade->trade_time << std::endl;

		Utils::Log(ss.str());
	}
	Utils::Log("查询成交 <<");
}

void CQuoteServerHandler::RefreshOrder(const Order& order)
{
	Utils::Log("报单通知 >>");

	std::stringstream ss;
	ss << "报单编号, " << order.order_sys_id << std::endl
		<< "合约名称, " << order.instrument_id << std::endl
		<< "买卖方向, " << CFormatter::GetInstance().Direction2string(order.direction) << std::endl
		<< "组合开平标志, " << CFormatter::GetInstance().OffsetFlag2string(order.offset_flag) << std::endl
		<< "请求编号, " << order.request_id << std::endl
		<< "价格, " << order.price << std::endl
		<< "数量, " << order.volume << std::endl
		<< "今成交数量, " << order.volume_traded << std::endl
		<< "剩余数量, " << order.volume_remained << std::endl
		<< "报单状态, " << CFormatter::GetInstance().Status2String(order.status) << std::endl
		<< "报单日期, " << order.order_day << std::endl
		<< "委托时间, " << order.order_time << std::endl
		<< "状态信息, " << order.status_msg << std::endl;
	Utils::Log(ss.str());

	Utils::Log("报单通知 <<");
}

void CQuoteServerHandler::RefreshTrade(const Trade& trade)
{
	Utils::Log("成交通知 >>");

	std::stringstream ss;
	ss << "交易所代码, " << CFormatter::GetInstance().ExchangeID2string(trade.exchange_id) << std::endl
		<< "合约名称, " << trade.instrument_id << std::endl
		<< "买卖方向, " << CFormatter::GetInstance().Direction2string(trade.direction) << std::endl
		<< "组合开平标志, " << CFormatter::GetInstance().OffsetFlag2string(trade.offset_flag) << std::endl
		<< "报单编号, " << trade.order_sys_id << std::endl
		<< "价格, " << trade.price << std::endl
		<< "数量, " << trade.volume << std::endl
		<< "成交编号, " << trade.trade_id << std::endl
		<< "成交时期, " << trade.trade_date << std::endl
		<< "成交时间, " << trade.trade_time << std::endl;
	Utils::Log(ss.str());

	Utils::Log("成交通知 <<");
}
