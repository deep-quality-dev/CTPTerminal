#pragma once

#include "DataCenter.h"
#include <map>

class IStrategy;

class CStrategyDataCenter :
	public CDataCenter
{
public:
	CStrategyDataCenter();
	~CStrategyDataCenter();

	void AddStrategy(const std::string& instrument_id, IStrategy* strategy);
	void RemoveStrategy(const std::string& instrument_id);

	Quote OnRtnQuote(const Quote& quote);

private:
	std::map<std::string, IStrategy*> strategy_map_;
};

