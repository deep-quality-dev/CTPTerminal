#include "stdafx.h"
#include "DataCenter.h"
#include "TradeApi.h"
#include "Strategy.h"
#include "DataTypes/Formatter.h"
#include "Utils/Utils.h"
#include "Utils/Logger.h"

void IStrategy::OnQuoteCallback(const Quote& quote)
{

}

void IStrategy::OnOrderCallback(const Order& order)
{
	Utils::Log("IStrategy::OnOrderCallback >>" + order.order_sys_id + ", " + CFormatter::GetInstance().Status2String(order.status));

	{
		std::lock_guard<std::mutex> lk(mutex_alive_orders_);
		alive_orders_[order.GetKey().order_ref] = order;
		for (auto it_order = alive_orders_.begin(); it_order != alive_orders_.end();) {
			if (it_order->second.status == OrderStatus::Status_Error ||
				it_order->second.status == OrderStatus::Status_Canceled ||
				it_order->second.status == OrderStatus::Status_AllTraded) {

				auto it_orderref = order_ref2timer_id_.find(it_order->first);
				if (it_orderref != order_ref2timer_id_.end()) {
					ExitTimer(it_orderref->second);
					auto it_timer = timer_id2order_ref_.find(it_orderref->second);
					ASSERT_TRUE(it_timer != timer_id2order_ref_.end());
					order_ref2timer_id_.erase(it_orderref);
					timer_id2order_ref_.erase(it_timer);
				}
				alive_orders_.erase(it_order++);
			}
			else {
				it_order++;
			}
		}
	}

	if (order.status == Status_Error || order.status == Status_Canceled) {
		if (order.volume_remained > 0) {
			Order remain_order = order;
			if (order.offset_flag == OffsetFlag::Open) {}
			else {
				int volume = order.direction == Direction::Sell ? HasBuyPosition(order.instrument_id) : HasSellPosition(order.instrument_id);
				remain_order.volume_remained = order.volume_remained > volume ? volume : order.volume_remained;
			}

			if (remain_order.volume_remained > 0) {
				// 如果市场价报单之后2秒(MaxMarketOrderTimeout)之前未成交或者部分成交，
				// 自动会被撤单，剩下的重新报单。
				int ret = InsertMarketOrder(remain_order.instrument_id, remain_order.offset_flag, remain_order.direction, remain_order.volume_remained);
				std::cout << "InsertMarektOrder : " << ret << std::endl;
			}
		}
	}
}

void IStrategy::OnTradeCallback(int order_ref, const Trade& trade)
{

}

void IStrategy::OnQryOrderCallback(const std::set<Order>& orders)
{
	std::lock_guard<std::mutex> lk(mutex_alive_orders_);

	alive_orders_.clear();
	for (auto it_order = orders.begin(); it_order != orders.end(); it_order++) {
		if (it_order->status == OrderStatus::Status_UnTraded || it_order->status == OrderStatus::Status_PartTraded) {
			alive_orders_[it_order->key.order_ref] = *it_order;
		}
	}
}

void IStrategy::OnTradeAccountCallback(const TradingAccount& account)
{

}

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
	order_insert.is_market_order = false;

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

			// 如果2秒之内没有全部成交，立即撤单
			int timer_id = ++timer_id_;
			order_ref2timer_id_.insert(std::make_pair(order_insert.order_ref, timer_id));
			timer_id2order_ref_.insert(std::make_pair(timer_id, order_insert.order_ref));
			CreateTimer(timer_id, MaxMarketOrderTimeout);

			return order_insert.order_ref;
		}
	}
	return -1;
}

int IStrategy::CancelOrder(const Order& order)
{
	if (trade_api_) {
		return trade_api_->ReqCancelOrder(order);
	}
	return -1;
}

void IStrategy::OnProcessMsg(EmptyMessage* msg)
{

}

void IStrategy::OnTimer(int timer_id)
{
	auto it_timer = timer_id2order_ref_.find(timer_id);
	if (it_timer != timer_id2order_ref_.end()) {
		ExitTimer(timer_id);

		int order_ref = it_timer->second;
		auto it_alive = alive_orders_.find(order_ref);
		if (it_alive != alive_orders_.end()) {
			CancelOrder(it_alive->second);
		}

		auto it_orderref = order_ref2timer_id_.find(it_timer->second);
		ASSERT_TRUE(it_orderref != order_ref2timer_id_.end());
		order_ref2timer_id_.erase(it_orderref);
		timer_id2order_ref_.erase(it_timer);
	}
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
