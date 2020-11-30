#include "stdafx.h"
#include "QuoteServerHandler.h"
#include "QuoteService.h"


CQuoteServerHandler::CQuoteServerHandler(CQuoteService* quote_service) : quote_service_(quote_service)
{
}


CQuoteServerHandler::~CQuoteServerHandler()
{
}

void CQuoteServerHandler::OnLoginProcess(ApiEvent api_event, const char* content /*= NULL*/, int error_id /*= 0*/)
{

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
