#include "stdafx.h"
#include "NtTypes.h"

std::wstring GetYesterdayChangeSign(char value)
{
	switch (value)
	{
	case '1':
		return _T("상한");
	case '2':
		return _T("상승");
	case '3':
		return _T("보합");
	case '4':
		return _T("하한");
	case '5':
		return _T("하락");
	default:
		return _T("에러?");
	}
}


////////////////////////////////////////////////////////////////////////////////

void Nt2101::SetFrom(t2101OutBlock* pData)
{
	hname = KRtoWide(pData->hname, sizeof(pData->hname));
	sign = GetYesterdayChangeSign(pData->sign[0]);

	lastmonth = KRtoWide(pData->lastmonth, sizeof(pData->lastmonth));

	price = ParseFloat(pData->price, 6, 2);
	change = ParseFloat(pData->change, 6, 2);
	jnilclose = ParseFloat(pData->jnilclose, 6, 2);
	uplmtprice = ParseFloat(pData->uplmtprice, 6, 2);
	dnlmtprice = ParseFloat(pData->dnlmtprice, 6, 2);
	recprice = ParseFloat(pData->recprice, 6, 2);
	kospijisu = ParseFloat(pData->kospijisu, 6, 2);

	jandatecnt = ParseInteger(pData->jandatecnt, sizeof(pData->jandatecnt));
}