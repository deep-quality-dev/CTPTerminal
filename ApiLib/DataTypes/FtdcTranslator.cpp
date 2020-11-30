#include "stdafx.h"
#include "FtdcTranslator.h"
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

void SetFromFtdcExchangeID(ExchangeID& exchangeid, const TThostFtdcExchangeIDType& ftdcexchangeid)
{
	exchangeid = UNKNOWN;
	if (ftdcexchangeid == std::string("SHFE")) {
		exchangeid = SHFE;
	}
	else if (ftdcexchangeid == std::string("CZCE")) {
		exchangeid = CZCE;
	}
	else if (ftdcexchangeid == std::string("DCE")) {
		exchangeid = DCE;
	}
	else if (ftdcexchangeid == std::string("CFFEX")) {
		exchangeid = CFFEX;
	}
}

void SetFromFtdcDirection(Direction& direction, const TThostFtdcDirectionType& Direction)
{
	switch (direction)
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

