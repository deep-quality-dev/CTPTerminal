﻿#include "stdafx.h"
#include "ConfigParser.h"
#include "Utils/IniManager.h"
#include "Utils/Utils.h"
#include <sstream>

std::vector<MarketDataServerConfig> CConfigParser::quote_server_configs_;
TradeServerConfig CConfigParser::trade_server_config_;

void CConfigParser::LoadServerConfig(const char* config_path)
{
	CIniManager ini_manager;
	ini_manager.init_content();

	ini_manager.load(Utils::GetRelativePath(config_path).c_str());
	std::string broker_id = ini_manager.get_value("Trade", "broker_id");
	std::string user_id = ini_manager.get_value("Trade", "user_id");
	std::string password = ini_manager.get_value("Trade", "password");
	std::string front_addrs = ini_manager.get_value("Trade", "fronts");
	std::string user_product_info = ini_manager.get_value("Trade", "user_product_info");
	std::string auth_code = ini_manager.get_value("Trade", "auth_code");
	std::string app_id = ini_manager.get_value("Trade", "app_id");

	TradeServerConfig trade_server_config;
	trade_server_config.broker_id = broker_id;
	trade_server_config.user_id = user_id;
	trade_server_config.password = password;
	trade_server_config.user_product_info = user_product_info;
	trade_server_config.auth_code = auth_code;
	trade_server_config.app_id = app_id;

	std::stringstream ss;
	ss << front_addrs;
	while (ss.good()) {
		std::string front_addr;
		getline(ss, front_addr, ',');
		trade_server_config.addresses.push_back(front_addr);
	}

	broker_id = ini_manager.get_value("MarketData", "broker_id");
	user_id = ini_manager.get_value("MarketData", "user_id");
	password = ini_manager.get_value("MarketData", "password");
	front_addrs = ini_manager.get_value("MarketData", "fronts");

	MarketDataServerConfig md_server_config;
	md_server_config.broker_id = broker_id;
	md_server_config.user_id = user_id;
	md_server_config.password = password;

	ss.clear(); ss << front_addrs;
	while (ss.good()) {
		std::string front_addr;
		getline(ss, front_addr, ',');
		md_server_config.addresses.push_back(front_addr);
	}

	quote_server_configs_.clear();
	quote_server_configs_.push_back(md_server_config);

	trade_server_config_ = trade_server_config;
}