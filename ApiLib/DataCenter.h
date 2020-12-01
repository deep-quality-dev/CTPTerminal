#pragma once

#include "DataTypes/DataTypeDefs.h"
#include <map>
#include <set>

class CDataCenter
{
public:
	CDataCenter();
	~CDataCenter();

	void OnRtnInstruments(const std::set<Instrument>& instruments);
	void OnRtnQuote(const Quote& quote);

protected:
	void SaveQuote(const Quote& quote);

protected:
	std::set<Instrument> instruments_;
	std::map<std::string, QuoteDeque> quotes_;

	__time64_t server_time_;
};

