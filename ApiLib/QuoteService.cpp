#include "stdafx.h"
#include "QuoteService.h"
#include "QuoteServerHandler.h"
#include "ThostMdApiWrapper.h"
#include "ThostTradeApiWrapper.h"

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
	// TEST CODE
	ServerConfig server_config;
	server_config.broker_id = "3030";
	server_config.addresses.push_back("tcp://192.168.0.100:5050");
	server_config.user_id = "9999";
	server_config.password = "0000";
	quote_server_configs_.push_back(server_config);
	trade_server_config_ = server_config;
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
