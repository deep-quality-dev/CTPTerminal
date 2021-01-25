#include "stdafx.h"
#include "ConfigParser.h"
#include "Utils/IniManager.h"
#include "Utils/Utils.h"
#include <sstream>

std::vector<MarketDataServerConfig> CConfigParser::quote_server_configs_;
TradeServerConfig CConfigParser::trade_server_config_;

std::string CConfigParser::main_instrument_id_;
std::string CConfigParser::sub_instrument_id_;
int CConfigParser::ma_period_ = 0;
int CConfigParser::volume_ = 1;
int CConfigParser::order_limit_ = 1;
double CConfigParser::loss_limit_ = 5;
int CConfigParser::order_interval_ = 5;
int CConfigParser::price_offset_ = 0;

void CConfigParser::LoadServerConfig(const char* config_path)
{
	CIniManager ini_manager;
	ini_manager.init_content();

	ini_manager.load(Utils::GetRelativePath(config_path).c_str());

	// Configuration of CTP servers
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

	// Configuration for Trading
	main_instrument_id_ = ini_manager.get_value("Param", "main_instrument_id");
	sub_instrument_id_ = ini_manager.get_value("Param", "sub_instrument_id");

	std::string ma_period = ini_manager.get_value("Param", "ma_period");
	std::string volume = ini_manager.get_value("Param", "volume");

	ma_period_ = std::atoi(ma_period.c_str());
	volume_ = std::atoi(volume.c_str());

	// Configuration for 风控
	std::string order_limit = ini_manager.get_value("Param", "order_limit");
	order_limit_ = std::atoi(order_limit.c_str());

	std::string loss_limit = ini_manager.get_value("Param", "loss_limit");
	loss_limit_ = std::atof(loss_limit.c_str());

	std::string order_interval = ini_manager.get_value("Param", "order_interval");
	order_interval_ = std::atoi(order_interval.c_str());

	std::string price_offset = ini_manager.get_value("Strategy", "price_offset");
	price_offset_ = std::atoi(price_offset.c_str());
}
