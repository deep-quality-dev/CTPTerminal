#include "stdafx.h"
#include "QuoteServerHandler.h"
#include "QuoteService.h"
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

void CQuoteServerHandler::RefreshOrders(const std::set<Order>& order)
{

}

void CQuoteServerHandler::RefreshTrades(const std::set<Trade>& trade)
{

}

void CQuoteServerHandler::RefreshOrder(const Order& order)
{

}

void CQuoteServerHandler::RefreshTrade(const Trade& trade)
{

}
