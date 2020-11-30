#include "stdafx.h"
#include <string.h>
#include "DataTypeDefs.h"
#include "Formatter.h"


CFormatter::CFormatter()
{
}


CFormatter::~CFormatter()
{
}

CFormatter CFormatter::GetInstance()
{
	static CFormatter _instance;
	return _instance;
}

void CFormatter::SetDataCenter(CDataCenter* data_center)
{
	if (data_center != NULL) {
		data_center_ = data_center;
	}
}

std::string CFormatter::Bool2string(bool bcondition)
{
	if (bcondition)
	{
		return "是";
	}
	return "否";
}

std::string CFormatter::Int2String(int tick)
{
	std::stringstream ss;
	ss << tick;
	return ss.str();
}

std::string CFormatter::Price2string(double Price, const std::string& Instrument)
{
// 	if (data_center_)
// 	{
// 		auto it = data_center_->m_Instruments.find(Instrument);
// 		if (it != data_center_->m_Instruments.end())
// 		{
// 			std::stringstream ss;
// 			ss.setf(std::ios_base::fixed);
// 			ss.precision(it->Decs);
// 			if (abs(Price - 0) <= 1e-4) Price = 0.0;
// 			ss << Price;
// 			return ss.str();
// 		}
// 	}
	return "";
}

std::string CFormatter::Double2string(double value, int dec)
{
	std::stringstream ss;
	ss.setf(std::ios_base::fixed);
	ss.precision(dec);
	ss << value;
	return ss.str();
}

std::string CFormatter::Direction2string(Direction direction)
{
	std::string str;
	if (direction == Buy)
	{
		str = "买";
	}
	else if (direction == Sell)
	{
		str = "  卖";
	}
	else
	{
		str = "未知";
	}
	return str;
}

std::string CFormatter::OffsetFlag2string(OffsetFlag offsetflag)
{
	if (offsetflag == Open)
	{
		return "开仓";
	}
	else if (offsetflag == Close)
	{
		return "平仓";
	}
	else if (offsetflag == CloseToday)
	{
		return "平今";
	}
	else if (offsetflag == CloseYestoday)
	{
		return "平昨";
	}
	else if (offsetflag == ForceClose)
	{
		return "强平";
	}
	else
	{
		return "未知";
	}
}

std::string CFormatter::Status2String(OrderStatus status)
{
	switch (status)
	{
	case Status_AllTraded:
		return "全部成交";
	case Status_Canceled:
		return "已撤单";
	case Status_PartTraded:
		return "部分成交";
	case Status_UnTraded:
		return "未成交";
	case Status_NotTouched:
		return "未触发";
	case Status_Touched:
		return "已触发";
	case Status_Error:
		return "错误";
	default:
		return "未知";
	}
}

std::string CFormatter::ExchangeID2string(ExchangeID exchangeid)
{
	if (exchangeid == DCE)
	{
		return "大商所";
	}
	else if (exchangeid == CZCE)
	{
		return "郑交所";
	}
	else if (exchangeid == CFFEX)
	{
		return "中金所";
	}
	else if (exchangeid == SHFE)
	{
		return "上期所";
	}
	else
	{
		return "未知";
	}
}
