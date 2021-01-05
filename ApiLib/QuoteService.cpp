#include "stdafx.h"
#include "QuoteService.h"
#include "QuoteServerHandler.h"
#include "ThostMdApiWrapper.h"
#include "ThostTradeApiWrapper.h"
#include "ConfigParser.h"
#include "Utils/Utils.h"
#include "Utils/Logger.h"
#include <sstream>

CQuoteService::CQuoteService() :
	data_center_(NULL),
	gui_action_(NULL),
	trade_api_(NULL),
	logined_(false), md_logined_(0), trade_logined_(0), instrument_inited_(false), position_list_inited_(false),
	trader_inited_(false), md_inited_(false)
{
	login_handle = ::CreateEvent(NULL, FALSE, FALSE, _T("CTP_LOGIN_EVENT"));
	account_handle = ::CreateEvent(NULL, FALSE, FALSE, _T("CTP_ACCOUNT_EVENT"));
	insert_order_handle = ::CreateEvent(NULL, FALSE, FALSE, _T("CTP_INSERT_ORDER_EVENT"));
	position_list_handle = ::CreateEvent(NULL, FALSE, FALSE, _T("CTP_POSITIONLIST_EVENT"));
}


CQuoteService::~CQuoteService()
{
	Utils::CloseHandleSafely(login_handle);
	Utils::CloseHandleSafely(account_handle);
	Utils::CloseHandleSafely(insert_order_handle);
	Utils::CloseHandleSafely(position_list_handle);

	Deinitialize();
}

void CQuoteService::Initialize(const std::vector<MarketDataServerConfig>& quote_configs,
	const TradeServerConfig& trade_config)
{
	quote_server_configs_ = quote_configs;
	trade_server_config_ = trade_config;

	InitTradeWrappers();
	InitMdWrappers();
}

void CQuoteService::Deinitialize()
{
	if (trade_api_) {
		trade_api_->Deinitialize();
	}
	for (auto it_md = md_apis_.begin(); it_md != md_apis_.end(); it_md++) {
		IMarketDataApi* md_api = *it_md;
		if (md_api) {
			md_api->Deinitialize();
		}
	}

// 	if (trade_api_) {
// 		delete trade_api_;
// 	}
// 	for (auto it_md = md_apis_.begin(); it_md != md_apis_.end(); it_md++) {
// 		IMarketDataApi* md_api = *it_md;
// 		delete md_api;
// 	}
	trade_api_ = NULL;
	md_apis_.clear();
}

int CQuoteService::Login()
{
	if (logined_) {
		return 0;
	}

	logined_ = false;
	::ResetEvent(login_handle);

	CConfigParser::LoadServerConfig("config.ini");
	std::vector<MarketDataServerConfig> quote_server_configs =
		CConfigParser::market_data_server_config();
	TradeServerConfig trade_server_config =
		CConfigParser::trade_server_config();

	Initialize(quote_server_configs, trade_server_config);

	md_logined_ = 0;
	trade_logined_ = 0;
	instrument_inited_ = false;
	position_list_inited_ = false;

	for (auto it_md = md_apis_.begin(); it_md != md_apis_.end(); it_md++) {
		(*it_md)->Login();
	}
	trade_api_->Login();

	if (WaitForSingleObject(login_handle, MaxLoginTimeout) != WAIT_OBJECT_0) {
		return -1;
	}

	if (md_logined_ > 0 && trade_logined_ > 0 && instrument_inited_ && position_list_inited_) {
		logined_ = true;
		return 0;
	}

	return -1;
}

int CQuoteService::Logout()
{
	for (auto it_md = md_apis_.begin(); it_md != md_apis_.end(); it_md++) {
		(*it_md)->Logout();
	}
	if (trade_api_)
		trade_api_->Logout();

	Deinitialize();

	logined_ = false;

	return 0;
}

void CQuoteService::SetInstruments(const std::set<Instrument>& instruments)
{
	instruments_ = instruments;

	std::stringstream ss;
	ss << "查询合约结束：总共 " << instruments.size();
	Utils::Log(ss.str());

	CheckSubscribes();
}

void CQuoteService::SetSubscribeProducts(const std::set<std::string>& products)
{
	subscribe_products_ = products;

	CheckSubscribes();
}

void CQuoteService::SetSubscribeInstruments(const std::set<std::string>& instrument_ids)
{
	subscribe_instrument_ids_ = instrument_ids;

	CheckSubscribes();
}

void CQuoteService::ReqQryTradingAccount()
{
	int ret = 0;
	while ((ret = trade_api()->ReqQryTradingAccount()) < 0) {
		Sleep(3000);
	}
}

void CQuoteService::ReqQryPositionDetail()
{
	int ret = 0;
	while ((ret = trade_api()->ReqQryPositionDetail()) < 0) {
		Sleep(3000);
	}
}

void CQuoteService::InitMdWrappers()
{
	if (md_inited_)
		return;

	for (auto it_config = quote_server_configs_.begin();
		it_config != quote_server_configs_.end();
		it_config++) {
		IMarketDataApi* md_api = new CThostMdApiWrapper(data_center_, gui_action_);
		md_api->Initialize(it_config->broker_id, it_config->user_id, it_config->password, it_config->addresses);
		md_apis_.insert(md_api);
	}

	CheckSubscribes();

	md_inited_ = true;
}

void CQuoteService::InitTradeWrappers()
{
	if (trader_inited_)
		return;

	trade_api_ = new CThostTradeApiWrapper(data_center_, gui_action_);
	trade_api_->Initialize(trade_server_config_.broker_id,
		trade_server_config_.user_id,
		trade_server_config_.password,
		trade_server_config_.addresses,
		trade_server_config_.user_product_info,
		trade_server_config_.auth_code,
		trade_server_config_.app_id);

	data_center_->set_trade_api(trade_api_);

	trader_inited_ = true;
}

void CQuoteService::CheckSubscribes()
{
	// 把需要订阅的合约分配给行情终端
	for (auto it_md = md_apis_.begin(); it_md != md_apis_.end(); it_md++) {
		std::set<std::string> need_subscribes;
		for (auto it_inst = instruments_.begin(); it_inst != instruments_.end(); it_inst++) {
			auto it_product = subscribe_products_.find(it_inst->product_id);
			if (it_product != subscribe_products_.end()) {
				need_subscribes.insert(it_inst->instrument_id);
			}
			else {
				auto it_subscr_inst = subscribe_instrument_ids_.find(it_inst->instrument_id);
				if (it_subscr_inst != subscribe_instrument_ids_.end()) {
					need_subscribes.insert(it_inst->instrument_id);
				}
			}
		}

		// 如果还没获取所有的合约列表，先订阅指定的合约
		for (auto it_subscr_inst = subscribe_instrument_ids_.begin();
			it_subscr_inst != subscribe_instrument_ids_.end();
			it_subscr_inst++) {
			auto it_inst = need_subscribes.find(*it_subscr_inst);
			if (it_inst == need_subscribes.end()) {
				need_subscribes.insert(*it_subscr_inst);
			}
		}
		if (need_subscribes.size() > 0) {
			(*it_md)->ReqSubscribeQuote(need_subscribes);
		}
	}
}

void CQuoteService::OnLoginProcess(ApiEvent api_event, const char* content /*= NULL*/, int error_id /*= 0*/, const char* error_msg /*= NULL*/)
{
	CQuoteServerHandler::OnLoginProcess(api_event, content, error_id, error_msg);

	switch (api_event)
	{
	case ApiEvent::ApiEvent_MdLoginSuccess:
		md_logined_ = 1;
		break;
	case ApiEvent::ApiEvent_MdLoginFailed:
	case ApiEvent::ApiEvent_MdLogoutSuccess:
		md_logined_ = -1;
		break;
	case ApiEvent::ApiEvent_TradeLoginSuccess:
		trade_logined_ = 1;
		break;
	case ApiEvent::ApiEvent_TradeLoginFailed:
	case ApiEvent::ApiEvent_TradeLogoutSuccess:
		trade_logined_ = -1;
		break;
	default:
		break;
	}

	switch (api_event)
	{
	case ApiEvent::ApiEvent_MdLoginFailed:
	case ApiEvent::ApiEvent_TradeLoginFailed:
		if (md_logined_ != 0 && trade_logined_ != 0) {
			::SetEvent(login_handle);
		}
		break;
	// 	case ApiEvent::ApiEvent_MdLoginSuccess:
	// 	case ApiEvent::ApiEvent_MdLogoutSuccess:
	// 	case ApiEvent::ApiEvent_TradeLoginSuccess:
	// 	case ApiEvent::ApiEvent_TradeLogoutSuccess:
	case ApiEvent::ApiEvent_QryInstrumentSuccess:
		instrument_inited_ = true;
		break;
	case ApiEvent::ApiEvent_QryPositionDetailSuccess:
		position_list_inited_ = true;
		break;
	}

	if (md_logined_ != 0 && trade_logined_ != 0 && instrument_inited_ && position_list_inited_) {
		::SetEvent(login_handle);
	}
}

void CQuoteService::RefreshAccount(const TradingAccount& account)
{
	CQuoteServerHandler::RefreshAccount(account);
}

void CQuoteService::RefreshQuotes(const std::set<Quote>& quotes)
{
	CQuoteServerHandler::RefreshQuotes(quotes);
}

void CQuoteService::RefreshInstruments(const std::set<Instrument>& instruments)
{
	CQuoteServerHandler::RefreshInstruments(instruments);
}

void CQuoteService::RefreshInstrumentMarginRate(const InstrumentMarginRate& margin_rate)
{
	CQuoteServerHandler::RefreshInstrumentMarginRate(margin_rate);
}

void CQuoteService::RefreshPositions(const std::set<Position>& positions)
{
	CQuoteServerHandler::RefreshPositions(positions);
}

void CQuoteService::RefreshPositionDetails(const std::set<PositionDetail>& positions)
{
	CQuoteServerHandler::RefreshPositionDetails(positions);
}

void CQuoteService::RefreshOrders(const std::set<Order>& orders)
{
	CQuoteServerHandler::RefreshOrders(orders);
}

void CQuoteService::RefreshTrades(const std::set<Trade>& trades)
{
	CQuoteServerHandler::RefreshTrades(trades);
}

void CQuoteService::RefreshOrder(const Order& order)
{
	CQuoteServerHandler::RefreshOrder(order);
}

void CQuoteService::RefreshTrade(const Trade& trade)
{
	CQuoteServerHandler::RefreshTrade(trade);
}
