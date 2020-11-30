#include "stdafx.h"
#include "ThostTradeApiWrapper.h"
#include "ThostSpiHandler.h"
#include "QryManager.h"
#include "DataCenter.h"
#include "DataTypes/GuiDataAction.h"
#include "Utils/Utils.h"


CThostTradeApiWrapper::CThostTradeApiWrapper(CDataCenter* data_center, IGuiDataAction* gui_action) :
	CThostBaseWrapper(data_center, gui_action),
	trader_api_(NULL), connected_(false), logined_(false), login_times_(0),
	connect_timer_id_(100),
	qry_manager_(new CQryManager())
{
}


CThostTradeApiWrapper::~CThostTradeApiWrapper()
{
}

void CThostTradeApiWrapper::OnProcessMsg(CThostSpiMessage* msg)
{
	switch (msg->msg_type())
	{
	case SPI::OnTradeFrontConnected:
	{
		ExitTimer(connect_timer_id_);
		connected_ = true;
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_ConnectSuccess);
		}
		ReqUserLogin();
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
	}
}

void CThostTradeApiWrapper::OnTimer(int timer_id)
{
	if (timer_id == connect_timer_id_) {
		ExitTimer(timer_id);
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_ConnectTimeout);
		}
		return;
	}
}

void CThostTradeApiWrapper::Initialize(const std::string& broker_id, const std::string& user_id, const std::string& password, const std::vector<std::string> fronts)
{
	CThostBaseWrapper::Initialize(broker_id, user_id, password, fronts);

	if (!connected_) {
		ReqConnect();
	}
	else {
		ReqUserLogin();
	}
}

void CThostTradeApiWrapper::ReqConnect()
{
	if (trader_api_) {
		trader_api_->Release();
	}

	std::string path = GetTempPath(user_id());
	trader_api_ = CThostFtdcTraderApi::CreateFtdcTraderApi(path.c_str());
	trader_spi_handler_ = std::shared_ptr<CThostTraderSpiHandler>(new CThostTraderSpiHandler(this));
	trader_api_->RegisterSpi(trader_spi_handler_.get());

	for (auto it = fronts_.begin(); it != fronts_.end(); it++) {
		trader_api_->RegisterFront((char *)it->c_str());
	}
	trader_api_->Init();

	trader_api_->SubscribePrivateTopic(THOST_TERT_QUICK);
	trader_api_->SubscribePublicTopic(THOST_TERT_RESTART);

	CreateTimer(connect_timer_id_, MaxConnectTimeout);
}

void CThostTradeApiWrapper::ReqUserLogin()
{
	CThostFtdcReqUserLoginField login_field;
	memset(&login_field, 0, sizeof(CThostFtdcReqUserLoginField));
	safe_strcpy(login_field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	safe_strcpy(login_field.UserID, user_id(), sizeof(TThostFtdcUserIDType));
	safe_strcpy(login_field.Password, password(), sizeof(TThostFtdcPasswordType));
	//version
	safe_strcpy(login_field.UserProductInfo, "EasyTrader", sizeof(TThostFtdcProductInfoType));
	int reqid = GetRequestId();
	trader_api_->ReqUserLogin(&login_field, reqid);
}

int CThostTradeApiWrapper::ReqQryTradingAccount()
{
	CThostFtdcQryTradingAccountField field;
	safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	safe_strcpy(field.CurrencyID, "CNY", sizeof(TThostFtdcCurrencyIDType));
	int reqid = GetRequestId();
	trader_api_->ReqQryTradingAccount(&field, reqid);
	return reqid;
}

int CThostTradeApiWrapper::ReqQryAllInstrument()
{
	CThostFtdcQryInstrumentField field;
	memset(&field, 0, sizeof(CThostFtdcQryInstrumentField));
	int reqid = GetRequestId();
	trader_api_->ReqQryInstrument(&field, reqid);
	return reqid;
}

int CThostTradeApiWrapper::ReqQryOrder()
{
	CThostFtdcQryOrderField field;
	memset(&field, 0, sizeof(CThostFtdcQryOrderField));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	trader_api_->ReqQryOrder(&field, reqid);
	return reqid;
}

int CThostTradeApiWrapper::ReqQryTrade()
{
	CThostFtdcQryTradeField field;
	memset(&field, 0, sizeof(CThostFtdcQryTradeField));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	trader_api_->ReqQryTrade(&field, reqid);
	return reqid;
}

int CThostTradeApiWrapper::ReqQryPosition()
{
	CThostFtdcQryInvestorPositionField field;
	memset(&field, 0, sizeof(CThostFtdcQryInvestorPositionField));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	trader_api_->ReqQryInvestorPosition(&field, reqid);
	return reqid;
}

int CThostTradeApiWrapper::ReqQryPositionDetail()
{
	CThostFtdcQryInvestorPositionDetailField field;
	memset(&field, 0, sizeof(CThostFtdcQryInvestorPositionDetailField));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	trader_api_->ReqQryInvestorPositionDetail(&field, reqid);
	return reqid;
}

int CThostTradeApiWrapper::ReqQryDepthMarketData()
{
	CThostFtdcQryDepthMarketDataField field;
	memset(&field, 0, sizeof(CThostFtdcQryDepthMarketDataField));
	int reqid = GetRequestId();
	trader_api_->ReqQryDepthMarketData(&field, reqid);
	return reqid;
}

void CThostTradeApiWrapper::OnRspUserLogin(CThostSpiMessage* msg)
{
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_LoginFailed);
		}
	}
	else {
		logined_ = true;
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_LoginSuccess);
		}

		if (!login_times_) { // 第一次登录
			qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQryAllInstrument, this), "查询合约");
		}
	}
}

void CThostTradeApiWrapper::OnRspUserLogout(CThostSpiMessage* msg)
{

}

void CThostTradeApiWrapper::OnRspQryOrder(CThostSpiMessage* msg)
{
	CThostFtdcOrderField* f = msg->GetFieldPtr<CThostFtdcOrderField>();
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_QryOrderFailed, "");
		}
	}
	else {
		Order order(*f);
	}
}

void CThostTradeApiWrapper::OnRspQryTrade(CThostSpiMessage* msg)
{

}

void CThostTradeApiWrapper::OnRspQryInvestorPosition(CThostSpiMessage* msg)
{

}

void CThostTradeApiWrapper::OnRspQryTradingAccount(CThostSpiMessage* msg)
{
	CThostFtdcTradingAccountField* f = msg->GetFieldPtr<CThostFtdcTradingAccountField>();
	if (msg->rsp_field()->ErrorID) {
		gui_action_->OnLoginProcess(ApiEvent_QryTradingAccountFailed, "查询帐户资金失败", msg->rsp_field()->ErrorID);
	}
	else {
		TradingAccount account(*f);

		if (gui_action_) {
			gui_action_->RefreshAccount(account);
			gui_action_->OnLoginProcess(ApiEvent_QryTradingAccountSuccess, "查询帐户资金成功");
		}
	}
}

void CThostTradeApiWrapper::OnRspQryInstrument(CThostSpiMessage* msg)
{
	CThostFtdcInstrumentField* f = msg->GetFieldPtr<CThostFtdcInstrumentField>();
	if (msg->rsp_field()->ErrorID) {
		gui_action_->OnLoginProcess(ApiEvent_QryInstrumentFailed, "查询合约失败", msg->rsp_field()->ErrorID);
	}
	else {
		Instrument inst(*f);
		instruments_cache_.insert(inst);

		if (msg->is_last()) {
			if (data_center_) {
				data_center_->OnRtnInstruments(instruments_cache_);
			}

			if (gui_action_) {
				gui_action_->RefreshInstruments(instruments_cache_);
			}
		}
		instruments_cache_.clear();
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryInstrumentSuccess, "查询合约成功");
		}
	}
}

void CThostTradeApiWrapper::OnRspQryDepthMarketData(CThostSpiMessage* msg)
{

}

void CThostTradeApiWrapper::OnRspQryInvestorPositionDetail(CThostSpiMessage* msg)
{

}
