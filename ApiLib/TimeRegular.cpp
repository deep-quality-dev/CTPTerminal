#include "stdafx.h"
#include "TimeRegular.h"
#include "Utils/Utils.h"


CTimeRegular::CTimeRegular()
{
	Init();
}


CTimeRegular::~CTimeRegular()
{
}

CTimeRegular CTimeRegular::GetInstance()
{
	static CTimeRegular time_regular;
	return time_regular;
}

std::vector<TimeDuration> CTimeRegular::GetTimeDuration(const ExchangeID exchange_id, const std::string& product_id)
{
	for (auto it = trading_durations_.begin(); it != trading_durations_.end(); it++) {
		if (it->first.first == exchange_id && it->first.second == product_id) {
			return it->second;
		}
	}
	return std::vector<TimeDuration>();
}

bool CTimeRegular::WithIn(const ExchangeID exchange_id, const std::string& product_id, __time64_t timestamp)
{
	std::vector<TimeDuration> durations = GetTimeDuration(exchange_id, product_id);
	for (auto it = durations.begin(); it != durations.end(); it++) {
		if (WithIn(*it, timestamp)) {
			return true;
		}
	}
	return false;
}

void CTimeRegular::AddTradingDuration(const ExchangeID exchange_id, const std::string& product_id, const TimeDuration& duration)
{
	trading_durations_[std::make_pair(exchange_id, product_id)].push_back(duration);
}

void CTimeRegular::Init()
{
	// CZCE : 郑商所
	// WH, TC, TA, SR, SM, SF, RS, RM, RI, PM, OI, ME, MA, LR, JR, FG, CF
	std::string czce_product_id_yepan[] = { "RM", "SR", "ME", "MA", "TA", "CF", "FG", "OI", "TC", "" };			//白糖(sr)、棉花(cf)、菜粕(rm)、甲醇(ma)、PTA(ta)、玻璃(fg)、菜油(oi)
	std::string czce_product_id_baitian1[] = { "PM", "WH", "RI", "LR", "RS", "" };	//普麦(pm)、强麦(wh)、早籼稻(ri)、晚籼稻(lr)、菜籽(rs)
	std::string czce_product_id_baitian2[] = { "JR", "SF", "SM", "" };					//粳稻(jr)、动力煤(tc)、硅铁(sf)、锰硅(sm)

	for (int idx_product = 0; czce_product_id_yepan[idx_product] != ""; ++idx_product) {
		std::string product_id = czce_product_id_yepan[idx_product];
		for (int i = 1; i < 6; ++i) {
			AddTradingDuration(ExchangeID::CZCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 21, 0, 23, 30));
			AddTradingDuration(ExchangeID::CZCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 9, 0, 10, 15));
			AddTradingDuration(ExchangeID::CZCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 10, 30, 11, 30));
			AddTradingDuration(ExchangeID::CZCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 13, 30, 15, 0));
		}
	}

	for (int idx_product = 0; czce_product_id_baitian1[idx_product] != ""; ++idx_product) {
		std::string product_id = czce_product_id_baitian1[idx_product];
		for (int i = 1; i < 6; ++i) {
			AddTradingDuration(ExchangeID::CZCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 9, 0, 10, 15));
			AddTradingDuration(ExchangeID::CZCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 10, 30, 11, 30));
			AddTradingDuration(ExchangeID::CZCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 13, 30, 15, 0));
		}
	}

	for (int idx_product = 0; czce_product_id_baitian2[idx_product] != ""; ++idx_product) {
		std::string product_id = czce_product_id_baitian2[idx_product];
		for (int i = 1; i < 6; ++i) {
			AddTradingDuration(ExchangeID::CZCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 9, 0, 10, 15));
			AddTradingDuration(ExchangeID::CZCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 10, 30, 11, 30));
			AddTradingDuration(ExchangeID::CZCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 13, 30, 15, 0));
		}
	}

	// DCE
	// y, v, pp, p, m, l, jm, jd, j, i, fb, cs, c, bb, a
	std::string dce_product_id_yepan[] = { "j", "p", "m", "y", "a", "jm", "i", "" };			//焦炭(j), 棕榈油(p), 豆粕(m), 豆油(y), 大豆2(), 大豆1(a), 焦煤(jm), 铁矿石(i)
	std::string dce_product_id_baitian[] = { "c", "l", "v", "pp", "jd", "fb", "bb", "cs", "" };	//玉米(c)、聚乙烯(l)、聚氯乙烯(v)、聚丙烯(pp)、鸡蛋(jd)、纤维板(fb)、胶合板(bb)、玉米淀粉(cs)

	for (int idx_product = 0; dce_product_id_yepan[idx_product] != ""; ++idx_product) {
		std::string product_id = dce_product_id_yepan[idx_product];
		for (int i = 1; i < 6; ++i) {
			AddTradingDuration(ExchangeID::DCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 21, 0, 23, 30));
			AddTradingDuration(ExchangeID::DCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 9, 0, 10, 15));
			AddTradingDuration(ExchangeID::DCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 10, 30, 11, 30));
			AddTradingDuration(ExchangeID::DCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 13, 30, 15, 0));
		}
	}

	for (int idx_product = 0; dce_product_id_baitian[idx_product] != ""; ++idx_product) {
		std::string product_id = dce_product_id_baitian[idx_product];
		for (int i = 1; i < 6; ++i) {
			AddTradingDuration(ExchangeID::DCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 9, 0, 10, 15));
			AddTradingDuration(ExchangeID::DCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 10, 30, 11, 30));
			AddTradingDuration(ExchangeID::DCE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 13, 30, 15, 0));
		}
	}

	// SHFE
	// zn, wr, sn, ru, rb, pb, ni, hc, fu, cu, bu, au, al, ag
	std::string shfe_product_id_yepan1[] = { "cu", "al", "pb", "zn", "rb", "bu", "hc", "ni", "sn", "" };	//铜(cu)、铝(al)、铅(pb)、锌(zn)、螺纹(rb)、沥青(bu)、热轧卷板(hc)、镍(ni)、锡(sn)
	std::string shfe_product_id_yepan2[] = { "ag", "au", "" };		//白银, 黄金
	std::string shfe_product_id_yepan3[] = { "ru", "" };			//天胶
	std::string shfe_product_id_baitian[] = { "fu", "wr", "" };		//燃油、线材

	for (int idx_product = 0; shfe_product_id_yepan1[idx_product] != ""; ++idx_product) {
		std::string product_id = shfe_product_id_yepan1[idx_product];
		for (int i = 1; i < 6; ++i) {
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 21, 0, 24, 0));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 0, 0, 1, 0));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 9, 0, 10, 15));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 10, 30, 11, 30));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 13, 30, 15, 0));
		}
	}

	for (int idx_product = 0; shfe_product_id_yepan2[idx_product] != ""; ++idx_product) {
		std::string product_id = shfe_product_id_yepan2[idx_product];
		for (int i = 1; i < 6; ++i) {
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 21, 0, 24, 0));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 0, 0, 2, 30));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 9, 0, 10, 15));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 10, 30, 11, 30));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 13, 30, 15, 0));
		}
	}

	for (int idx_product = 0; shfe_product_id_yepan3[idx_product] != ""; ++idx_product) {
		std::string product_id = shfe_product_id_yepan3[idx_product];
		for (int i = 1; i < 6; ++i) {
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 21, 0, 23, 0));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 9, 0, 10, 15));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 10, 30, 11, 30));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 13, 30, 15, 0));
		}
	}

	for (int idx_product = 0; shfe_product_id_baitian[idx_product] != ""; ++idx_product) {
		std::string product_id = shfe_product_id_baitian[idx_product];
		for (int i = 1; i < 6; ++i) {
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 9, 0, 10, 15));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 10, 30, 11, 30));
			AddTradingDuration(ExchangeID::SHFE, product_id, TimeDuration::MakeTimeDuration(product_id, i, 13, 30, 15, 0));
		}
	}
}

bool CTimeRegular::WithIn(const TimeDuration& td, __time64_t timestamp)
{
	SYSTEMTIME start_time = Utils::GetSystemTime(td.start_time);
	SYSTEMTIME end_time = Utils::GetSystemTime(td.end_time);

	SYSTEMTIME systime = Utils::GetSystemTime(timestamp);
	if (start_time.wDayOfWeek != systime.wDayOfWeek)
		return false;

	if (start_time.wHour > systime.wHour || systime.wHour > (end_time.wHour == 0 ? 24 : end_time.wHour))
		return false;

	if (start_time.wHour == systime.wHour && start_time.wMinute > systime.wMinute)
		return false;

	if (start_time.wHour == systime.wHour && start_time.wMinute == systime.wMinute && start_time.wSecond > systime.wSecond)
		return false;

	if (systime.wHour == end_time.wHour && systime.wMinute > end_time.wMinute)
		return false;

	if (systime.wHour == end_time.wHour && systime.wMinute == end_time.wMinute && systime.wSecond > end_time.wSecond)
		return false;

	return true;
}
