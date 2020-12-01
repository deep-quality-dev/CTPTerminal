// CTPTerminal.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "QuoteService.h"
#include <set>
#include <iostream>

int main()
{
	CQuoteService quote_service;
	quote_service.LoadServerConfig();

	// 在行情终端，只能手动设置合约列表
	std::set<Instrument> instruments;
	instruments.insert(Instrument(ExchangeID::SHFE, "au2012"));
	instruments.insert(Instrument(ExchangeID::SHFE, "au2101"));
	instruments.insert(Instrument(ExchangeID::SHFE, "au2102"));
	instruments.insert(Instrument(ExchangeID::SHFE, "ag2012"));
	instruments.insert(Instrument(ExchangeID::SHFE, "ag2101"));
	instruments.insert(Instrument(ExchangeID::SHFE, "ag2102"));

	quote_service.SetInstruments(instruments);
	quote_service.data_center()->OnRtnInstruments(instruments);
	quote_service.gui_action()->RefreshInstruments(instruments);

	std::set<std::string> subscribe_products;
	subscribe_products.insert("rb");
	subscribe_products.insert("ru");
	subscribe_products.insert("au");
	subscribe_products.insert("ag");
	quote_service.SetSubscribeProducts(subscribe_products);

	std::set<std::string> subscribe_instruments;
	for (auto it_inst = instruments.begin(); it_inst != instruments.end(); it_inst++) {
		subscribe_instruments.insert(it_inst->instrument_id);
	}
	quote_service.SetSubscribeInstruments(subscribe_instruments);

	quote_service.Initialize();

	char c;
	std::cin >> c;

    return 0;
}

