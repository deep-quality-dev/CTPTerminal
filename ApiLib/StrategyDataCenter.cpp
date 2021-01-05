#include "stdafx.h"
#include "StrategyDataCenter.h"
#include "ArbitrageStrategy.h"


CStrategyDataCenter::CStrategyDataCenter()
{
}


CStrategyDataCenter::~CStrategyDataCenter()
{
}

void CStrategyDataCenter::AddStrategy(const std::string& instrument_id, IStrategy* strategy)
{
	strategy_map_[instrument_id] = strategy;
	strategy->set_data_center(this);
}

void CStrategyDataCenter::RemoveStrategy(const std::string& instrument_id)
{
	strategy_map_.erase(instrument_id);
}

Quote CStrategyDataCenter::OnRtnQuote(const Quote& quote)
{
	for (auto it_strategy = strategy_map_.begin(); it_strategy != strategy_map_.end(); it_strategy++) {
		if (it_strategy->first == quote.instrument_id) {
			if (it_strategy->second) {
				it_strategy->second->OnQuoteCallback(quote);
			}
		}
	}

	return quote;
}
