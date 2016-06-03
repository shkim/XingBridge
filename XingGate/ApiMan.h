#pragma once

struct ApiListener
{
	virtual void API_OnLoginResult(bool success) = 0;
	virtual void API_OnLogout() = 0;
};

struct _RECV_PACKET;

struct RequestInfo
{
	int nReqType;
	int nDataReceived;

	int nPeriodType;
	int nIndexType;
};

class ApiMan
{
public:
	ApiMan();

	bool Create(HWND hWnd, ApiListener* pLsnr);
	void Destroy();
	bool HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

	bool Login();
	bool Logout();

	void Request_T2101(const char* shcode);
	void Request_ChartIndex(const char* shcode, int nIndexType);

	enum ChartIndexTypes
	{
		CHART_PRICE_MOVE_AVG = 1,
		CHART_MACD,
		CHART_RSI
	};

private:
	void OnLoginResult(WPARAM, LPARAM);
	void OnReceiveData(WPARAM, LPARAM);

	RequestInfo* RegisterRequestInfo(int nReqId, int nReqType);
	void Process_T2101(RequestInfo* pRI, _RECV_PACKET* pPacket);
	void Process_ChartIndex(RequestInfo* pRI, _RECV_PACKET* pPacket);

	HWND m_hWndNotify;
	ApiListener* m_pListener;

	std::wstring m_iniPath;
	int m_nChartQueryRows;
	std::map<int, RequestInfo*> m_mapReqId2Info;
};

extern ApiMan* g_api;