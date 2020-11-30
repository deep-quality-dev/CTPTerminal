#include "stdafx.h"
#include "ThostMdApiWrapper.h"
#include "ThostSpiHandler.h"
#include "DataCenter.h"
#include "DataTypes/GuiDataAction.h"
#include "Utils/Utils.h"


CThostMdApiWrapper::CThostMdApiWrapper(CDataCenter* data_center, IGuiDataAction* gui_action) : 
	CThostBaseWrapper(data_center, gui_action),
	md_api_(NULL), connected_(false), logined_(false), login_times_(0), 
	connect_timer_id_(100)
{
}


CThostMdApiWrapper::~CThostMdApiWrapper()
{
}

void CThostMdApiWrapper::OnProcessMsg(std::shared_ptr<CThostSpiMessage> msg)
{
	switch (msg->msg_type())
	{
	case SPI::OnMdFrontConnected:
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
			gui_action_->OnLoginProcess(ApiEvent_ConnectTimeout);
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

		need_subscribe_instruments_.insert(instrument_id);
		CheckSubscribe();
	}
}

void CThostMdApiWrapper::Initialize(const std::string& broker_id, const std::string& user_id, const std::string& password, const std::vector<std::string> fronts)
{
	CThostBaseWrapper::Initialize(broker_id, user_id, password, fronts);

	if (!connected_) {
		ReqConnect();
	}
	else {
		ReqUserLogin();
	}
}

void CThostMdApiWrapper::ReqConnect()
{
	if (md_api_) {
		md_api_->Release();
	}

	std::string path = GetTempPath(user_id());
	md_api_ = CThostFtdcMdApi::CreateFtdcMdApi(path.c_str());
	md_spi_handler_ = std::shared_ptr<CThostMdSpiHandler>(new CThostMdSpiHandler(this));
	md_api_->RegisterSpi(md_spi_handler_.get());

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
	safe_strcpy(login_field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	safe_strcpy(login_field.UserID, user_id(), sizeof(TThostFtdcUserIDType));
	safe_strcpy(login_field.Password, password(), sizeof(TThostFtdcPasswordType));
	//version
	safe_strcpy(login_field.UserProductInfo, "EasyTrader", sizeof(TThostFtdcProductInfoType));
	int reqid = GetRequestId();
	md_api_->ReqUserLogin(&login_field, reqid);
}

void CThostMdApiWrapper::ReqSubscribeQuote(std::set<std::string> instruments)
{
	allocated_instruments_ = instruments;
	CheckSubscribe();
}

void CThostMdApiWrapper::CheckSubscribe()
{
	if (connected_ && logined_) {
		std::set<std::string> need_subscribes;
		for (auto it = need_subscribe_instruments_.begin(); it != need_subscribe_instruments_.end(); it++) {
			auto it_sub = subscribing_instruments_.find(*it);
			if (it_sub == subscribing_instruments_.end()) {
				subscribing_instruments_.insert(*it);
				need_subscribes.insert(*it);
			}
			need_subscribe_instruments_.erase(it);
		}
		Subscribe(need_subscribes);
	}
}

void CThostMdApiWrapper::Subscribe(std::set<std::string> instruments)
{
	if (instruments.size() < 1)
		return;

	char** instrument_ids = new char*[instruments.size()];
	for (auto it = instruments.begin(); it != instruments.end(); it++) {
		int index = std::distance(instruments.begin(), it);
		instrument_ids[index] = new char[it->length() + 1];
		safe_strcpy(instrument_ids[index], it->c_str(), it->length() + 1);

		int timer_id = GetTimerId();
		instrument2timer_id_.insert(std::make_pair(*it, timer_id));
		timer_id2instrument_.insert(std::make_pair(timer_id, *it));
		CreateTimer(timer_id, MaxRequestTimeout);
	}

	md_api_->SubscribeMarketData(instrument_ids, instruments.size());
	for (size_t idx = 0; idx < instruments.size(); idx++) {
		delete[] instrument_ids[idx];
	}
	delete[] instrument_ids;
}

void CThostMdApiWrapper::OnRspUserLogin(std::shared_ptr<CThostSpiMessage> msg)
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

void CThostMdApiWrapper::OnRspUserLogout(std::shared_ptr<CThostSpiMessage> msg)
{

}

void CThostMdApiWrapper::OnRspSubMarketData(std::shared_ptr<CThostSpiMessage> msg)
{
	CThostFtdcSpecificInstrumentField* f = msg->GetFieldPtr<CThostFtdcSpecificInstrumentField>();
	if (f) {
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

void CThostMdApiWrapper::OnRspUnSubMarketData(std::shared_ptr<CThostSpiMessage> msg)
{

}

void CThostMdApiWrapper::OnRtnDepthMarketData(std::shared_ptr<CThostSpiMessage> msg)
{
	CThostFtdcDepthMarketDataField* f = msg->GetFieldPtr<CThostFtdcDepthMarketDataField>();
	if (f) {
		Quote quote(*f);

		if (data_center_) {
			data_center_->OnRtnQuote(quote);
		}

		if (gui_action_) {
			std::set<Quote> quotes;
			quotes.insert(quote);
			gui_action_->RefreshQuotes(quotes);
		}
	}
}
