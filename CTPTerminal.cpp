// CTPTerminal.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "QuoteService.h"
#include <set>
#include <iostream>

// #define ONLY_QUOTE

#include "QryManager.h"
#include <math.h>

int g_request_id = 0;
int testRequestId() {
	static int request_id = rand();
	return g_request_id = ++request_id;
}

void testQueryManager()
{
	CQryManager qry_manager;
	qry_manager.CreateThread();
	qry_manager.AddQuery(testRequestId, "testRequestId");
	while (!g_request_id) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	qry_manager.CheckQuery(g_request_id, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(100000));
}

int main()
{
// 	testQueryManager();
// 	return 0;

	CQuoteService quote_service;
	quote_service.LoadServerConfig();

#ifdef ONLY_QUOTE
	// 在行情终端，只能手动设置合约列表
	std::set<Instrument> instruments;
	Instrument au2012(ExchangeID::SHFE, "au2012"); au2012.product_id = "au"; instruments.insert(au2012);
	Instrument au2101(ExchangeID::SHFE, "au2101"); au2101.product_id = "au"; instruments.insert(au2101);
	Instrument ag2012(ExchangeID::SHFE, "ag2012"); ag2012.product_id = "au"; instruments.insert(ag2012);
	Instrument ag2102(ExchangeID::SHFE, "ag2102"); ag2102.product_id = "au"; instruments.insert(ag2102);

	quote_service.SetInstruments(instruments);
	quote_service.data_center()->OnRtnInstruments(instruments);
	quote_service.gui_action()->RefreshInstruments(instruments);

	std::set<std::string> subscribe_instruments;
	for (auto it_inst = instruments.begin(); it_inst != instruments.end(); it_inst++) {
		subscribe_instruments.insert(it_inst->instrument_id);
	}
	quote_service.SetSubscribeInstruments(subscribe_instruments);
#endif

	std::set<std::string> subscribe_products;
	subscribe_products.insert("rb");
	subscribe_products.insert("ru");
	subscribe_products.insert("au");
	subscribe_products.insert("ag");
	quote_service.SetSubscribeProducts(subscribe_products);

	quote_service.Initialize();

	char c;
	std::cin >> c;

    return 0;
}

