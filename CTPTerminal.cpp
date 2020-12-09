// CTPTerminal.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "QuoteService.h"
#include <set>
#include <iostream>

// #define ONLY_QUOTE

int main()
{
	CQuoteService quote_service;
	quote_service.LoadServerConfig();

#ifdef ONLY_QUOTE
	// �������նˣ�ֻ���ֶ����ú�Լ�б�
	std::set<Instrument> instruments;
	std::set<Instrument> instruments;
	Instrument au2012(ExchangeID::SHFE, "au2012"); au2012.product_id = "au"; instruments.insert(au2012);
	Instrument au2101(ExchangeID::SHFE, "au2101"); au2101.product_id = "au"; instruments.insert(au2101);
	Instrument au2102(ExchangeID::SHFE, "au2102"); au2102.product_id = "au"; instruments.insert(au2102);
	Instrument au2103(ExchangeID::SHFE, "au2103"); au2103.product_id = "au"; instruments.insert(au2103);
	Instrument au2104(ExchangeID::SHFE, "au2104"); au2104.product_id = "au"; instruments.insert(au2104);
	Instrument au2105(ExchangeID::SHFE, "au2105"); au2105.product_id = "au"; instruments.insert(au2105);
	Instrument au2106(ExchangeID::SHFE, "au2106"); au2106.product_id = "au"; instruments.insert(au2106);
	Instrument au2107(ExchangeID::SHFE, "au2107"); au2107.product_id = "au"; instruments.insert(au2107);
	Instrument au2108(ExchangeID::SHFE, "au2108"); au2108.product_id = "au"; instruments.insert(au2108);
	Instrument au2109(ExchangeID::SHFE, "au2109"); au2109.product_id = "au"; instruments.insert(au2109);
	Instrument au2110(ExchangeID::SHFE, "au2110"); au2110.product_id = "au"; instruments.insert(au2110);
	Instrument au2111(ExchangeID::SHFE, "au2111"); au2111.product_id = "au"; instruments.insert(au2111);
	Instrument au2112(ExchangeID::SHFE, "au2112"); au2112.product_id = "au"; instruments.insert(au2112);

	Instrument ag2012(ExchangeID::SHFE, "ag2012"); ag2012.product_id = "au"; instruments.insert(ag2012);
	Instrument ag2101(ExchangeID::SHFE, "ag2101"); ag2101.product_id = "au"; instruments.insert(ag2101);
	Instrument ag2102(ExchangeID::SHFE, "ag2102"); ag2102.product_id = "au"; instruments.insert(ag2102);
	Instrument ag2103(ExchangeID::SHFE, "ag2103"); ag2103.product_id = "au"; instruments.insert(ag2103);
	Instrument ag2104(ExchangeID::SHFE, "ag2104"); ag2104.product_id = "au"; instruments.insert(ag2104);
	Instrument ag2105(ExchangeID::SHFE, "ag2105"); ag2105.product_id = "au"; instruments.insert(ag2105);
	Instrument ag2106(ExchangeID::SHFE, "ag2106"); ag2106.product_id = "au"; instruments.insert(ag2106);
	Instrument ag2107(ExchangeID::SHFE, "ag2107"); ag2107.product_id = "au"; instruments.insert(ag2107);
	Instrument ag2108(ExchangeID::SHFE, "ag2108"); ag2108.product_id = "au"; instruments.insert(ag2108);
	Instrument ag2109(ExchangeID::SHFE, "ag2109"); ag2109.product_id = "au"; instruments.insert(ag2109);
	Instrument ag2110(ExchangeID::SHFE, "ag2110"); ag2110.product_id = "au"; instruments.insert(ag2110);
	Instrument ag2111(ExchangeID::SHFE, "ag2111"); ag2111.product_id = "au"; instruments.insert(ag2111);
	Instrument ag2112(ExchangeID::SHFE, "ag2112"); ag2112.product_id = "au"; instruments.insert(ag2112);

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
	// �Ͻ���
	subscribe_products.insert("rb"); // ����
	subscribe_products.insert("ru"); // ��
	subscribe_products.insert("cu"); // ͭ
	subscribe_products.insert("au"); // �ƽ�
	subscribe_products.insert("ag"); // ����
	// ������
	subscribe_products.insert("m"); // ����
	subscribe_products.insert("i"); // ����
	quote_service.SetSubscribeProducts(subscribe_products);

	quote_service.Initialize();

	char c;
	std::cin >> c;

    return 0;
}

