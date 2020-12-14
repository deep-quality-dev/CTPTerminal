#include "stdafx.h"
#include "ThostTradeApiWrapper.h"
#include "ThostSpiHandler.h"
#include "QryManager.h"
#include "DataCenter.h"
#include "DataTypes/GuiDataAction.h"
#include "DataTypes/FtdcTranslator.h"
#include "Utils/Utils.h"
#include "Utils/Logger.h"
#include <sstream>


CThostTradeApiWrapper::CThostTradeApiWrapper(CDataCenter* data_center, IGuiDataAction* gui_action) :
	CThostBaseWrapper(data_center, gui_action),
	trader_api_(NULL), connected_(false), authenticated_(false), logined_(false), login_times_(0),
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
		OnRspConnected(msg);
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
			gui_action_->OnLoginProcess(ApiEvent_ConnectTimeout, "交易服务器链接超时");
		}
		return;
	}
}

void CThostTradeApiWrapper::Initialize(const std::string& broker_id, 
	const std::string& user_id, const std::string& password, const std::vector<std::string> fronts,
	const std::string& user_product_info,
	const std::string& auth_code,
	const std::string& app_id)
{
	CThostBaseWrapper::Initialize(broker_id, user_id, password, fronts);
	user_product_info_ = user_product_info;
	auth_code_ = auth_code;
	app_id_ = app_id;

	if (app_id_.length() < 1 || auth_code.length() < 1) {
		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_AuthenticationFailed, "交易终端认证失败");
		}
		return;
	}

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

void CThostTradeApiWrapper::ReqAuthenticate()
{
	CThostFtdcReqAuthenticateField field = { 0 };
	safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	safe_strcpy(field.UserID, user_id(), sizeof(TThostFtdcUserIDType));
	safe_strcpy(field.UserProductInfo, user_product_info_.c_str(), sizeof(TThostFtdcProductInfoType));
	safe_strcpy(field.AuthCode, auth_code_.c_str(), sizeof(TThostFtdcAuthCodeType));
	safe_strcpy(field.AppID, app_id_.c_str(), sizeof(TThostFtdcAppIDType));

	int reqid = GetRequestId();
	trader_api_->ReqAuthenticate(&field, reqid);
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
	orders_cache_.clear();
	return reqid;
}

int CThostTradeApiWrapper::ReqQryTrade()
{
	CThostFtdcQryTradeField field;
	memset(&field, 0, sizeof(CThostFtdcQryTradeField));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	trader_api_->ReqQryTrade(&field, reqid);
	trades_cache_.clear();
	return reqid;
}

int CThostTradeApiWrapper::ReqQryPosition()
{
	CThostFtdcQryInvestorPositionField field;
	memset(&field, 0, sizeof(CThostFtdcQryInvestorPositionField));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	trader_api_->ReqQryInvestorPosition(&field, reqid);
	positions_cache_.clear();
	return reqid;
}

int CThostTradeApiWrapper::ReqQryPositionDetail()
{
	CThostFtdcQryInvestorPositionDetailField field;
	memset(&field, 0, sizeof(CThostFtdcQryInvestorPositionDetailField));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	int reqid = GetRequestId();
	trader_api_->ReqQryInvestorPositionDetail(&field, reqid);
	position_details_cache_.clear();
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

void CThostTradeApiWrapper::ReqInsertOrder(const OrderInsert& order_insert)
{
	CThostFtdcInputOrderField field;
	memset(&field, 0, sizeof(CThostFtdcInputOrderField));
	safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	safe_strcpy(field.InstrumentID, order_insert.instrument_id.c_str(), sizeof(TThostFtdcInstrumentIDType));
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
	safe_strcpy(field.OrderRef, ss.str().c_str(), sizeof(TThostFtdcOrderRefType));
	field.RequestID = GetRequestId();
	trader_api_->ReqOrderInsert(&field, field.RequestID);
}

void CThostTradeApiWrapper::ReqQryMarginRate(const std::string& instrument_id)
{
	if (querying_margin_rates_.find(instrument_id) == querying_margin_rates_.end() &&
		ready_margin_rates_.find(instrument_id) == ready_margin_rates_.end()) {
		qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQryInstrumentMarginRate, this, instrument_id), "查询合约");
	}
}

void CThostTradeApiWrapper::ReqCancelOrder(const Order& order)
{
	CThostFtdcInputOrderActionField	field;
	memset(&field, 0, sizeof(CThostFtdcInputOrderActionField));
	safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	field.ActionFlag = THOST_FTDC_AF_Delete;		//ActionFlag;
	field.FrontID = front_id();
	field.SessionID = session_id();
	safe_strcpy(field.OrderSysID, order.order_sys_id.c_str(), sizeof(TThostFtdcOrderSysIDType));
}

int CThostTradeApiWrapper::ReqQryInstrumentMarginRate(const std::string& instrument_id)
{
	Utils::Log("ReqQryInstrumentMarginRate << " + instrument_id);

	CThostFtdcQryInstrumentMarginRateField field;
	memset(&field, 0, sizeof(CThostFtdcQryInstrumentMarginRateField));
	safe_strcpy(field.BrokerID, broker_id(), sizeof(TThostFtdcBrokerIDType));
	safe_strcpy(field.InvestorID, user_id(), sizeof(TThostFtdcInvestorIDType));
	safe_strcpy(field.InstrumentID, instrument_id.c_str(), sizeof(TThostFtdcInstrumentIDType));
	//speculation
	field.HedgeFlag = '1';
	int reqid = GetRequestId();
	trader_api_->ReqQryInstrumentMarginRate(&field, reqid);
	return reqid;
}

void CThostTradeApiWrapper::OnRspConnected(CThostSpiMessage* msg)
{
	ExitTimer(connect_timer_id_);
	connected_ = true;
	if (gui_action_) {
		gui_action_->OnLoginProcess(ApiEvent::ApiEvent_ConnectSuccess, "交易服务器链接成功");
	}
	ReqAuthenticate();
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
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_LoginFailed,
				"交易服务器登录失败",
				msg->rsp_field()->ErrorID,
				msg->rsp_field()->ErrorMsg);
		}
	}
	else {
		CThostFtdcRspUserLoginField* f = msg->GetFieldPtr<CThostFtdcRspUserLoginField>();

		front_id_ = f->FrontID;
		session_id_ = f->SessionID;
		order_ref_ = atoi(f->MaxOrderRef);
		if (data_center_) {
			data_center_->SetOrderRef(order_ref_);
		}

		logined_ = true;

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent::ApiEvent_LoginSuccess, "交易服务器登录成功");
		}

		if (!login_times_) { // 第一次登录
			qry_manager_->AddQuery(std::bind(&CThostTradeApiWrapper::ReqQryAllInstrument, this), "查询合约");
		}
	}
}

void CThostTradeApiWrapper::OnRspUserLogout(CThostSpiMessage* msg)
{

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

					std::set<Position> positions = data_center_->positions();
					if (gui_action_) {
						gui_action_->RefreshPositions(positions);
					}
				}

				if (gui_action_) {
					gui_action_->RefreshTrade(trade);
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

		if (gui_action_) {
			gui_action_->RefreshTrade(trade);
		}

		bool current_session = true;
		auto it_order = sysid2orderkey_.find(trade.order_sys_id);
		if (it_order != sysid2orderkey_.end()) {
			if (it_order->second.front_id != front_id() || it_order->second.session_id != session_id()) {
				current_session = false;
			}

			ASSERT_TRUE(atoi(f->OrderRef) == it_order->second.order_ref);
			if (data_center_) {
				data_center_->OnRtnTrade(trade);

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
			if (it_position != positions_cache_.end()) {
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

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryPositionSuccess,
				("查询持仓成功, " + position.instrument_id).c_str());
		}
	}

	if (msg->is_last()) {
		if (data_center_) {
			data_center_->OnRtnPositions(positions_cache_);
		}

		if (gui_action_) {
			gui_action_->RefreshPositions(positions_cache_);
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

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryInstrumentSuccess, ("查询合约成功, " + inst.instrument_id).c_str());
		}
	}

	if (msg->is_last()) {
		if (data_center_) {
			data_center_->OnRtnInstruments(instruments_cache_);
		}

		if (gui_action_) {
			gui_action_->RefreshInstruments(instruments_cache_);
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
		PositionDetail position(*f);
		position_details_cache_.insert(position);

		if (gui_action_) {
			gui_action_->OnLoginProcess(ApiEvent_QryPositionDetailSuccess,
				("查询持仓成功, " + position.instrument_id).c_str());
		}
	}

	if (msg->is_last()) {
		if (data_center_) {
			data_center_->OnRtnPositionDetails(position_details_cache_);
		}

		if (gui_action_) {
			gui_action_->RefreshPositionDetails(position_details_cache_);
		}
		position_details_cache_.clear();
	}
}
