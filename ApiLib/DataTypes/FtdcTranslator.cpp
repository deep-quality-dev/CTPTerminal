#include "stdafx.h"
#include "FtdcTranslator.h"
#include "Utils/Utils.h"
#include "Utils/Fault.h"

void SetFromFtdcOrderStatus(OrderStatus& order_status, const TThostFtdcOrderStatusType& ftdc_status)
{
	switch (ftdc_status)
	{
	case THOST_FTDC_OST_AllTraded:
		order_status = Status_AllTraded;
		break;
	case THOST_FTDC_OST_PartTradedQueueing:
	case THOST_FTDC_OST_PartTradedNotQueueing:
		order_status = Status_PartTraded;
		break;
	case THOST_FTDC_OST_NoTradeQueueing:
	case THOST_FTDC_OST_NoTradeNotQueueing:
		order_status = Status_UnTraded;
		break;
	case THOST_FTDC_OST_Canceled:
		order_status = Status_Canceled;
		break;
	case THOST_FTDC_OST_NotTouched:
		order_status = Status_NotTouched;
		break;
	case THOST_FTDC_OST_Touched:
		order_status = Status_Touched;
		break;
	case THOST_FTDC_OST_Unknown:
	default:
		order_status = Status_Unknown;
	}
}

void SetFromFtdcOffsetflag(OffsetFlag& offsetFlag, const char& c)
{
	switch (c)
	{
	case THOST_FTDC_OF_Open:
		offsetFlag = Open;
		break;
	case THOST_FTDC_OF_Close:
		offsetFlag = Close;
		break;
	case THOST_FTDC_OF_ForceClose:
		offsetFlag = ForceClose;
		break;
	case THOST_FTDC_OF_CloseToday:
		offsetFlag = CloseToday;
		break;
	case THOST_FTDC_OF_CloseYesterday:
		offsetFlag = CloseYestoday;
		break;
	default:
		ASSERT_TRUE(false);
	}
}

void SetFromFtdcExchangeID(ExchangeID& exchangeid, const TThostFtdcExchangeIDType& ftdc_exchangeid)
{
	exchangeid = UNKNOWN;
	if (ftdc_exchangeid == std::string("SHFE")) {
		exchangeid = SHFE;
	}
	else if (ftdc_exchangeid == std::string("CZCE")) {
		exchangeid = CZCE;
	}
	else if (ftdc_exchangeid == std::string("DCE")) {
		exchangeid = DCE;
	}
	else if (ftdc_exchangeid == std::string("CFFEX")) {
		exchangeid = CFFEX;
	}
}

void SetFromFtdcPosiDirection(Direction& direction, const TThostFtdcPosiDirectionType& ftdc_direction)
{
	switch (ftdc_direction)
	{
	case THOST_FTDC_PD_Long:
		direction = Buy;
		break;
	case THOST_FTDC_PD_Short:
		direction = Sell;
		break;
	default:
		break;
	}
}

void SetFromFtdcDirection(Direction& direction, const TThostFtdcDirectionType& ftdc_direction)
{
	switch (ftdc_direction)
	{
	case THOST_FTDC_D_Buy:
		direction = Buy;
		break;
	case THOST_FTDC_D_Sell:
		direction = Sell;
		break;
	default:
		ASSERT_TRUE(false);
	}
}

void GetFromTickplusDirection(TThostFtdcDirectionType& ftdc, const Direction& direction)
{
	if (direction == Buy) {
		ftdc = '0';
	}
	else {
		ftdc = '1';
	}
}

void GetFromTickplusOffsetFlag(TThostFtdcCombOffsetFlagType& ftdc, const OffsetFlag& flag)
{
	if (flag == Open) {
		ftdc[0] = THOST_FTDC_OF_Open;
	}
	else if (flag == Close) {
		ftdc[0] = THOST_FTDC_OF_Close;
	}
	else if (flag == CloseToday) {
		ftdc[0] = THOST_FTDC_OF_CloseToday;
	}
	else if (flag == CloseYestoday) {
		ftdc[0] = THOST_FTDC_OF_CloseYesterday;
	}
	else {
		ftdc[0] = THOST_FTDC_OF_Open;
	}
}

void GetFromTickplusExchangeID(TThostFtdcExchangeIDType& ftdc, const ExchangeID& exchangeid)
{
	switch (exchangeid)
	{
	case CZCE:
		Utils::safe_strcpy(ftdc, "CZCE", sizeof(TThostFtdcExchangeIDType));
		break;
	case DCE:
		Utils::safe_strcpy(ftdc, "DCE", sizeof(TThostFtdcExchangeIDType));
		break;
	case SHFE:
		Utils::safe_strcpy(ftdc, "SHFE", sizeof(TThostFtdcExchangeIDType));
		break;
	case CFFEX:
		Utils::safe_strcpy(ftdc, "CFFEX", sizeof(TThostFtdcExchangeIDType));
		break;
	default:
		ASSERT_TRUE(FALSE);
	}
}
