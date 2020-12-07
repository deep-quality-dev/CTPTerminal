#pragma once

#include "DataTypes/DataTypeDefs.h"
#include <map>

class CTimeRegular
{
public:
	CTimeRegular();
	~CTimeRegular();

	static CTimeRegular GetInstance();

	std::vector<TimeDuration> GetTimeDuration(const ExchangeID exchange_id, const std::string& product_id);
	bool WithIn(const ExchangeID exchange_id, const std::string& product_id, __time64_t timestamp);

	void AddTradingDuration(const ExchangeID exchange_id, const std::string& product_id, const TimeDuration& duration);

protected:
	void Init();
	bool WithIn(const TimeDuration& td, __time64_t timestamp);

private:
	std::map<std::pair<ExchangeID, std::string>, std::vector<TimeDuration>> trading_durations_;
};

