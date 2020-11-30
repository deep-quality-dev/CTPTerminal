#pragma once

#include "DataTypeDefs.h"

void SetFromFtdcOrderStatus(OrderStatus& order_satus, const TThostFtdcOrderStatusType& ftdc_status);
void SetFromFtdcOffsetflag(OffsetFlag& offset_flag, const char& c);
void SetFromFtdcExchangeID(ExchangeID& exchangeid, const TThostFtdcExchangeIDType& ftdcexchangeid);
void SetFromFtdcDirection(Direction& direction, const TThostFtdcDirectionType& Direction);