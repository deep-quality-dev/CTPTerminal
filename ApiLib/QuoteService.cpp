#include "stdafx.h"
#include "QuoteService.h"
#include "QuoteServerHandler.h"
#include "ThostMdApiWrapper.h"
#include "ThostTradeApiWrapper.h"
#include "Utils/IniManager.h"
#include "Utils/Utils.h"
#include <sstream>

CQuoteService::CQuoteService() : 
	inited_trader_(false), inited_md_(false),
	data_center_(new CDataCenter()),
	gui_action_(new CQuoteServerHandler(this))
{
	
}


CQuoteService::~CQuoteService()
{
}

void CQuoteService::LoadServerConfig()
{
	CIniManager ini_manager;
	ini_manager.init_content();

	ini_manager.load(GetRelativePath("config.ini").c_str());
	std::string broker_id = ini_manager.get_value("Trade", "broker_id");
	std::string user_id = ini_manager.get_value("Trade", "user_id");
	std::string password = ini_manager.get_value("Trade", "password");
	std::string front_addrs = ini_manager.get_value("Trade", "fronts");

	ServerConfig trade_server_config;
	trade_server_config.broker_id = broker_id;
	trade_server_config.user_id = user_id;
	trade_server_config.password = password;
	
	std::stringstream ss; ss << front_addrs;
	while (ss.good()) {
		std::string front_addr;
		getline(ss, front_addr, ',');
		trade_server_config.addresses.push_back(front_addr);
	}

	broker_id = ini_manager.get_value("MarketData", "broker_id");
	user_id = ini_manager.get_value("MarketData", "user_id");
	password = ini_manager.get_value("MarketData", "password");
	front_addrs = ini_manager.get_value("MarketData", "fronts");

	ServerConfig md_server_config;
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
}

void CQuoteService::Initialize()
{
	InitTradeWrappers();
	InitMdWrappers();
}

void CQuoteService::SetSubscribeProducts(const std::set<std::string>& products)
{
	products_ = products;

	CheckSubscribeProducts();
}

void CQuoteService::SetInstruments(const std::set<Instrument>& instruments)
{
	instruments_ = instruments;

	CheckSubscribeProducts();
}

void CQuoteService::InitMdWrappers()
{
	if (inited_md_)
		return;

	for (auto it_config = quote_server_configs_.begin();
		it_config != quote_server_configs_.end();
		it_config++) {
		std::shared_ptr<IMarketDataApi> md_api(new CThostMdApiWrapper(data_center_.get(), gui_action_.get()));
		md_api->Initialize(it_config->broker_id, it_config->user_id, it_config->password, it_config->addresses);
		md_apis_.insert(md_api);
	}

	inited_md_ = true;
}

void CQuoteService::InitTradeWrappers()
{
	if (inited_trader_)
		return;

	trade_api_ = std::shared_ptr<CThostTradeApiWrapper>(new CThostTradeApiWrapper(data_center_.get(), gui_action_.get()));
	trade_api_->Initialize(trade_server_config_.broker_id,
		trade_server_config_.user_id,
		trade_server_config_.password,
		trade_server_config_.addresses);

	inited_trader_ = true;
}

void CQuoteService::CheckSubscribeProducts()
{
	for (auto it_md = md_apis_.begin(); it_md != md_apis_.end(); it_md++) {
		std::set<std::string> need_subscribes;
		for (auto it_inst = instruments_.begin(); it_inst != instruments_.end(); it_inst++) {
			auto it_product = products_.find(it_inst->product_id);
			if (it_product != products_.end()) {
				need_subscribes.insert(it_inst->instrument_id);
			}
		}
		if (need_subscribes.size() > 0) {
			(*it_md)->ReqSubscribeQuote(need_subscribes);
		}
	}
}
