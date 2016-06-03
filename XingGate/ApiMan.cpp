#include "stdafx.h"
#include "XingGate.h"

#include "TrTypes.h"
#include "TrChartIndex.h"
#include "NtTypes.h"

#include "IXingAPI.h"

enum tagCHART_PERIOD
{
	CHART_PER_TICK = 0,		// 틱
	CHART_PER_MIN = 1,		// 분
	CHART_PER_DAILY = 2,		// 일
	CHART_PER_WEEKLY = 3,		// 주
	CHART_PER_MONTHLY = 4,		// 월
};

enum tagCHART_MARKET_TYPE
{
	MK_STOCK = 1,		// 주식
	MK_IND = 2,		// 업종
	MK_FO = 5,		// 선물옵션
};

enum
{
	REQTYPE_T2101 = 1,
	REQTYPE_CHARTINDEX,
	REQTYPE_TODO
};

static IXingAPI s_xingApi;

#define START_WM_ID		WM_USER

static LPCTSTR INIFILE_APPNAME = _T("Xing"); // section

ApiMan::ApiMan()
{
	m_hWndNotify = NULL;
	m_pListener = NULL;

	TCHAR szIniPath[MAX_PATH];
	GetModuleFileName(g_hInstance, szIniPath, MAX_PATH);
	_tcscpy_s(_tcsrchr(szIniPath, '.'), 16, _T(".ini"));
	m_iniPath = szIniPath;

	if (_taccess(szIniPath, 0) < 0)
	{
		// INI file not exists; create defaults.
		WritePrivateProfileString(INIFILE_APPNAME, _T("ApiDir"), _T("C:\\eBEST\\XingAPI"), szIniPath);
		WritePrivateProfileString(INIFILE_APPNAME, _T("UserID"), _T("userid"), szIniPath);
		WritePrivateProfileString(INIFILE_APPNAME, _T("UserPW"), _T("passwd1"), szIniPath);
		WritePrivateProfileString(INIFILE_APPNAME, _T("CertPW"), _T("passwd2"), szIniPath);
		WritePrivateProfileString(INIFILE_APPNAME, _T("ChartRows"), _T("100"), szIniPath);
		WritePrivateProfileString(INIFILE_APPNAME, _T("BaseUrl"), _T("http://api.server.com/a"), szIniPath);
	}

}

bool ApiMan::Create(HWND hWnd, ApiListener* pLsnr)
{
	TCHAR szBuff[MAX_PATH];
	GetPrivateProfileString(INIFILE_APPNAME, _T("ApiDir"), _T("C:\\eBEST\\XingAPI"), szBuff, MAX_PATH, m_iniPath.c_str());

	if (s_xingApi.Init(szBuff) == FALSE)
	{
		LogE(_T("XingApi init failed."));
		return false;
	}

	GetPrivateProfileString(INIFILE_APPNAME, _T("ChartRows"), _T("50"), szBuff, MAX_PATH, m_iniPath.c_str());
	m_nChartQueryRows = _ttoi(szBuff);
	if (m_nChartQueryRows <= 0)
		m_nChartQueryRows = 50;
	else if (m_nChartQueryRows > 500)
		m_nChartQueryRows = 500;

	GetPrivateProfileString(INIFILE_APPNAME, _T("BaseUrl"), _T("http://api.server.com/a"), szBuff, MAX_PATH, m_iniPath.c_str());
	g_svr->SetBaseUrl(szBuff);

	LogI(_T("XingApi initialized."));
	m_hWndNotify = hWnd;
	m_pListener = pLsnr;
	return true;
}

void ApiMan::Destroy()
{
	if (m_hWndNotify && s_xingApi.IsConnected())
	{
		s_xingApi.Disconnect();
	}

	m_hWndNotify = NULL;
}

bool ApiMan::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message < START_WM_ID || message > (START_WM_ID + XM_RECEIVE_REAL_DATA_CHART))
	{
		return false;
	}

	switch (message - START_WM_ID)
	{
	case XM_LOGIN:
		OnLoginResult(wParam, lParam);
		break;

	case XM_LOGOUT:
		LogI(_T("TODO: XM_LOGOUT"));
		m_pListener->API_OnLogout();
		break;

	case XM_RECEIVE_DATA:
		OnReceiveData(wParam, lParam);
		break;

	default:
		LogE(_T("Unhandled XM: %d"), (message - START_WM_ID));
	}

	return true;
}

bool ApiMan::Logout()
{
	LogI(L"Left ReqID count: %d", m_mapReqId2Info.size());

	if (m_hWndNotify && s_xingApi.IsConnected())
	{
		s_xingApi.Logout(m_hWndNotify);
		return true;
	}

	return false;
}

bool ApiMan::Login()
{
	if (s_xingApi.IsConnected())
	{
		LogE(_T("OnLogin: was connected, do disconnect."));
		s_xingApi.Disconnect();
	}

	if (s_xingApi.Connect(m_hWndNotify, (LPCTSTR)("hts.etrade.co.kr"), 20001, START_WM_ID))
	{
		LogD(_T("Connect API succeeded"));
	}
	else
	{
		LogE(_T("Connect API Failed."));
		return false;
	}

	TCHAR szUserID[64];
	TCHAR szUserPW[64];
	TCHAR szCertPW[64];
	GetPrivateProfileString(INIFILE_APPNAME, _T("UserID"), _T("uid"), szUserID, 64, m_iniPath.c_str());
	GetPrivateProfileString(INIFILE_APPNAME, _T("UserPW"), _T("upw"), szUserPW, 64, m_iniPath.c_str());
	GetPrivateProfileString(INIFILE_APPNAME, _T("CertPW"), _T("cpw"), szCertPW, 64, m_iniPath.c_str());

	USES_CONVERSION;
	std::string userid(W2A(szUserID));
	std::string passwd(W2A(szUserPW));
	std::string certpw(W2A(szCertPW));

	if (s_xingApi.Login(m_hWndNotify, userid.c_str(), passwd.c_str(), certpw.c_str(), 0, TRUE))
	{
		LogD(_T("Login API succeeded."));
		return true;
	}
	else
	{
		LogE(_T("Login API failed."));
		return false;
	}
}

void CopyStringAndFillSpace(char* pDest, int lenDest, const char* pszSource)
{
	int lenSrc = strlen(pszSource);
	int nCopy = (lenDest < lenSrc) ? lenDest : lenSrc;
	memcpy(pDest, pszSource, nCopy);
	for (int n = nCopy; nCopy < lenDest; n++)
	{
		pDest[n] = ' ';
	}
}

RequestInfo* ApiMan::RegisterRequestInfo(int nReqId, int nReqType)
{
	if (nReqId < 0)
	{
		LogE(L"Request (Type=%d) failed (ID is -1)", nReqType);
		return NULL;
	}

	if (m_mapReqId2Info.find(nReqId) != m_mapReqId2Info.end())
	{
		LogE(L"FATAL: ReqID %d duplicated.", nReqId);
	}

	RequestInfo* pRI = new RequestInfo();
	memset(pRI, 0, sizeof(RequestInfo));
	pRI->nReqType = nReqType;

	m_mapReqId2Info[nReqId] = pRI;
	LogI(L"Request %d registered with type %d", nReqId, nReqType);
	return pRI;
}

void ApiMan::Request_T2101(const char* shcode)
{
	ASSERT(shcode != NULL);

	t2101InBlock inBlock;
	memset(&inBlock, ' ', sizeof(inBlock));
	CopyStringAndFillSpace(inBlock.focode, sizeof(inBlock.focode), shcode);

	int nReqId = s_xingApi.Request(m_hWndNotify,
		(LPCTSTR)"t2101",
		&inBlock, sizeof(inBlock));

	RegisterRequestInfo(nReqId, REQTYPE_T2101);
}

void ApiMan::Request_ChartIndex(const char* shcode, int nIndexType)
{
	ChartIndexInBlock inblock;
	memset(&inblock, 0x20, sizeof(ChartIndexInBlock));

	switch (nIndexType)
	{
	case CHART_PRICE_MOVE_AVG:
		StringCchCopyA(inblock.indexname, 40, W2K(L"가격 이동평균"));
		break;
	case CHART_MACD:
		StringCchCopyA(inblock.indexname, 40, W2K(L"MACD"));
		break;
	case CHART_RSI:
		StringCchCopyA(inblock.indexname, 40, W2K(L"RSI"));
		break;
	default:
		LogE(L"Invalid Chart index type: %d", nIndexType);
		return;
	}

	SYSTEMTIME tm;
	GetLocalTime(&tm);
	char szDate[32];
	StringCchPrintfA(szDate, 32, "%04d%02d%02d", tm.wYear, tm.wMonth, tm.wDay);
	
	inblock.market[0] = MK_FO + '0';
	inblock.period[0] = CHART_PER_DAILY + '0';
	CopyStringAndFillSpace(inblock.shcode, sizeof(inblock.shcode), shcode);
	StringCchPrintfA(inblock.qrycnt, 4, "%d", m_nChartQueryRows); // 요청건수(최대 500개)
	StringCchPrintfA(inblock.ncnt, 4, "%d", 1);
	CopyStringAndFillSpace(inblock.edate, sizeof(inblock.edate), szDate); // 종료일자(일/주/월 해당)
	inblock.Isamend[0] = '1'; // 수정주가 반영
	inblock.Isgab[0] = '1'; // 갭 보정
	inblock.IsReal[0] = '0'; // 실시간 데이터 자동 등록 여부 (0:조회만, 1:실시간 자동 등록)
	
	int nReqId = s_xingApi.RequestService(m_hWndNotify, (LPCTSTR)NAME_ChartIndex, (LPCTSTR)&inblock);
	RequestInfo* pRI = RegisterRequestInfo(nReqId, REQTYPE_CHARTINDEX);
	if (pRI)
	{
		pRI->nIndexType = nIndexType;
		pRI->nPeriodType = CHART_PER_DAILY;
	}
}

void ApiMan::OnLoginResult(WPARAM wParam, LPARAM lParam)
{
	LPCSTR pszCode = (LPCSTR)wParam;
	LPCSTR pszMsgA = (LPCSTR)lParam;

	std::wstring msgW = KRtoWide(pszMsgA);
	LogI(_T("LoginResult: [%S] %s"), pszCode, msgW.c_str());

	if (!strcmp(pszCode, "0000"))
	{
		m_pListener->API_OnLoginResult(true);
	}
	else
	{
		m_pListener->API_OnLoginResult(false);
	}
}

void ApiMan::OnReceiveData(WPARAM wParam, LPARAM lParam)
{
	if (wParam == REQUEST_DATA)
	{
		RECV_PACKET* pRpData = (RECV_PACKET*)lParam;
		auto itr = m_mapReqId2Info.find(pRpData->nRqID);
		if (itr == m_mapReqId2Info.end())
		{
			LogE(_T("Request (ID=%d) not found in Info map"), pRpData->nRqID);
		}
		else
		{
			RequestInfo* pRI = itr->second;
			switch (pRI->nReqType)
			{
			case REQTYPE_T2101:
				Process_T2101(pRI, pRpData);
				break;

			case REQTYPE_CHARTINDEX:
				Process_ChartIndex(pRI, pRpData);
				break;

			default:
				LogE(_T("Unknown RequestType %d (ReqID=%d)"), pRI->nReqType, pRpData->nRqID);
			}
		}

		if (pRpData->cCont[0] == '1')
		{
			LogE(L"Next data available (cCont==1), but not programmed");
		}
	}
	else if (wParam == MESSAGE_DATA)
	{
		MSG_PACKET* pMsg = (MSG_PACKET*)lParam;

		std::wstring msg = KRtoWide((char*)pMsg->lpszMessageData, pMsg->nMsgLength);
		LogE(_T("MESSAGE_DATA: %s"), msg.c_str());

		s_xingApi.ReleaseMessageData(lParam);
	}
	else if (wParam == SYSTEM_ERROR_DATA)
	{
		MSG_PACKET* pMsg = (MSG_PACKET*)lParam;

		std::wstring msg = KRtoWide((char*)pMsg->lpszMessageData, pMsg->nMsgLength);
		LogE(_T("SYSTEM_ERROR_DATA: %s"), msg.c_str());

		s_xingApi.ReleaseMessageData(lParam);
	}
	else if (wParam == RELEASE_DATA)
	{
		LogD(_T("Got RELEASE_DATA: %d"), (int)lParam);
		s_xingApi.ReleaseRequestData((int)lParam);

		auto itr = m_mapReqId2Info.find((int)lParam);
		if (itr == m_mapReqId2Info.end())
		{
			LogE(_T("Request (ID=%d) not found in Info map"), lParam);
		}
		else
		{
			delete itr->second;
			m_mapReqId2Info.erase(itr);
		}
	}
	else
	{
		LogE(_T("Unknown XM Recv wParam=%d"), wParam);
	}
}

////////////////////////////////////////////////////////////////////////////////

void ApiMan::Process_T2101(RequestInfo* pRI, _RECV_PACKET* pPacket)
{
	t2101OutBlock* pBlock = (t2101OutBlock*)pPacket->lpData;

	Nt2101 data;
	data.SetFrom(pBlock);

	LogI(_T("REQUEST_DATA: %s, 현재가=%.2f, 전일대비=%s, 만기일=%s, 남은날=%d"), data.hname.c_str(), data.price, data.sign.c_str(), data.lastmonth.c_str(), data.jandatecnt);
	LogI(_T("  전일종가=%.2f, 상한가=%.2f, 하한가=%.2f, 기준가=%.2f, KOSPI200지수=%.2f"), data.jnilclose, data.uplmtprice, data.dnlmtprice, data.recprice, data.kospijisu);
}

struct ChartIndexOutWrap
{
	ChartIndexOutBlock	outBlock;
	char				sCountOutBlock[5];	// 반복데이터 갯수
	ChartIndexOutBlock1	outBlock1[1];
};

void ApiMan::Process_ChartIndex(RequestInfo* pRI, _RECV_PACKET* pPacket)
{
	ChartIndexOutWrap* pWrap = (ChartIndexOutWrap*)pPacket->lpData;

//	TRACE("indexid: %s\n", KRtoAnsi(pWrap->outBlock.indexid, sizeof(pWrap->outBlock.indexid)).c_str());
//	TRACE("sCountOutBlk: %s, nDataLen=%d\n", pWrap->sCountOutBlock, pPacket->nDataLength);
//	TRACE("sizeof lpData: %d, elapsed time: %d, dataMode: %d\n", pPacket->nTotalDataBufferSize, pPacket->nElapsedTime, pPacket->nDataMode);

	int nColumnCount = std::stoi((KRtoAnsi(pWrap->outBlock.validdata_cnt, sizeof(pWrap->outBlock.validdata_cnt))));

	if (KRtoWide(pWrap->outBlock1[0].date, sizeof(pWrap->outBlock1[0].date)).compare(L"일자")
		|| KRtoWide(pWrap->outBlock1[0].time, sizeof(pWrap->outBlock1[0].time)).compare(L"시간")
		|| KRtoWide(pWrap->outBlock1[0].open, sizeof(pWrap->outBlock1[0].open)).compare(L"시가")
		|| KRtoWide(pWrap->outBlock1[0].high, sizeof(pWrap->outBlock1[0].high)).compare(L"고가")
		|| KRtoWide(pWrap->outBlock1[0].low, sizeof(pWrap->outBlock1[0].low)).compare(L"저가")
		|| KRtoWide(pWrap->outBlock1[0].close, sizeof(pWrap->outBlock1[0].close)).compare(L"종가")
		|| KRtoWide(pWrap->outBlock1[0].volume, sizeof(pWrap->outBlock1[0].volume)).compare(L"거래량"))
	{
		LogE(L"ChartIndex 기본 필드명에 에러가 감지되었습니다.");
		return;
	}

	if (pRI->nIndexType == CHART_PRICE_MOVE_AVG)
	{
		// 가격 이동 평균
		if (nColumnCount != 12
			|| KRtoWide(pWrap->outBlock1[0].value1, sizeof(pWrap->outBlock1[0].value1)).compare(L"종가 단순")
			|| KRtoWide(pWrap->outBlock1[0].value2, sizeof(pWrap->outBlock1[0].value2)).compare(L"10")
			|| KRtoWide(pWrap->outBlock1[0].value3, sizeof(pWrap->outBlock1[0].value3)).compare(L"20")
			|| KRtoWide(pWrap->outBlock1[0].value4, sizeof(pWrap->outBlock1[0].value4)).compare(L"60")
			|| KRtoWide(pWrap->outBlock1[0].value5, sizeof(pWrap->outBlock1[0].value5)).compare(L"120"))
		{
			LogE(L"ChartIndex 가격 이동평균 필드명에 에러가 감지되었습니다.");
			return;
		}
	}
	else if (pRI->nIndexType == CHART_MACD)
	{
		// MACD
		if (nColumnCount != 10
			|| KRtoWide(pWrap->outBlock1[0].value1, sizeof(pWrap->outBlock1[0].value1)).compare(L"MACD Oscil")
			|| KRtoWide(pWrap->outBlock1[0].value2, sizeof(pWrap->outBlock1[0].value2)).compare(L"MACD 12,26")
			|| KRtoWide(pWrap->outBlock1[0].value3, sizeof(pWrap->outBlock1[0].value3)).compare(L"시그널 9"))
		{
			LogE(L"ChartIndex MACD 필드명에 에러가 감지되었습니다.");
			return;
		}
	}
	else if (pRI->nIndexType == CHART_RSI)
	{
		// RSI
		if (nColumnCount != 9
			|| KRtoWide(pWrap->outBlock1[0].value1, sizeof(pWrap->outBlock1[0].value1)).compare(L"RSI 14")
			|| KRtoWide(pWrap->outBlock1[0].value2, sizeof(pWrap->outBlock1[0].value2)).compare(L"시그널 9"))
		{
			LogE(L"ChartIndex RSI 필드명에 에러가 감지되었습니다.");
			return;
		}
	}
	else
	{
		LogE(L"Unknown ChartIndex index type: %d", pRI->nIndexType);
		return;
	}

	int nRecordCnt = std::stoi(KRtoWide(pWrap->sCountOutBlock, sizeof(pWrap->sCountOutBlock)));
	std::vector<Kospi200FutureItem> items;
	items.reserve(nRecordCnt);
	for (int i = 1; i < nRecordCnt; i++)
	{
		Kospi200FutureItem item;
		
		std::wstring pos = KRtoWide(pWrap->outBlock1[i].pos, sizeof(pWrap->outBlock1[i].pos));
		item.day = KRtoWide(pWrap->outBlock1[i].date, sizeof(pWrap->outBlock1[i].date));
		item.time = KRtoWide(pWrap->outBlock1[i].time, sizeof(pWrap->outBlock1[i].time));
		item.start = KRtoWide(pWrap->outBlock1[i].open, sizeof(pWrap->outBlock1[i].open));
		item.high = KRtoWide(pWrap->outBlock1[i].high, sizeof(pWrap->outBlock1[i].high));
		item.low = KRtoWide(pWrap->outBlock1[i].low, sizeof(pWrap->outBlock1[i].low));
		item.end = KRtoWide(pWrap->outBlock1[i].close, sizeof(pWrap->outBlock1[i].close));
		item.quantity = KRtoWide(pWrap->outBlock1[i].volume, sizeof(pWrap->outBlock1[i].volume));

		item.value1 = KRtoWide(pWrap->outBlock1[i].value1, sizeof(pWrap->outBlock1[i].value1));
		item.value2 = KRtoWide(pWrap->outBlock1[i].value2, sizeof(pWrap->outBlock1[i].value2));
		item.value3 = KRtoWide(pWrap->outBlock1[i].value3, sizeof(pWrap->outBlock1[i].value3));
		item.value4 = KRtoWide(pWrap->outBlock1[i].value4, sizeof(pWrap->outBlock1[i].value4));
		item.value5 = KRtoWide(pWrap->outBlock1[i].value5, sizeof(pWrap->outBlock1[i].value5));

		items.push_back(item);

		LogI(L"%s: 일자:%s 시간:%s 시가:%s 고가:%s 저가:%s 종가:%s 거래량:%s", pos.c_str(),
			item.day.c_str(), item.time.c_str(), item.start.c_str(), item.high.c_str(), item.low.c_str(), item.end.c_str(), item.quantity.c_str());

		if (pRI->nIndexType == CHART_PRICE_MOVE_AVG)
		{
			LogI(L"\t종가단순:%s 10:%s 20:%s 60:%s 120:%s", item.value1.c_str(), item.value2.c_str(), item.value3.c_str(), item.value4.c_str(), item.value5.c_str());
		}
		else if (pRI->nIndexType == CHART_MACD)
		{
			LogI(L"\tMACD Oscil:%s MACD 12,26:%s 시그널9:%s", item.value1.c_str(), item.value2.c_str(), item.value3.c_str());
		}
		else if (pRI->nIndexType == CHART_RSI)
		{
			LogI(L"\tRSI14:%s 시그널9:%s", item.value1.c_str(), item.value2.c_str());
		}
	}

	g_svr->PutKospiFutureChart(items, pRI->nIndexType);
}