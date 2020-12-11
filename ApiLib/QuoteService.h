#pragma once

#include "DataCenter.h"
#include "DataTypes/GuiDataAction.h"
#include "MarketDataApi.h"
#include "TradeApi.h"
#include <vector>
#include <memory>

// class IMarketDataApi;
// class ITradeApi;

class CQuoteService
{
public:
	CQuoteService();
	~CQuoteService();

	CDataCenter* data_center() {
		return data_center_.get();
	}
	IGuiDataAction* gui_action() {
		return gui_action_.get();
	}
	ITradeApi* trade_api() {
		return trade_api_.get();
	}

	void LoadServerConfig();
	void Initialize();
	void SetInstruments(const std::set<Instrument>& instruments);

	void SetSubscribeProducts(const std::set<std::string>& products);
	void SetSubscribeInstruments(const std::set<std::string>& instruments);

protected:
	void InitMdWrappers();
	void InitTradeWrappers();

	void CheckSubscribes();

private:
	bool inited_trader_, inited_md_;

	std::shared_ptr<CDataCenter> data_center_;
	std::shared_ptr<IGuiDataAction> gui_action_;

	struct ServerConfig
	{
		std::string broker_id;	// 经纪公司代码
		std::vector<std::string> addresses;
		std::string user_id;	// 用户代码
		std::string password;
	};
	struct TradeServerConfig : public ServerConfig
	{
		std::string user_product_info; // 用户端产品信息
		std::string auth_code;	// 认证码
		std::string app_id;		// App代码
	};

	std::vector<ServerConfig> quote_server_configs_;
	TradeServerConfig trade_server_config_;

	std::set<std::shared_ptr<IMarketDataApi>> md_apis_;
	std::shared_ptr<ITradeApi> trade_api_;

	std::set<Instrument> instruments_;
	std::set<std::string> subscribe_products_;
	std::set<std::string> subscribe_instruments_;
};

