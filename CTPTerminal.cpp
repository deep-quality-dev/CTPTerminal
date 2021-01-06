// CTPTerminal.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ConfigParser.h"
#include "QuoteService.h"
#include "ArbitrageStrategy.h"
#include "StepStrategy.h"
#include "Utils/Logger.h"
#include <set>
#include <sstream>
#include <iostream>

// #define ONLY_QUOTE

int main()
{
	CConfigParser::LoadServerConfig("config.ini");

	CDataCenter data_center;
	CQuoteService quote_service;
	quote_service.set_data_center(&data_center);
	quote_service.set_gui_action(&quote_service);
	data_center.set_gui_action(&quote_service);

	quote_service.Login();

	std::set<std::string> subscribe_products;

#ifdef ONLY_QUOTE
	// 上期所
	subscribe_products.insert("rb"); // 螺纹
	subscribe_products.insert("ru"); // 橡胶
	subscribe_products.insert("cu"); // 铜
	subscribe_products.insert("au"); // 黄金
	subscribe_products.insert("ag"); // 白银
	subscribe_products.insert("fu"); // 燃油
	subscribe_products.insert("MA"); // 甲醇
	subscribe_products.insert("bu"); // 沥青
	subscribe_products.insert("hc"); // 热卷
	subscribe_products.insert("ni"); // 沪镍
	// 大商所
	subscribe_products.insert("m"); // 豆粕
	subscribe_products.insert("i"); // 铁矿
	subscribe_products.insert("p"); // 棕榈
	subscribe_products.insert("y"); // 豆油
	subscribe_products.insert("c"); // 玉米
	subscribe_products.insert("j"); // 焦炭
	subscribe_products.insert("jm"); // 焦煤
	subscribe_products.insert("v"); // PVC

#else
	CStepStrategy strategy(&data_center, quote_service.trade_api());
	// CArbitrageStrategy strategy(&data_center, quote_service.trade_api());

	strategy.set_main_instrument_id(CConfigParser::main_instrument_id());
	strategy.set_sub_instrument_id(CConfigParser::sub_instrument_id());
	strategy.set_ma_period(CConfigParser::ma_period());
	strategy.set_volume(CConfigParser::volume());

	data_center.SetQuoteCallback(std::bind(&CStepStrategy::OnQuoteCallback, &strategy, std::placeholders::_1));
	data_center.SetOnOrderCallback(std::bind(&CStepStrategy::OnOrderCallback, &strategy, std::placeholders::_1));
	data_center.SetOnTradeCallback(std::bind(&CStepStrategy::OnTradeCallback, &strategy, std::placeholders::_1, std::placeholders::_2));
	data_center.SetOnTradeAccountCallback(std::bind(&CStepStrategy::OnTradeAccountCallback, &strategy, std::placeholders::_1));

	strategy.Initialize();

	subscribe_products.insert("au"); // 黄金
	quote_service.SetSubscribeProducts(subscribe_products);

	while (true) {
		Utils::Log("1: ReqQryTradingAccount");
		Utils::Log("2: ReqQryOrder");
		Utils::Log("3: ReqQryTrade");
		Utils::Log("4: ReqQryPosition");
		Utils::Log("5: ReqQryMarginRate");
		Utils::Log("6: StartStopTrade");
		Utils::Log("7: ReqInsertMarketOrder(Open/Buy/1)");
		Utils::Log("8: ReqInsertMarketOrder(Close/Sell/1)");

		int choose;
		std::cin >> choose;
		switch (choose) {
		case 1:
			quote_service.trade_api()->ReqQryTradingAccount();
			break;

		case 2:
			//quote_service.trade_api()->ReqQryOrder();
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

		case 6:
			strategy.set_enable_trade(!strategy.is_enable_trade());
			if (strategy.is_enable_trade()) {
				Utils::Log("已开始运行策略");
			}
			else {
				Utils::Log("已停止运行策略");
			}
			
			break;

		case 7:
			strategy.InsertMarketOrder(CConfigParser::main_instrument_id(), OffsetFlag::Open, Direction::Buy, 1);
			break;

		case 8:
			strategy.InsertMarketOrder(CConfigParser::main_instrument_id(), OffsetFlag::Close, Direction::Sell, 1);
			break;

		case 0:
			break;
		}
	}

#endif

	char c;
	std::cin >> c;

	quote_service.Logout();

    return 0;
}

