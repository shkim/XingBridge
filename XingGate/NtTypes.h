#pragma once

#include "TrTypes.h"

class Nt2101
{
public:
	std::wstring hname;	// 한글명
	std::wstring sign;	// 전일대비구분

	std::wstring lastmonth;	// 만기일
	int jandatecnt;	// 잔여일

	double price;	// 현재가
	double change;	// 전일대비
	double jnilclose;	// 전일종가
	double uplmtprice;	// 상한가
	double dnlmtprice;	// 하한가
	double recprice;	// 기준가
	double kospijisu;	// KOSPI200지수

	void SetFrom(t2101OutBlock* pData);
};

std::wstring GetYesterdayChangeSign(char value);
