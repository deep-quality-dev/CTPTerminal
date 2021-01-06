#include "stdafx.h"
#include "DataCenter.h"
#include "TradeApi.h"
#include "Strategy.h"
#include "Utils/Utils.h"

int IStrategy::InsertOrder(const std::string& instrument_id, OffsetFlag offset_flag, Direction direction, double price, int volume)
{
	__time64_t now_time = Utils::GetTimestamp();
	if (now_time - last_order_time_ < order_interval_ * 1000) {
		return -2;
	}

	OrderInsert order_insert;
	order_insert.instrument_id = instrument_id;
	order_insert.offset_flag = offset_flag;
	order_insert.direction = direction;
	order_insert.limit_price = price;
	order_insert.volume = volume;
	order_insert.order_ref = ++order_ref_;

	if (trade_api_) {
		if (trade_api_->ReqInsertOrder(order_insert) >= 0) {
			last_order_time_ = now_time;
			return order_insert.order_ref;
		}
	}
	return -1;
}

int IStrategy::InsertMarketOrder(const std::string& instrument_id, OffsetFlag offset_flag, Direction direction, int volume)
{
	__time64_t now_time = Utils::GetTimestamp();
	if (now_time - last_order_time_ < order_interval_ * 1000) {
		return -2;
	}

	// NOTE: 上期所不支持市价报单

	OrderInsert order_insert;
	order_insert.instrument_id = instrument_id;
	order_insert.offset_flag = offset_flag;
	order_insert.direction = direction;
	order_insert.is_market_order = false;
	order_insert.limit_price = data_center()->GetMarketPrice(instrument_id, direction);
	order_insert.volume = volume;
	order_insert.order_ref = ++order_ref_;

	if (trade_api_) {
		if (trade_api_->ReqInsertOrder(order_insert) >= 0) {
			last_order_time_ = now_time;
			return order_insert.order_ref;
		}
	}
	return -1;
}

int IStrategy::HasSellPosition(const std::string& instrument_id)
{
	std::set<Position>& positions = data_center_->positions();
	for (auto it_position = positions.begin(); it_position != positions.end(); it_position++) {
		if (it_position->instrument_id == instrument_id) {
			if (it_position->direction == Direction::Sell) {
				return it_position->volume();
			}
		}
	}
	return 0;
}

int IStrategy::HasBuyPosition(const std::string& instrument_id)
{
	std::set<Position>& positions = data_center_->positions();
	for (auto it_position = positions.begin(); it_position != positions.end(); it_position++) {
		if (it_position->instrument_id == instrument_id) {
			if (it_position->direction == Direction::Buy) {
				return it_position->volume();
			}
		}
	}
	return 0;
}
