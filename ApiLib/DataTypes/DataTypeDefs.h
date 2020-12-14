#pragma once

#include "Utils/Fault.h"
#include "ThostApi/ThostFtdcUserApiStruct.h"
#include <string>
#include <vector>
#include <tuple>

/************************************************************************/
/* ENUM                                                                 */
/************************************************************************/

typedef enum
{
	CZCE = 0,
	SHFE,
	DCE,
	CFFEX,
	UNKNOWN
} ExchangeID;

typedef enum
{
	Buy = 0,
	Sell = 1
} Direction;

typedef enum
{
	Open = 0,
	Close,
	CloseToday,
	CloseYestoday,
	ForceClose
} OffsetFlag;

typedef enum
{
	Status_Unknown,
	Status_UnTraded,
	Status_Error,
	Status_AllTraded,
	Status_PartTraded,
	Status_Canceled,
	Status_NotTouched,
	Status_Touched
} OrderStatus;

/************************************************************************/
/* STRUCTURE                                                            */
/************************************************************************/

struct TimeDuration
{
	__time64_t	start_time;
	__time64_t	end_time;
	std::string	product_id;

	TimeDuration(const std::string& product_id, __time64_t start_time, __time64_t end_time);

	static TimeDuration MakeTimeDuration(const std::string& product_id, int week_day,
		int start_hour, int start_minute,
		int end_hour, int end_minute);
};

struct TradingDuration
{
	ExchangeID	exchange_id;
	std::vector<TimeDuration> durations;
};

struct TradingAccount
{
	TradingAccount();
	TradingAccount(CThostFtdcTradingAccountField& field);

	///经纪公司代码
	std::string		broker_id;
	///投资者帐号
	std::string		account_id;
	///上次质押金额
	double			pre_mortgage;
	///上次信用额度
	double			pre_credit;
	///上次存款额
	double			pre_deposit;
	///上次结算准备金
	double			pre_balance;
	///上次占用的保证金
	double			pre_margin;
	///利息收入
	double			interest;
	///入金金额
	double			deposit;
	///出金金额
	double			withdraw;
	///冻结的保证金
	double			frozen_margin;
	///冻结的资金
	double			frozen_cash;
	///冻结的手续费
	double			frozen_commission;
	///当前保证金总额
	double			curr_margin;
	///资金差额
	double			cash_in;
	///手续费
	double			commission;
	///平仓盈亏
	double			close_profit;
	///持仓盈亏
	double			position_profit;
	///期货结算准备金
	double			balance;
	///可用资金
	double			available;
	///可取资金
	double			withdraw_quota;
	///基本准备金
	double			reserve;
	///交易日
	std::string		trading_day;
	///信用额度
	double			credit;
	///质押金额
	double			mortgage;
	///交易所保证金
	double			exchange_margin;
	///投资者交割保证金
	double			delivery_margin;
	///交易所交割保证金
	double			exchange_delivery_margin;
	//静态权益
	double  StaticBalance();
	//动态权益
	double  DynamicBalance() const;
	//初始权益：= PreBalance - 出金 + 入金
	double  InitBalance() const;
};

struct Quote
{
	std::string		trading_day;
	ExchangeID		exchange_id;
	std::string		instrument_id;
	double			ask_price1;
	double			bid_price1;
	double			last_price;			//最新价

	double			open_price;			//今开盘	
	double			close_price;		//今收盘
	double			settlement_price;	//结算价
	double			highest_price;		//最高价
	double			lowest_price;		//最低价

	double			pre_settlement_price;	//上次结算价	;涨跌	= 昨结算-最新价
	double			average_price;

	int				trade_volume;		//成交量
	int				position_volume;    //持仓量
	int				ask_volume1;		//买价量
	int				bid_volume1;		//卖价量

	double			upper_limit_price;	// 涨停板价
	double			lower_limit_price;	// 跌停板价

	__time64_t		last_time;

	Quote(const CThostFtdcDepthMarketDataField& field);
	Quote(const std::string& instrument_id);

	bool operator < (const Quote& quote) const;
	Quote& operator=(const Quote& quote);

	typedef std::pair<std::string, __time64_t> key_type;
	key_type GetKey() const { return std::make_pair(instrument_id, last_time); }
};

struct QuoteDeque
{
	Quote* quotes[2];
	QuoteDeque() {
		quotes[0] = NULL;
		quotes[1] = NULL;
	}
	Quote*& operator[] (int index) {
		ASSERT_TRUE(index == 0 || index == 1);
		return quotes[index];
	}
};

struct Instrument
{
	ExchangeID		exchange_id;
	std::string		instrument_id;
	std::string		instrument_name;
	double			price_tick;			// 最小变动价位
	int				decs;
	int				volume_multiple;	// 合约数量乘数
	std::string		product_id;			// 产品代码
	bool			allow_market_order;

	Instrument(const CThostFtdcInstrumentField& field);
	Instrument(const Instrument& ins);
	Instrument(const std::string& id);
	Instrument(ExchangeID exid, const std::string& id);
	bool operator < (const Instrument& Instrument) const;
	typedef std::string key_type;
	key_type GetKey() const { return instrument_id; }
};

struct InstrumentMarginRate
{
	std::string		instrument_id;
	Direction		direction;
	double			margin_ratio_by_money;
	double			margin_ratio_by_volume;

	InstrumentMarginRate();
	bool operator < (const InstrumentMarginRate& rate) const;
};

struct FrontSession
{
	int front_id;
	int session_id;

	FrontSession() : front_id(0), session_id(0) {}
	FrontSession(int f, int s) : front_id(f), session_id(s) {}
	bool operator == (const FrontSession& ov) const;
	bool operator < (const FrontSession& ov) const;
};

struct OrderKey : public FrontSession
{
	int order_ref;
	OrderKey() : order_ref(0) {}
	OrderKey(int f, int s, int or) : FrontSession(f, s), order_ref(or) {}
	bool operator < (const OrderKey& ov) const;
};

struct Order
{
	std::string		instrument_id;
	std::string		order_sys_id;	// 报单编号
	Direction		direction;		// 买卖方向
	OffsetFlag		offset_flag;	// 组合开平标志
	int				request_id;		// 请求编号
	double			price;			// 价格
	int				volume;			// 数量
	int				volume_traded;	// 今成交数量
	int				volume_remained;	// 剩余数量
	int				broker_order_seq; // 经纪公司报单编号
	OrderStatus		status;			// 报单状态
	std::string		order_day;		// 报单日期
	std::string		order_time;		//委托时间
	std::string		status_msg;		// 状态信息
	OrderKey		key;
	
	Order();
	Order(CThostFtdcOrderField& field);
	Order(OrderKey& ref);
	bool operator < (const Order& order) const;
	typedef OrderKey key_type;
	key_type GetKey() const { return key; }
};

struct OrderInsert
{
	std::string		instrument_id;
	int				order_ref;
	Direction		direction;
	OffsetFlag		offset_flag;
	double			limit_price;
	int				volume;
	bool			is_market_order;
};

struct Trade
{
	ExchangeID		exchange_id;
	std::string		instrument_id;
	Direction		direction;
	OffsetFlag		offset_flag;
	std::string		order_sys_id;
	double			price;
	int				volume;
	std::string		trade_id;			// 成交编号
	std::string		trade_date;
	std::string		trade_time;

	Trade(CThostFtdcTradeField& field);
	bool operator < (const Trade& trade) const;
	typedef std::string key_type;
	key_type GetKey() const { return trade_id; }
};

struct Position
{
	ExchangeID		exchange_id;
	std::string		instrument_id;
	Direction		direction;
	int				yesterday_volume; // 上日持仓
	int				today_volume; // 今日持仓
	double			position_cost; // 持仓成本
	double			commission; // 手续费
	double			profit; // 持仓盈亏

	Position(const CThostFtdcInvestorPositionField& field);
	Position(const std::string& instrument_id, Direction direction);
	int volume() const { return today_volume + yesterday_volume; }
	bool operator < (const Position& position) const;
	typedef std::pair<std::string, Direction> key_type;
	key_type GetKey() const { return std::make_pair(instrument_id, direction); }
};

struct PositionDetail
{
	ExchangeID		exchange_id;
	std::string		instrument_id;
	Direction		direction;
	std::string		trade_id;
	int				volume;
	double			open_price;
	double			margin;
	double			profit;

	PositionDetail(const CThostFtdcInvestorPositionDetailField& field);
	bool operator < (const PositionDetail& position) const;
	typedef std::string key_type;
	key_type GetKey() const { return trade_id; }
};
