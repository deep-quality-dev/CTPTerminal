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

	static int order_limit() {
		return order_limit_;
	}

	static void set_order_limit(int limit) {
		order_limit_ = limit;
	}

	static double loss_limit() {
		return loss_limit_;
	}

	static void set_loss_limit(double limit) {
		loss_limit_ = limit;
	}

	static int order_interval() {
		return order_interval_;
	}

	static void set_order_interval(int interval) {
		order_interval_ = interval;
	}

	static int price_offset() {
		return price_offset_;
	}

private:
	static std::vector<MarketDataServerConfig> quote_server_configs_;
	static TradeServerConfig trade_server_config_;

	static std::string main_instrument_id_;
	static std::string sub_instrument_id_;
	
	static int ma_period_;
	static int volume_;

	// 当天报单次数限制
	static int order_limit_;

	// 资金管理, %
	static double loss_limit_;

	// 报单之间时间间隔, 单位为秒
	static int order_interval_;

	static int price_offset_;
};
