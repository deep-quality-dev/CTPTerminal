#pragma once

#include "DataCenter.h"
#include "DataTypes/GuiDataAction.h"
#include <vector>
#include <memory>

class IMarketDataApi;
class ITradeApi;

class CQuoteService
{
public:
	CQuoteService();
	~CQuoteService();

	void LoadServerConfig();
	void Initialize();
	void SetSubscribeProducts(const std::set<std::string>& products);
	void SetInstruments(const std::set<Instrument>& instruments);

protected:
	void InitMdWrappers();
	void InitTradeWrappers();

	void CheckSubscribeProducts();

private:
	bool inited_trader_, inited_md_;

	std::shared_ptr<CDataCenter> data_center_;
	std::shared_ptr<IGuiDataAction> gui_action_;

	std::vector<ServerConfig> quote_server_configs_;
	ServerConfig trade_server_config_;

	std::set<std::shared_ptr<IMarketDataApi>> md_apis_;
	std::shared_ptr<ITradeApi> trade_api_;

	std::set<std::string> products_;
	std::set<Instrument> instruments_;
};

