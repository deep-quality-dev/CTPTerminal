#pragma once

#include "DataTypes/DataTypeDefs.h"
#include <vector>

class IMarketDataApi
{
public:
	virtual void Initialize(const std::string& broker_id,
		const std::string& user_id,
		const std::string& password,
		const std::vector<std::string> fronts) = 0;

	virtual void ReqSubscribeQuote(std::set<std::string> instruments) = 0;
};