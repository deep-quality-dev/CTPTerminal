#pragma once

#include <sstream>

class CDataCenter;
class CFormatter
{
public:
	CFormatter();
	~CFormatter();

	static CFormatter GetInstance();

	void SetDataCenter(CDataCenter* data_center);

	template<class T>
	std::string Value2String(T value)
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	std::string Bool2string(bool bcondition);
	std::string Int2String(int tick);
	std::string Price2string(double Price, const std::string& Instrument);
	std::string Double2string(double value, int dec);
	std::string Direction2string(Direction direction);
	std::string OffsetFlag2string(OffsetFlag offsetflag);
	std::string Status2String(OrderStatus status);
	std::string ExchangeID2string(ExchangeID exchangeid);

private:
	CDataCenter* data_center_;
};

