#pragma once

#include <vector>
#include "DataTypes/DataTypeDefs.h"

class CConfigParser
{
public:
	static std::vector<MarketDataServerConfig> market_data_server_config() {
		return quote_server_configs_;
	}

	static TradeServerConfig trade_server_config() {
		return trade_server_config_;
	}

	static void LoadServerConfig(const char* path);

private:
	static std::vector<MarketDataServerConfig> quote_server_configs_;
	static TradeServerConfig trade_server_config_;
};
