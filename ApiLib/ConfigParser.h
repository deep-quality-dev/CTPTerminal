#pragma once

#include <vector>
#include "DataTypes/DataTypeDefs.h"

class CConfigParser
{
public:
	static void LoadServerConfig(const char* path);

	static std::vector<MarketDataServerConfig> market_data_server_config() {
		return quote_server_configs_;
	}

	static TradeServerConfig trade_server_config() {
		return trade_server_config_;
	}

	static std::string main_instrument_id() {
		return main_instrument_id_;
	}

	static std::string sub_instrument_id() {
		return sub_instrument_id_;
	}

	static int ma_period() {
		return ma_period_;
	}

	static int volume() {
		return volume_;
	}

private:
	static std::vector<MarketDataServerConfig> quote_server_configs_;
	static TradeServerConfig trade_server_config_;

	static std::string main_instrument_id_;
	static std::string sub_instrument_id_;
	
	static int ma_period_;
	static int volume_;
};
