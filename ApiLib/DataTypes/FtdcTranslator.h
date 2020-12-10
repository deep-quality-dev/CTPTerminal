#pragma once

#include "DataTypeDefs.h"

void SetFromFtdcOrderStatus(OrderStatus& order_satus, const TThostFtdcOrderStatusType& ftdc_status);
void SetFromFtdcOffsetflag(OffsetFlag& offset_flag, const char& c);
void SetFromFtdcExchangeID(ExchangeID& exchangeid, const TThostFtdcExchangeIDType& ftdc_exchangeid);
void SetFromFtdcPosiDirection(Direction& direction, const TThostFtdcPosiDirectionType& ftdc_direction);
void SetFromFtdcDirection(Direction& direction, const TThostFtdcDirectionType& ftdc_direction);
void GetFromTickplusDirection(TThostFtdcDirectionType& ftdc, const Direction& direction);
void GetFromTickplusOffsetFlag(TThostFtdcCombOffsetFlagType& ftdc, const OffsetFlag& flag);
void GetFromTickplusExchangeID(TThostFtdcExchangeIDType& ftdc, const ExchangeID& exchangeid);