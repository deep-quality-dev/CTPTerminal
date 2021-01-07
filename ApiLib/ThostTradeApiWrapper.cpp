#include "stdafx.h"
#include "ThostTradeApiWrapper.h"
#include "ThostSpiHandler.h"
#include "QryManager.h"
#include "DataCenter.h"
#include "GuiDataAction.h"
#include "DataTypes/FtdcTranslator.h"
#include "DataTypes/Formatter.h"
#include "Utils/Utils.h"
#include "Utils/Logger.h"
#include <sstream>

#pragma comment(lib, "ThostApi/thosttraderapi_se.lib")
// #pragma comment(lib, "Site/CPP/ChnFutureSite/ThostApi/thosttraderapi_se.lib")

CThostTradeApiWrapper::CThostTradeApiWrapper(CDataCenter* data_center, IGuiDataAction* gui_action) :
	CThostBaseWrapper(data_center, gui_action),
	trader_api_(NULL), trader_spi_handler_(NULL), qry_manager_(NULL),
	connected_(false), authenticated_(false), logined_(false), login_times_(0), force_logout_(false),
	connect_timer_id_(100)
{
	logout_event_ = ::CreateEventW(NULL, FALSE, FALSE, L"TRADE_LOGOUT_EVENT");
}


CThostTradeApiWrapper::~CThostTradeApiWrapper()
{
	::CloseHandle(logout_event_);
}

void CThostTradeApiWrapper::OnProcessMsg(CThostSpiMessage* msg)
{
	switch (msg->msg_type())
	{
	case SPI::OnTradeFrontConnected:
	{
		OnRspConnected(msg);
		break;
	}
	case SPI::OnTradeFrontDisconnected:
	{
		OnRspDisconnected(msg);
		break;
	}
	case SPI::OnRspAuthenticate:
	{
		OnRspAuthenticate(msg);
		break;
	}
	case SPI::OnRspUserLogin:
	{
		OnRspUserLogin(msg);
		break;
	}
	case SPI::OnRspUserLogout:
	{
		OnRspUserLogout(msg);
		break;
	}
	case SPI::OnRspQryOrder:
	{
		OnRspQryOrder(msg);
		break;
	}
	case SPI::OnRspQryTrade:
	{
		OnRspQryTrade(msg);
		break;
	}
	case SPI::OnRspQryInvestorPosition:
	{
		OnRspQryInvestorPosition(msg);
		break;
	}
	case SPI::OnRspQryTradingAccount:
	{
		OnRspQryTradingAccount(msg);
		break;
	}
	case SPI::OnRspQryInstrumentMarginRate:
	{
		OnRspQryInstrumentMarginRate(msg);
		break;
	}
	case SPI::OnRspQryInstrument:
	{
		OnRspQryInstrument(msg);
		break;
	}
	case SPI::OnRspQryDepthMarketData:
	{
		OnRspQryDepthMarketData(msg);
		break;
	}
	case SPI::OnRspQryInvestorPositionDetail:
	{
		OnRspQryInvestorPositionDetail(msg);
		break;
	}
	case SPI::OnRspSettlementInfoConfirm:
	{
		OnRspSettlementInfoConfirm(msg);
		break;
	}
	case SPI::OnRspQrySettlementInfo:
	{
		OnRspQrySettlementInfo(msg);
		break;
	}
	case SPI::OnRspQrySettlementInfoConfirm:
	{
		OnRspQrySettlementInfoConfirm(msg);
		break;
	}
	case SPI::OnRtnOrder:
	{
		OnRtnOrder(msg);
		break;
	}
	case SPI::OnRtnTrade:
	{
		OnRtnTrade(msg);
		break;
	}
	case SPI::OnErrRtnOrderInsert:
	{
		OnErrRtnOrderInsert(msg);
		break;
	}
	case SPI::OnRspOrderInsert:
	{
		OnRspOrderInsert(msg);
		break;
	}
	}

	if (msg) {
		if (msg->is_last() || !msg->GetFieldPtr<void*>()) {
			qry_manager_->CheckQuery(msg->request_id(), msg->rsp_field()->ErrorID);
		}
	}
}

void CThostTradeApiWrapper::OnTimer(int timer_id)
{
	if (timer_id == connect_timer_id_) {
		ExitTimer(timer_id);
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_TradeConnectTimeout, "交易服务器链接超时");
		}
		return;
	}
}

void CThostTradeApiWrapper::Initialize(const std::string& broker_id,
	const std::string& user_id,
	const std::string& password,
	const std::vector<std::string>& fronts,
	const std::string& user_product_info,
	const std::string& auth_code,
	const std::string& app_id)
{
	CreateThread();
	qry_manager_ = new CQryManager();
	qry_manager_->CreateThread();

	CThostBaseWrapper::Initialize(broker_id, user_id, password, fronts);

	Utils::Log(std::string("API版本: ") + GetApiVersion());

	user_product_info_ = user_product_info;
	auth_code_ = auth_code;
	app_id_ = app_id;

	if (app_id_.length() < 1 || auth_code.length() < 1) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_AuthenticationFailed, "交易终端认证失败");
		}
		return;
	}
}

void CThostTradeApiWrapper::Deinitialize()
{
	if (qry_manager_) {
		qry_manager_->ExitThread();
		delete qry_manager_;
		qry_manager_ = NULL;
	}

	ExitThread();

	if (trader_api_) {
		trader_api_->RegisterSpi(NULL);
		trader_api_->Release();
		trader_api_ = NULL;
	}

	if (trader_spi_handler_) {
		delete trader_spi_handler_;
		trader_spi_handler_ = NULL;
	}
}

const char* CThostTradeApiWrapper::GetApiVersion()
{
	if (trader_api_) {
		return trader_api_->GetApiVersion();
	}
	return "";
}

void CThostTradeApiWrapper::Login()
{
	force_logout_ = false;

	if (!connected_) {
		ReqConnect();
	}
	else if (authenticated_ && !logined_) {
		ReqUserLogin();
	}
	else {
		ReqAuthenticate();
	}
}

void CThostTradeApiWrapper::Logout()
{
	force_logout_ = true;

	if (logined_) {
		::ResetEvent(logout_event_);
		ReqUserLogout();
		::WaitForSingleObject(logout_event_, 10000);
	}
// 	else if (connected_) {
// 		if (trader_api_) {
// 			trader_api_->Release();
// 			trader_api_ = NULL;
// 		}
// 		connected_ = false;
// 	}
}

void CThostTradeApiWrapper::ReqConnect()
{
	if (trader_api_) {
		trader_api_->RegisterSpi(NULL);
		trader_api_->Release();
		trader_api_ = NULL;
	}

	if (trader_spi_handler_) {
		delete trader_spi_handler_;
		trader_spi_handler_ = NULL;
	}

	std::string path = Utils::GetTempPath(user_id());
	trader_api_ = CThostFtdcTraderApi::CreateFtdcTraderApi(path.c_str());
	trader_spi_handler_ = new CThostTraderSpiHandler(this);
	trader_api_->RegisterSpi(trader_spi_handler_);

	for (auto it = fronts_.begin(); it != fronts_.end(); it++) {
		trader_api_->RegisterFront((char *)it->c_str());
	}
	trader_api_->Init();

	trader_api_->SubscribePrivateTopic(THOST_TERT_QUICK);
	trader_api_->SubscribePublicTopic(THOST_TERT_RESTART);

	CreateTimer(connect_timer_id_, MaxConnectTimeout);
}

void CThostTradeApiWrapper::ReqAuthenticate()
{
	CThostFtdcReqAuthenticateField field = { 0 };
	Utils::safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	Utils::safe_strcpy(field.UserID, user_id(), sizeof(TThostFtdcUserIDType));
	Utils::safe_strcpy(field.UserProductInfo, user_product_info_.c_str(), sizeof(TThostFtdcProductInfoType));
	Utils::safe_strcpy(field.AuthCode, auth_code_.c_str(), sizeof(TThostFtdcAuthCodeType));
	Utils::safe_strcpy(field.AppID, app_id_.c_str(), sizeof(TThostFtdcAppIDType));

	int reqid = GetRequestId();
	trader_api_->ReqAuthenticate(&field, reqid);
}

void CThostTradeApiWrapper::ReqUserLogin()
{
	CThostFtdcReqUserLoginField login_field;
	memset(&login_field, 0, sizeof(CThostFtdcReqUserLoginField));
	Utils::safe_strcpy(login_field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	Utils::safe_strcpy(login_field.UserID, user_id(), sizeof(TThostFtdcUserIDType));
	Utils::safe_strcpy(login_field.Password, password(), sizeof(TThostFtdcPasswordType));
	//version
	Utils::safe_strcpy(login_field.UserProductInfo, "", sizeof(TThostFtdcProductInfoType));
	int reqid = GetRequestId();
	trader_api_->ReqUserLogin(&login_field, reqid);
}

void CThostTradeApiWrapper::ReqUserLogout()
{
	CThostFtdcUserLogoutField logout_field;
	memset(&logout_field, 0, sizeof(CThostFtdcUserLogoutField));
	Utils::safe_strcpy(logout_field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	Utils::safe_strcpy(logout_field.UserID, user_id(), sizeof(TThostFtdcUserIDType));
	int reqid = GetRequestId();
	trader_api_->ReqUserLogout(&logout_field, reqid);
}

int CThostTradeApiWrapper::ReqSettlementInfo()
{
	CThostFtdcSettlementInfoConfirmField field;
	memset(&field, 0, sizeof(CThostFtdcSettlementInfoConfirmField));
	Utils::safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	int ret = trader_api_->ReqSettlementInfoConfirm(&field, reqid);
	return ret < 0 ? ret : reqid;
}

int CThostTradeApiWrapper::ReqQrySettlementInfoConfirm()
{
	CThostFtdcQrySettlementInfoConfirmField field;
	memset(&field, 0, sizeof(CThostFtdcQrySettlementInfoConfirmField));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	int ret = trader_api_->ReqQrySettlementInfoConfirm(&field, reqid);
	return ret < 0 ? ret : reqid;
}

int CThostTradeApiWrapper::ReqQrySettlementInfo(unsigned int date)
{
	CThostFtdcQrySettlementInfoField field;
	memset(&field, 0, sizeof(CThostFtdcQrySettlementInfoField));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	if (date != 0) {
		Utils::safe_strcpy(field.TradingDay, CFormatter::GetInstance().Value2String(date).c_str(), sizeof(TThostFtdcDateType));
	}
	int reqid = GetRequestId();
	int ret = trader_api_->ReqQrySettlementInfo(&field, reqid);
	return ret < 0 ? ret : reqid;
}

int CThostTradeApiWrapper::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField field;
	memset(&field, 0, sizeof(CThostFtdcQryTradingAccountField));
	Utils::safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	Utils::safe_strcpy(field.CurrencyID, "CNY", sizeof(TThostFtdcCurrencyIDType));
	int reqid = GetRequestId();
	int ret = trader_api_->ReqQryTradingAccount(&field, reqid);
	return ret < 0 ? ret : reqid;
}

int CThostTradeApiWrapper::ReqQryAllInstrument()
{
	CThostFtdcQryInstrumentField field;
	memset(&field, 0, sizeof(CThostFtdcQryInstrumentField));
	int reqid = GetRequestId();
	int ret = trader_api_->ReqQryInstrument(&field, reqid);
	return ret < 0 ? ret : reqid;
}

int CThostTradeApiWrapper::ReqQryOrder()
{
	orders_cache_.clear();
	CThostFtdcQryOrderField field;
	memset(&field, 0, sizeof(CThostFtdcQryOrderField));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	int ret = trader_api_->ReqQryOrder(&field, reqid);
	return ret < 0 ? ret : reqid;
}

int CThostTradeApiWrapper::ReqQryTrade()
{
	trades_cache_.clear();
	CThostFtdcQryTradeField field;
	memset(&field, 0, sizeof(CThostFtdcQryTradeField));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	int ret = trader_api_->ReqQryTrade(&field, reqid);
	return ret < 0 ? ret : reqid;
}

int CThostTradeApiWrapper::ReqQryPosition()
{
	positions_cache_.clear();
	CThostFtdcQryInvestorPositionField field;
	memset(&field, 0, sizeof(CThostFtdcQryInvestorPositionField));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	int ret = trader_api_->ReqQryInvestorPosition(&field, reqid);
	return ret < 0 ? ret : reqid;
}

int CThostTradeApiWrapper::ReqQryPositionDetail()
{
	position_details_cache_.clear();
	positions_cache_.clear();
	CThostFtdcQryInvestorPositionDetailField field;
	memset(&field, 0, sizeof(CThostFtdcQryInvestorPositionDetailField));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	int ret = trader_api_->ReqQryInvestorPositionDetail(&field, reqid);
	return ret < 0 ? ret : reqid;
}

int CThostTradeApiWrapper::ReqQryDepthMarketData()
{
	CThostFtdcQryDepthMarketDataField field;
	memset(&field, 0, sizeof(CThostFtdcQryDepthMarketDataField));
	int reqid = GetRequestId();
	int ret = trader_api_->ReqQryDepthMarketData(&field, reqid);
	return ret < 0 ? ret : reqid;
}

int CThostTradeApiWrapper::ReqInsertOrder(const OrderInsert& order_insert)
{
	CThostFtdcInputOrderField field;
	memset(&field, 0, sizeof(CThostFtdcInputOrderField));
	Utils::safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	Utils::safe_strcpy(field.InstrumentID, order_insert.instrument_id.c_str(), sizeof(TThostFtdcInstrumentIDType));
	GetFromTickplusDirection(field.Direction, order_insert.direction);
	field.TimeCondition = (order_insert.is_market_order ? THOST_FTDC_TC_IOC : THOST_FTDC_TC_GFD);
	field.OrderPriceType = (order_insert.is_market_order ? THOST_FTDC_OPT_AnyPrice : THOST_FTDC_OPT_LimitPrice);
	GetFromTickplusOffsetFlag(field.CombOffsetFlag, order_insert.offset_flag);
	field.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	field.LimitPrice = order_insert.limit_price;
	field.VolumeTotalOriginal = order_insert.volume;
	field.VolumeCondition = THOST_FTDC_VC_AV;
	field.ContingentCondition = THOST_FTDC_CC_Immediately;
	field.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	std::stringstream ss;
	ss << order_insert.order_ref;
	Utils::safe_strcpy(field.OrderRef, ss.str().c_str(), sizeof(TThostFtdcOrderRefType));
	field.RequestID = GetRequestId();
	int ret = trader_api_->ReqOrderInsert(&field, field.RequestID);
	return ret < 0 ? ret : field.RequestID;
}

void CThostTradeApiWrapper::ReqQryMarginRate(const std::string& instrument_id)
{
	if (querying_margin_rates_.find(instrument_id) == querying_margin_rates_.end() &&
		ready_margin_rates_.find(instrument_id) == ready_margin_rates_.end()) {
		qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQryInstrumentMarginRate, this, instrument_id), "查询合约");
	}
}

int CThostTradeApiWrapper::ReqCancelOrder(const Order& order)
{
	CThostFtdcInputOrderActionField	field;
	memset(&field, 0, sizeof(CThostFtdcInputOrderActionField));
	Utils::safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	field.ActionFlag = THOST_FTDC_AF_Delete;		//ActionFlag;
	field.FrontID = front_id();
	field.SessionID = session_id();
	Utils::safe_strcpy(field.OrderSysID, order.order_sys_id.c_str(), sizeof(TThostFtdcOrderSysIDType));
	field.RequestID = GetRequestId();
	int ret = trader_api_->ReqOrderAction(&field, field.RequestID);
	return ret < 0 ? ret : field.RequestID;
}

int CThostTradeApiWrapper::ReqQryInstrumentMarginRate(const std::string& instrument_id)
{
	Utils::Log("ReqQryInstrumentMarginRate << " + instrument_id);

	CThostFtdcQryInstrumentMarginRateField field;
	memset(&field, 0, sizeof(CThostFtdcQryInstrumentMarginRateField));
	Utils::safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	Utils::safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	Utils::safe_strcpy(field.InstrumentID, instrument_id.c_str(), sizeof(TThostFtdcInstrumentIDType));
	//speculation
	field.HedgeFlag = '1';
	int reqid = GetRequestId();
	int ret = trader_api_->ReqQryInstrumentMarginRate(&field, reqid);
	return ret < 0 ? ret : reqid;
}

void CThostTradeApiWrapper::OnRspConnected(CThostSpiMessage* msg)
{
	ExitTimer(connect_timer_id_);
	connected_ = true;
	if (gui_action_) {
		gui_action_->OnLoginProcess(ApiEvent::ApiEvent_TradeConnectSuccess, "交易服务器链接成功");
	}
	ReqAuthenticate();
}

void CThostTradeApiWrapper::OnRspDisconnected(CThostSpiMessage* msg)
{
	connected_ = false;
	if (gui_action_) {
		gui_action_->OnLoginProcess(ApiEvent::ApiEvent_TradeDisconnected, "交易服务器断链");
	}

	if (force_logout_) {
// 		if (trader_api_) {
// 			trader_api_->Release();
// 			trader_api_ = NULL;
//		}
		::SetEvent(logout_event_);
	}
}

void CThostTradeApiWrapper::OnRspAuthenticate(CThostSpiMessage* msg)
{
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_AuthenticationFailed,
				"交易终端认证失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
	}
	else {
		CThostFtdcRspAuthenticateField* f = msg->GetFieldPtr<CThostFtdcRspAuthenticateField>();

		Utils::Log("交易终端认证成功 >>");
		std::stringstream ss;
		ss << "经纪公司代码: " << f->BrokerID << std::endl
			<< "用户代码: " << f->UserID << std::endl
			<< "用户端产品信息: " << f->UserProductInfo << std::endl
			<< "App代码: " << f->AppID << std::endl
			<< "App类型: " << f->AppType << std::endl;
		Utils::Log(ss.str());
		Utils::Log("交易终端认证成功 <<");

		authenticated_ = true;
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_AuthenticationSuccess, "交易终端认证成功");
		}

		ReqUserLogin();
	}
}

void CThostTradeApiWrapper::OnRspUserLogin(CThostSpiMessage* msg)
{
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_TradeLoginFailed,
				"交易服务器登录失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
	}
	else {
		CThostFtdcRspUserLoginField* f = msg->GetFieldPtr<CThostFtdcRspUserLoginField>();

		Utils::Log("登录交易服务器成功 >>");
		std::stringstream ss;
		ss << "交易日: " << f->TradingDay << std::endl
			<< "登录成功时间: " << f->LoginTime << std::endl
			<< "经纪公司代码: " << f->BrokerID << std::endl
			<< "用户代码: " << f->UserID << std::endl
			<< "前置编号: " << f->FrontID << std::endl
			<< "会话编号: " << f->SessionID << std::endl
			<< "最大报单引用: " << f->MaxOrderRef << std::endl;
		Utils::Log(ss.str());
		Utils::Log("登录交易服务器成功 <<");

		front_id_ = f->FrontID;
		session_id_ = f->SessionID;
		order_ref_ = atoi(f->MaxOrderRef);
		if (data_center_) {
			data_center_->set_order_ref(order_ref_);
		}

		logined_ = true;

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_TradeLoginSuccess, "交易服务器登录成功");
		}

		if (!login_times_) { // 第一次登录
			qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQrySettlementInfoConfirm, this), "查询结算信息");
			qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQryAllInstrument, this), "查询合约");
// 			qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQryOrder, this), "查询委托");
// 			qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQryTrade, this), "查询成交");
//			qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQryPosition, this), "查询持仓");
			qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQryPositionDetail, this), "查询持仓明细");
		}

		login_times_++;
	}
}

void CThostTradeApiWrapper::OnRspUserLogout(CThostSpiMessage* msg)
{
	logined_ = false;
	if (gui_action_) {
		gui_action_->OnLoginProcess(ApiEvent::ApiEvent_TradeLogoutSuccess, "交易服务器注销");
	}

	if (force_logout_) {
// 		if (trader_api_) {
// 			trader_api_->Release();
// 			trader_api_ = NULL;
// 		}
		::SetEvent(logout_event_);
	}
}

void CThostTradeApiWrapper::OnRtnOrder(CThostSpiMessage* msg)
{
	CThostFtdcOrderField* f = msg->GetFieldPtr<CThostFtdcOrderField>();
	if (f) {
		Order order(*f);

		orderkey2sysid_[order.GetKey()] = order.order_sys_id;
		sysid2orderkey_[order.order_sys_id] = order.GetKey();

		bool current_session = true;
		if (f->FrontID != front_id() || f->SessionID != session_id()) {
			order.status_msg = "非本机发送或非本次登录发送, " + order.status_msg;
			current_session = false;
		}

		if (data_center_/* && current_session*/) {
			data_center_->OnRtnOrder(order);
		}

		if (gui_action_) {
			gui_action_->RefreshOrder(order);
		}

		std::set<Trade> need_del;
		for (auto it_trade = rtn_trade_cache_.begin();
			it_trade != rtn_trade_cache_.end(); it_trade++) {
			auto it_order = sysid2orderkey_.find(it_trade->order_sys_id);
			if (it_order != sysid2orderkey_.end()) {
				Trade trade(*it_trade);
				need_del.insert(trade);

				if (data_center_/* && current_session*/) {
					data_center_->OnRtnTrade(trade);

					if (gui_action_) {
						gui_action_->RefreshTrade(trade);
					}

					std::set<Position> positions = data_center_->positions();
					if (gui_action_) {
						gui_action_->RefreshPositions(positions);
					}
				}
			}
		}
		for (auto it_trade = need_del.begin();
			it_trade != need_del.end(); it_trade++) {
			rtn_trade_cache_.erase(*it_trade);
		}
	}
}

void CThostTradeApiWrapper::OnRtnTrade(CThostSpiMessage* msg)
{
	CThostFtdcTradeField* f = msg->GetFieldPtr<CThostFtdcTradeField>();
	if (f) {
		Trade trade(*f);

		bool current_session = true;
		auto it_order = sysid2orderkey_.find(trade.order_sys_id);
		if (it_order != sysid2orderkey_.end()) {
			if (it_order->second.front_id != front_id() || it_order->second.session_id != session_id()) {
				current_session = false;
			}

			ASSERT_TRUE(atoi(f->OrderRef) == it_order->second.order_ref);
			if (data_center_) {
				data_center_->OnRtnTrade(trade);

				if (gui_action_) {
					gui_action_->RefreshTrade(trade);
				}

				std::set<Position> positions = data_center_->positions();
				if (gui_action_) {
					gui_action_->RefreshPositions(positions);
				}
			}
		}
		else {
			rtn_trade_cache_.insert(trade);
		}
	}
}

void CThostTradeApiWrapper::OnErrRtnOrderInsert(CThostSpiMessage* msg)
{
	CThostFtdcInputOrderField* f = msg->GetFieldPtr<CThostFtdcInputOrderField>();
	if (f) {
		OrderKey key(front_id(), session_id(), atoi(f->OrderRef));
		Order order(*f, key, msg->rsp_field()->ErrorID, msg->rsp_field()->ErrorMsg);
		order.request_id = f->RequestID;

		if (order.status == Status_Error) {
			bool current_session = true;

			if (data_center_/* && current_session*/)
			{
				data_center_->OnRtnOrder(order);
			}
		}

		if (gui_action_) {
			gui_action_->RefreshOrder(order);
		}
	}
}

void CThostTradeApiWrapper::OnRspOrderInsert(CThostSpiMessage* msg)
{
	CThostFtdcInputOrderField* f = msg->GetFieldPtr<CThostFtdcInputOrderField>();
	if (f) {
		OrderKey key(front_id(), session_id(), atoi(f->OrderRef));
		Order order(*f, key, msg->rsp_field()->ErrorID, msg->rsp_field()->ErrorMsg);
		order.request_id = f->RequestID;

		if (order.status == Status_Error) {
			bool current_session = true;

			if (data_center_/* && current_session*/)
			{
				data_center_->OnRtnOrder(order);
			}
		}

		if (gui_action_) {
			gui_action_->RefreshOrder(order);
		}
	}
}

void CThostTradeApiWrapper::OnRspQryOrder(CThostSpiMessage* msg)
{
	CThostFtdcOrderField* f = msg->GetFieldPtr<CThostFtdcOrderField>();
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_QryOrderFailed, 
				"获取报单列表失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
		return;
	}
	if (f) {
		Order order(*f);
		orders_cache_.insert(order);

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryOrderSuccess,
				("查询报单列表成功, " + order.instrument_id).c_str());
		}
	}

	if (msg->is_last()) {
		if (data_center_) {
			data_center_->OnRspQryOrders(orders_cache_);
		}

		if (gui_action_) {
			gui_action_->RefreshOrders(orders_cache_);
		}
		orders_cache_.clear();
	}
}

void CThostTradeApiWrapper::OnRspQryTrade(CThostSpiMessage* msg)
{
	CThostFtdcTradeField* f = msg->GetFieldPtr<CThostFtdcTradeField>();
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_QryTradeFailed,
				"获取成交列表失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
		return;
	}
	if (f) {
		Trade trade(*f);
		trades_cache_.insert(trade);

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryTradeSuccess,
				("查询成交列表成功, " + trade.instrument_id).c_str());
		}
	}

	if (msg->is_last()) {
		if (data_center_) {
			data_center_->OnRspQryTrades(trades_cache_);
		}

		if (gui_action_) {
			gui_action_->RefreshTrades(trades_cache_);
		}
		trades_cache_.clear();
	}
}

void CThostTradeApiWrapper::OnRspQryInvestorPosition(CThostSpiMessage* msg)
{
	CThostFtdcInvestorPositionField* f = msg->GetFieldPtr<CThostFtdcInvestorPositionField>();
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryPositionFailed,
				"查询持仓失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
		return;
	}
	if (f) {
		Position position(*f);
		if (position.volume()) {
			auto it_position = std::find_if(positions_cache_.begin(), positions_cache_.end(),
				[&position](const Position& pp) {
				return pp.instrument_id == position.instrument_id && pp.direction == position.direction;
			});
			if (it_position == positions_cache_.end()) {
				positions_cache_.insert(position);
			}
			else {
				position.position_cost = 
					(it_position->position_cost * it_position->volume() + 
						position.position_cost * position.volume()) / (it_position->volume() + position.volume());
				position.today_volume += it_position->today_volume;
				position.yesterday_volume += it_position->yesterday_volume;
				positions_cache_.erase(position);
				positions_cache_.insert(position);
			}
		}
	}

	if (msg->is_last()) {
		if (data_center_) {
			data_center_->OnRtnPositions(positions_cache_);
		}

		if (gui_action_) {
			gui_action_->RefreshPositions(positions_cache_);
		}

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryPositionSuccess, "查询持仓成功");
		}
		positions_cache_.clear();
	}
}

void CThostTradeApiWrapper::OnRspQryTradingAccount(CThostSpiMessage* msg)
{
	CThostFtdcTradingAccountField* f = msg->GetFieldPtr<CThostFtdcTradingAccountField>();
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryTradingAccountFailed,
				"查询帐户资金失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
	}
	else {
		TradingAccount account(*f);

		if (data_center_) {
			data_center_->OnRspTradeAccount(account);
		}

		if (gui_action_) {
			gui_action_->RefreshAccount(account);
			gui_action_->OnLoginProcess(ApiEvent_QryTradingAccountSuccess,
				"查询帐户资金成功");
		}
	}
}

void CThostTradeApiWrapper::OnRspQryInstrumentMarginRate(CThostSpiMessage* msg)
{
	CThostFtdcInstrumentMarginRateField * f = msg->GetFieldPtr<CThostFtdcInstrumentMarginRateField >();
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryInstrumentMarginRateFailed,
				"查询合约保证金率失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
		return;
	}
	if (f) {
		InstrumentMarginRate sell_margin_rate;
		InstrumentMarginRate buy_margin_rate;
		sell_margin_rate.instrument_id = f->InstrumentID;
		buy_margin_rate.instrument_id = f->InstrumentID;
		sell_margin_rate.direction = Sell;
		buy_margin_rate.direction = Buy;
		sell_margin_rate.margin_ratio_by_money = f->ShortMarginRatioByMoney;
		sell_margin_rate.margin_ratio_by_volume = f->ShortMarginRatioByVolume;
		buy_margin_rate.margin_ratio_by_money = f->LongMarginRatioByMoney;
		buy_margin_rate.margin_ratio_by_volume = f->LongMarginRatioByVolume;

		if (data_center_) {
			data_center_->OnRspInstrumentMarginRate(sell_margin_rate);
			data_center_->OnRspInstrumentMarginRate(buy_margin_rate);
		}

		if (gui_action_) {
			gui_action_->RefreshInstrumentMarginRate(sell_margin_rate);
			gui_action_->RefreshInstrumentMarginRate(buy_margin_rate);
		}
	}

	if (gui_action_) {
		gui_action_->OnLoginProcess(ApiEvent_QryInstrumentSuccess, "查询合约保证金率成功");
	}
}

void CThostTradeApiWrapper::OnRspQryInstrument(CThostSpiMessage* msg)
{
	CThostFtdcInstrumentField* f = msg->GetFieldPtr<CThostFtdcInstrumentField>();
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryInstrumentFailed,
				"查询合约失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
		return;
	}
	if (f) {
		Instrument inst(*f);
		instruments_cache_.insert(inst);
	}

	if (msg->is_last()) {
		if (data_center_) {
			data_center_->OnRtnInstruments(instruments_cache_);
		}

		if (gui_action_) {
			gui_action_->RefreshInstruments(instruments_cache_);
		}

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryInstrumentSuccess, "查询合约成功");
		}

		instruments_cache_.clear();
	}
}

void CThostTradeApiWrapper::OnRspQryDepthMarketData(CThostSpiMessage* msg)
{

}

void CThostTradeApiWrapper::OnRspQryInvestorPositionDetail(CThostSpiMessage* msg)
{
	CThostFtdcInvestorPositionDetailField* f = msg->GetFieldPtr<CThostFtdcInvestorPositionDetailField>();
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryPositionDetailFailed,
				"查询持仓明细失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
		return;
	}
	if (f) {
		PositionDetail position_detail(*f);
		if (position_detail.volume > 0)
			position_details_cache_.insert(position_detail);


		Position position(*f);
		if (position.volume()) {
			auto it_position = std::find_if(positions_cache_.begin(), positions_cache_.end(),
				[&position](const Position& pp) {
				return pp.instrument_id == position.instrument_id && pp.direction == position.direction;
			});
			if (it_position == positions_cache_.end()) {
				positions_cache_.insert(position);
			}
			else {
				position.position_cost =
					(it_position->position_cost * it_position->volume() +
						position.position_cost * position.volume()) / (it_position->volume() + position.volume());
				position.today_volume += it_position->today_volume;
				position.yesterday_volume += it_position->yesterday_volume;
				positions_cache_.erase(position);
				positions_cache_.insert(position);
			}
		}
	}

	if (msg->is_last()) {
		if (data_center_) {
			positions_cache_ = data_center_->OnRtnPositions(positions_cache_);
			position_details_cache_ = data_center_->OnRtnPositionDetails(position_details_cache_);
		}

		if (gui_action_) {
			gui_action_->RefreshPositions(positions_cache_);
			gui_action_->RefreshPositionDetails(position_details_cache_);
		}

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryPositionDetailSuccess, "查询持仓明细成功");
		}
		position_details_cache_.clear();
		positions_cache_.clear();
	}
}

void CThostTradeApiWrapper::OnRspSettlementInfoConfirm(CThostSpiMessage* msg)
{
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_SettlementInfoFailed,
				"申请确认结算单失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
		return;
	}
	if (gui_action_) {
		gui_action_->OnLoginProcess(ApiEvent_SettlementInfoSuccess, "结算单已确认");
	}
}

void CThostTradeApiWrapper::OnRspQrySettlementInfo(CThostSpiMessage* msg)
{
	CThostFtdcSettlementInfoField* f = msg->GetFieldPtr<CThostFtdcSettlementInfoField>();
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QrySettlementInfoFailed,
				"查询投资者结算结果失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
		return;
	}
	if (f) {
		settement_info_.append(f->Content);
	}

	if (msg->is_last()) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QrySettlementInfoSuccess, "查询投资者结算结果成功");
		}
		Utils::Log(settement_info_);
		settement_info_.clear();

		qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqSettlementInfo, this), "申请确认结算单");
	}
}

void CThostTradeApiWrapper::OnRspQrySettlementInfoConfirm(CThostSpiMessage* msg)
{
	CThostFtdcSettlementInfoConfirmField* f = msg->GetFieldPtr<CThostFtdcSettlementInfoConfirmField>();
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QrySettlementInfoConfirmFailed, 
				"查询结算信息确认结果失败", 
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
		return;
	}
	if (f) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QrySettlementInfoConfirmSuccess, "结算单已确认");
		}
	}
	else {
		//查询结算信息
		qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQrySettlementInfo, this, 0), "查询结算信息");
	}
}
