#include "stdafx.h"
#include "ThostMdApiWrapper.h"
#include "ThostSpiHandler.h"
#include "DataCenter.h"
#include "GuiDataAction.h"
#include "Utils/Utils.h"
#include "Utils/Logger.h"
#include <sstream>

#pragma comment(lib, "ThostApi/thostmduserapi_se.lib")
// #pragma comment(lib, "Site/CPP/ChnFutureSite/ThostApi/thostmduserapi_se.lib")

CThostMdApiWrapper::CThostMdApiWrapper(CDataCenter* data_center, IGuiDataAction* gui_action) : 
	CThostBaseWrapper(data_center, gui_action),
	md_api_(NULL), md_spi_handler_(NULL), connected_(false), logined_(false), login_times_(0), force_logout_(false),
	connect_timer_id_(100)
{
	logout_event_ = ::CreateEventW(NULL, FALSE, FALSE, L"MD_LOGOUT_EVENT");
}


CThostMdApiWrapper::~CThostMdApiWrapper()
{
	::CloseHandle(logout_event_);
}

void CThostMdApiWrapper::OnProcessMsg(CThostSpiMessage* msg)
{
	switch (msg->msg_type())
	{
	case SPI::OnMdFrontConnected:
	{
		OnFrontConnected(msg);
		break;
	}
	case SPI::OnMdFrontDisconnected:
	{
		OnFrontDisconnected(msg);
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
	case SPI::OnRspSubMarketData:
	{
		OnRspSubMarketData(msg);
		break;
	}
	case SPI::OnRspUnSubMarketData:
	{
		OnRspUnSubMarketData(msg);
		break;
	}
	case SPI::OnRtnDepthMarketData:
	{
		OnRtnDepthMarketData(msg);
		break;
	}

	default:
		break;
	}
}

void CThostMdApiWrapper::OnTimer(int timer_id)
{
	if (timer_id == connect_timer_id_) {
		ExitTimer(timer_id);
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_MdConnectTimeout, "行情服务器链接超时");
		}
		return;
	}
	
	auto it_timer = timer_id2instrument_.find(timer_id);
	if (it_timer != timer_id2instrument_.end()) {
		ExitTimer(timer_id);
		std::string instrument_id = it_timer->second;
		auto it_inst = instrument2timer_id_.find(instrument_id);
		ASSERT_TRUE(it_inst != instrument2timer_id_.end());
		auto it_subscr = subscribing_instruments_.find(instrument_id);
		ASSERT_TRUE(it_subscr != subscribing_instruments_.end());

		instrument2timer_id_.erase(it_inst);
		subscribing_instruments_.erase(it_subscr);
		timer_id2instrument_.erase(it_timer);

		std::stringstream ss;
		ss << broker_id() << "-" << user_id() << " 订阅合约超时, " + instrument_id;
		Utils::Log(ss.str());

		need_subscribe_instruments_.insert(instrument_id);
		CheckSubscribe();
	}
}

void CThostMdApiWrapper::Initialize(const std::string& broker_id,
	const std::string& user_id,
	const std::string& password, 
	const std::vector<std::string>& fronts)
{
	CreateThread();

	CThostBaseWrapper::Initialize(broker_id, user_id, password, fronts);
}

void CThostMdApiWrapper::Deinitialize()
{
	ExitThread();

	if (md_api_) {
		md_api_->RegisterSpi(NULL);
		md_api_->Release();
		md_api_ = NULL;
	}

	if (md_spi_handler_) {
		delete md_spi_handler_;
		md_spi_handler_ = NULL;
	}
}

void CThostMdApiWrapper::Login()
{
	force_logout_ = false;

	if (!connected_) {
		ReqConnect();
	}
	else {
		ReqUserLogin();
	}
}

void CThostMdApiWrapper::Logout()
{
	force_logout_ = true;

	if (logined_) {
		::ResetEvent(logout_event_);
		ReqUserLogout();
		::WaitForSingleObject(logout_event_, 10000);
	}
// 	else if (connected_) {
// 		if (md_api_) {
// 			md_api_->Release();
// 			md_api_ = NULL;
// 		}
// 		connected_ = false;
// 	}
}

void CThostMdApiWrapper::ReqConnect()
{
	if (md_api_) {
		md_api_->RegisterSpi(NULL);
		md_api_->Release();
		md_api_ = NULL;
	}

	if (md_spi_handler_) {
		delete md_spi_handler_;
		md_spi_handler_ = NULL;
	}

	std::string path = Utils::GetTempPath(user_id());
	md_api_ = CThostFtdcMdApi::CreateFtdcMdApi(path.c_str());
	md_spi_handler_ = new CThostMdSpiHandler(this);
	md_api_->RegisterSpi(md_spi_handler_);

	for (auto it = fronts_.begin(); it != fronts_.end(); it++) {
		md_api_->RegisterFront((char *)it->c_str());
	}
	md_api_->Init();

	CreateTimer(connect_timer_id_, MaxConnectTimeout);
}

void CThostMdApiWrapper::ReqUserLogin()
{
	CThostFtdcReqUserLoginField login_field;
	memset(&login_field, 0, sizeof(CThostFtdcReqUserLoginField));
	Utils::safe_strcpy(login_field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	Utils::safe_strcpy(login_field.UserID, user_id(), sizeof(TThostFtdcUserIDType));
	Utils::safe_strcpy(login_field.Password, password(), sizeof(TThostFtdcPasswordType));
	//version
	Utils::safe_strcpy(login_field.UserProductInfo, "", sizeof(TThostFtdcProductInfoType));
	int reqid = GetRequestId();
	md_api_->ReqUserLogin(&login_field, reqid);
}

void CThostMdApiWrapper::ReqUserLogout()
{
	CThostFtdcUserLogoutField logout_field;
	memset(&logout_field, 0, sizeof(CThostFtdcUserLogoutField));
	Utils::safe_strcpy(logout_field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	Utils::safe_strcpy(logout_field.UserID, user_id(), sizeof(TThostFtdcUserIDType));
	int reqid = GetRequestId();
	md_api_->ReqUserLogout(&logout_field, reqid);
}

void CThostMdApiWrapper::ReqSubscribeQuote(std::set<std::string> instruments)
{
	// 先取消订阅不需要的合约
	std::set<std::string> need_unsubscribes = allocated_instruments_;
	allocated_instruments_ = instruments;
	for (auto it_inst = allocated_instruments_.begin(); it_inst != allocated_instruments_.end(); it_inst++) {
		need_unsubscribes.erase(*it_inst);
	}
	Unsubscribe(need_unsubscribes);
	for (auto it_subscr = need_unsubscribes.begin(); it_subscr != need_unsubscribes.end(); it_subscr++) {
		need_subscribe_instruments_.erase(*it_subscr);
		// subscribed_instruments_.erase(*it_subscr);
	}

	for (auto it_inst = allocated_instruments_.begin(); it_inst != allocated_instruments_.end(); it_inst++) {
		auto it_subscr = subscribed_instruments_.find(*it_inst);
		if (it_subscr == subscribed_instruments_.end()) {
			need_subscribe_instruments_.insert(*it_inst);
		}
	}

	CheckSubscribe();
}

void CThostMdApiWrapper::CheckSubscribe()
{
	if (connected_ && logined_) {
		std::set<std::string> need_subscribes;
		std::set<std::string> tneed_subscribe_instruments = need_subscribe_instruments_;
		for (auto it = tneed_subscribe_instruments.begin(); it != tneed_subscribe_instruments.end(); it++) {
			auto it_sub = subscribing_instruments_.find(*it);
			if (it_sub == subscribing_instruments_.end()) {
				subscribing_instruments_.insert(*it);
				need_subscribes.insert(*it);
			}
			need_subscribe_instruments_.erase(*it);
		}
		Subscribe(need_subscribes);
	}
}

void CThostMdApiWrapper::Subscribe(const std::set<std::string>& instruments)
{
	if (instruments.size() < 1)
		return;

	char** instrument_ids = new char*[instruments.size()];
	for (auto it = instruments.begin(); it != instruments.end(); it++) {
		int index = std::distance(instruments.begin(), it);
		instrument_ids[index] = new char[it->length() + 1];
		Utils::safe_strcpy(instrument_ids[index], it->c_str(), it->length() + 1);

		int timer_id = GetTimerId();
		instrument2timer_id_.insert(std::make_pair(*it, timer_id));
		timer_id2instrument_.insert(std::make_pair(timer_id, *it));
		CreateTimer(timer_id, MaxRequestTimeout);
	}

	md_api_->SubscribeMarketData(instrument_ids, instruments.size());
	for (size_t idx = 0; idx < instruments.size(); idx++) {
		std::stringstream ss;
		ss << broker_id() << "-" << user_id() << " 正在订阅合约, " + std::string(instrument_ids[idx]);
		Utils::Log(ss.str());

		delete[] instrument_ids[idx];
	}
	delete[] instrument_ids;
}

void CThostMdApiWrapper::Unsubscribe(const std::set<std::string>& instruments)
{
	if (instruments.size() < 1)
		return;

	char** instrument_ids = new char*[instruments.size()];
	for (auto it = instruments.begin(); it != instruments.end(); it++) {
		int index = std::distance(instruments.begin(), it);
		instrument_ids[index] = new char[it->length() + 1];
		Utils::safe_strcpy(instrument_ids[index], it->c_str(), it->length() + 1);
	}

	md_api_->UnSubscribeMarketData(instrument_ids, instruments.size());
	for (size_t idx = 0; idx < instruments.size(); idx++) {
		std::stringstream ss;
		ss << broker_id() << "-" << user_id() << " 正在取消订阅合约, " + std::string(instrument_ids[idx]);
		Utils::Log(ss.str());

		delete[] instrument_ids[idx];
	}
	delete[] instrument_ids;
}

void CThostMdApiWrapper::OnFrontConnected(CThostSpiMessage* msg)
{
	ExitTimer(connect_timer_id_);
	connected_ = true;
	if (gui_action_) {
		gui_action_->OnLoginProcess(ApiEvent::ApiEvent_MdConnectSuccess, "行情服务器连接成功");
	}
	ReqUserLogin();
}

void CThostMdApiWrapper::OnFrontDisconnected(CThostSpiMessage* msg)
{
	connected_ = false;
	if (gui_action_) {
		gui_action_->OnLoginProcess(ApiEvent::ApiEvent_MdDisconnected, "交易服务器断链");
	}

	if (force_logout_) {
// 		if (md_api_) {
// 			md_api_->Release();
// 			md_api_ = NULL;
// 		}
		::SetEvent(logout_event_);
	}
}

void CThostMdApiWrapper::OnRspUserLogin(CThostSpiMessage* msg)
{
	if (msg->rsp_field()->ErrorID) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_MdLoginFailed,
				"登录行情服务器失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
	}
	else {
		CThostFtdcRspUserLoginField* f = msg->GetFieldPtr<CThostFtdcRspUserLoginField>();

		Utils::Log("登录行情服务器成功 >>");
		std::stringstream ss;
		ss << "交易日: " << f->TradingDay << std::endl
			<< "登录成功时间: " << f->LoginTime << std::endl
			<< "经纪公司代码: " << f->BrokerID << std::endl
			<< "用户代码: " << f->UserID << std::endl
			<< "前置编号: " << f->FrontID << std::endl
			<< "会话编号: " << f->SessionID << std::endl
			<< "最大报单引用: " << f->MaxOrderRef << std::endl;
		Utils::Log(ss.str());
		Utils::Log("登录行情服务器成功 <<");

		logined_ = true;
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_MdLoginSuccess, "登录行情服务器成功");
		}

		if (login_times_) { // 如果登录过，比如掉线重新连接的时候
			need_subscribe_instruments_.clear();
			subscribing_instruments_.clear();
			subscribed_instruments_.clear();

			for (auto it = allocated_instruments_.begin(); it != allocated_instruments_.end(); it++) {
				need_subscribe_instruments_.insert(*it);
			}
		}

		CheckSubscribe();
		login_times_++;
	}
}

void CThostMdApiWrapper::OnRspUserLogout(CThostSpiMessage* msg)
{
	logined_ = false;
	if (gui_action_) {
		gui_action_->OnLoginProcess(ApiEvent::ApiEvent_MdLogoutSuccess, "行情服务器注销");
	}

	if (force_logout_) {
// 		if (md_api_) {
// 			md_api_->Release();
// 			md_api_ = NULL;
// 		}
		::SetEvent(logout_event_);
	}
}

void CThostMdApiWrapper::OnRspSubMarketData(CThostSpiMessage* msg)
{
	CThostFtdcSpecificInstrumentField* f = msg->GetFieldPtr<CThostFtdcSpecificInstrumentField>();
	if (f) {
		std::stringstream ss;
		ss << broker_id() << "-" << user_id() << " 订阅合约成功, " << msg->rsp_field()->ErrorMsg;
		Utils::Log(ss.str());

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_SubscribeMarketData, f->InstrumentID);
		}

		auto it_inst = instrument2timer_id_.find(f->InstrumentID);
		if (it_inst != instrument2timer_id_.end()) {
			ExitTimer(it_inst->second);
			auto it_timer = timer_id2instrument_.find(it_inst->second);
			ASSERT_TRUE(it_timer != timer_id2instrument_.end());
			instrument2timer_id_.erase(it_inst);
			timer_id2instrument_.erase(it_timer);
		}

		auto it_subscr = subscribing_instruments_.find(f->InstrumentID);
		if (it_subscr != subscribing_instruments_.end()) {
			subscribing_instruments_.erase(it_subscr);
			subscribed_instruments_.insert(f->InstrumentID);
		}
	}
}

void CThostMdApiWrapper::OnRspUnSubMarketData(CThostSpiMessage* msg)
{
	CThostFtdcSpecificInstrumentField* f = msg->GetFieldPtr<CThostFtdcSpecificInstrumentField>();
	if (f) {
		std::stringstream ss;
		ss << broker_id() << "-" << user_id() << " 取消订阅合约成功, " << msg->rsp_field()->ErrorMsg;
		Utils::Log(ss.str());

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_UnsubscribeMarketData, f->InstrumentID);
		}

		auto it_subscr = subscribed_instruments_.find(f->InstrumentID);
		if (it_subscr != subscribed_instruments_.end()) {
			subscribed_instruments_.erase(it_subscr);
		}
	}
}

void CThostMdApiWrapper::OnRtnDepthMarketData(CThostSpiMessage* msg)
{
	CThostFtdcDepthMarketDataField* f = msg->GetFieldPtr<CThostFtdcDepthMarketDataField>();
	if (f) {
		Quote quote(*f);

		if (data_center_) {
			quote = data_center_->OnRtnQuote(quote);
		}

		if (quote.instrument_id.empty()) {
			return;
		}

		if (gui_action_) {
			std::set<Quote> quotes;
			quotes.insert(quote);
			gui_action_->RefreshQuotes(quotes);
		}
	}
}
