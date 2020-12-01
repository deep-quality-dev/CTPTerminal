#pragma once

#include "ThostApi/ThostFtdcUserApiStruct.h"

class CThostSpiMessage
{
public:
	enum MSG_TYPE {
		OnMdFrontConnected,
		OnMdFrontDisconnected,
		OnTradeFrontConnected,
		OnTradeFrontDisconnected,
		OnRspAuthenticate,
		OnRspUserLogin,
		OnRspUserLogout,
		OnRspSubMarketData,
		OnRspUnSubMarketData,
		OnRtnDepthMarketData,
		OnRspQryOrder,
		OnRspQryTrade,
		OnRspQryInvestorPosition,
		OnRspQryTradingAccount,

		OnRspQryInstrument,
		OnRspQryDepthMarketData,
		OnRspQryInvestorPositionDetail
	};

	template <class T>
	CThostSpiMessage(MSG_TYPE msg_type, const T* thost_ftdc_spi_field = NULL, CThostFtdcRspInfoField* rsp_field = NULL, int request_id = 0, bool is_last = true) {
		msg_type_ = msg_type;
		thost_ftdc_spi_field_ = NULL;
		if (thost_ftdc_spi_field != NULL) {
			size_of_ = sizeof(T);
			thost_ftdc_spi_field_ = new T;
			memcpy_s(thost_ftdc_spi_field_, size_of_, thost_ftdc_spi_field, size_of_);
		}

		if (rsp_field != NULL) {
			memcpy_s(&rsp_field_, sizeof(CThostFtdcRspInfoField), rsp_field, sizeof(CThostFtdcRspInfoField));
		}
		else {
			memset(&rsp_field_, 0, sizeof(CThostFtdcRspInfoField));
		}

		request_id_ = request_id;
		is_last_ = is_last;
	}

	CThostSpiMessage(MSG_TYPE msg_type) {
		msg_type_ = msg_type;
		thost_ftdc_spi_field_ = NULL;
		size_of_ = 0;
		memset(&rsp_field_, 0, sizeof(rsp_field_));
		request_id_ = 0;
		is_last_ = true;
	}

	CThostSpiMessage(const CThostSpiMessage& spi_message) {
		this->msg_type_ = spi_message.msg_type_;
		this->size_of_ = spi_message.size_of_;
		this->thost_ftdc_spi_field_ = NULL;
		if (this->size_of_ > 0) {
			this->thost_ftdc_spi_field_ = new char[spi_message.size_of_];
			memcpy_s(this->thost_ftdc_spi_field_, size_of_, spi_message.thost_ftdc_spi_field_, size_of_);
		}
		memcpy_s(&rsp_field_, sizeof(CThostFtdcRspInfoField), &spi_message.rsp_field_, sizeof(CThostFtdcRspInfoField));
	}

	~CThostSpiMessage() {
		if (this->thost_ftdc_spi_field_) {
			delete[] this->thost_ftdc_spi_field_;
		}
	}

	template <class T>
	T* GetFieldPtr() {
		return (T*) this->thost_ftdc_spi_field_;
	}

	MSG_TYPE msg_type() const {
		return msg_type_;
	}

	CThostFtdcRspInfoField* rsp_field() {
		return &rsp_field_;
	}

	int request_id() const {
		return request_id_;
	}

	bool is_last() const {
		return is_last_;
	}

private:
	MSG_TYPE msg_type_;
	void* thost_ftdc_spi_field_;
	int size_of_;
	CThostFtdcRspInfoField rsp_field_;
	int request_id_;
	bool is_last_;
};

typedef CThostSpiMessage SPI;