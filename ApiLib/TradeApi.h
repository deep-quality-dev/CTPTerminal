#pragma once

#include "DataTypes/DataTypeDefs.h"
#include <vector>

class ITradeApi
{
public:
	virtual void Initialize(const std::string& broker_id,
		const std::string& user_id,
		const std::string& password,
		const std::vector<std::string> fronts,
		const std::string& user_product_info,
		const std::string& auth_code,
		const std::string& app_id) = 0;
};