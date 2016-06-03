#pragma once

struct SvrListener
{
	virtual void SVR_OnShcodeResult(LPCTSTR pszShcode) = 0;
};

struct Kospi200FutureItem
{
	std::wstring day, time;
	std::wstring start, high, low, end, quantity;
	std::wstring value1, value2, value3, value4, value5;
};

class SvrMan
{
public:
	void SetBaseUrl(LPCTSTR pszBaseUrl);

	void RequestKospiFutureShcode(SvrListener* pLsnr);
	void PutKospiFutureChart(std::vector<Kospi200FutureItem>& items, int nIndexType);

public:
	//SvrListener* m_pListener;
	std::wstring m_strApiBaseUrl;
};

extern SvrMan *g_svr;