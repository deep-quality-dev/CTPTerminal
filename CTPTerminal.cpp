// CTPTerminal.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "QuoteService.h"
#include "Utils/Logger.h"
#include <set>
#include <sstream>
#include <iostream>

// #define ONLY_QUOTE

int main()
{
	CDataCenter data_center;
	CQuoteService quote_service;
	quote_service.set_data_center(&data_center);
	quote_service.set_gui_action(&quote_service);

	quote_service.Login();

	std::set<std::string> subscribe_products;
// 	// ������
// 	subscribe_products.insert("rb"); // ����
// 	subscribe_products.insert("ru"); // ��
// 	subscribe_products.insert("cu"); // ͭ
// 	subscribe_products.insert("au"); // �ƽ�
// 	subscribe_products.insert("ag"); // ����
// 	subscribe_products.insert("fu"); // ȼ��
// 	subscribe_products.insert("MA"); // �״�
// 	subscribe_products.insert("bu"); // ����
// 	subscribe_products.insert("hc"); // �Ⱦ�
// 	// ������
// 	subscribe_products.insert("m"); // ����
// 	subscribe_products.insert("i"); // ����
// 	subscribe_products.insert("p"); // ���
// 	subscribe_products.insert("y"); // ����
// 	subscribe_products.insert("c"); // ����
// 	subscribe_products.insert("j"); // ��̿
// 	subscribe_products.insert("jm"); // ��ú

	subscribe_products.insert("au"); // �ƽ�

	quote_service.SetSubscribeProducts(subscribe_products);

	while (true) {
		Utils::Log("1: ReqQryTradingAccount");
		Utils::Log("2: ReqQryOrder");
		Utils::Log("3: ReqQryTrade");
		Utils::Log("4: ReqQryPosition");
		Utils::Log("5: ReqQryMarginRate");

		int choose;
		std::cin >> choose;
		switch (choose) {
		case 1:
			quote_service.trade_api()->ReqQryTradingAccount();
			break;

		case 2:
			quote_service.trade_api()->ReqQryOrder();
			break;

		case 3:
			quote_service.trade_api()->ReqQryTrade();
			break;

		case 4:
			quote_service.trade_api()->ReqQryPosition();
			break;

		case 5:
			quote_service.trade_api()->ReqQryMarginRate("au2102");
			break;

		case 0:
			break;
		}
	}

	char c;
	std::cin >> c;

	quote_service.Logout();

    return 0;
}

