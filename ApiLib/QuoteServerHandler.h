#pragma once

#include "DataTypes/GuiDataAction.h"

class CQuoteService;

class CQuoteServerHandler : public IGuiDataAction
{
public:
	CQuoteServerHandler(CQuoteService* quote_service);
	~CQuoteServerHandler();

	virtual void OnLoginProcess(ApiEvent api_event, const char* content = NULL, int error_id = 0);

	virtual void RefreshAccount(const TradingAccount& account);

	virtual void RefreshQuotes(const std::set<Quote>& quotes);

	virtual void RefreshInstruments(const std::set<Instrument>& instruments);

private:
	CQuoteService* quote_service_;
};

