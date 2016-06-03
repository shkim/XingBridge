#include "stdafx.h"
#include "XingGate.h"
#include "SvrMan.h"

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency;

void SvrMan::SetBaseUrl(LPCTSTR pszBaseUrl)
{
	m_strApiBaseUrl = pszBaseUrl;
}

void SvrMan::RequestKospiFutureShcode(SvrListener* pLsnr)
{
	TCHAR szURL[128];
	StringCchPrintf(szURL, 128, U("%s/v1/shcode/kospi200f"), m_strApiBaseUrl.c_str());

	http_client client(szURL);
	client.request(methods::GET).then([pLsnr](task<http_response> responseTask)
	{
		try
		{
			http_response resp = responseTask.get();

			const web::json::value& rj = resp.extract_json().get();
			const web::json::value& shcode = rj.at(U("shcode"));
			if (shcode.is_string())
			{
				pLsnr->SVR_OnShcodeResult(shcode.as_string().c_str());
				return;
			}
		}
		catch (const http_exception &ex)
		{
			TRACE("RequestKospiFutureShcode failed: %s\n", ex.error_code().message().c_str());
		}

		// FIXME
		pLsnr->SVR_OnShcodeResult(U("101L6000"));
	});
}

void SvrMan::PutKospiFutureChart(std::vector<Kospi200FutureItem>& items, int nIndexType)
{
	if (items.empty())
	{
		LogE(L"PutKospiFutureChart: items is empty");
		return;
	}

	TCHAR szURL[128];
	StringCchPrintf(szURL, 128, U("%s/v1/kospi200f"), m_strApiBaseUrl.c_str());

	json::value body;
	body[U("period")] = json::value::string(U("day"));

	utility::string_t extraType;
	switch (nIndexType)
	{
	case ApiMan::CHART_PRICE_MOVE_AVG:
		extraType = U("PMA");
		break;
	case ApiMan::CHART_MACD:
		extraType = U("MACD");
		break;
	case ApiMan::CHART_RSI:
		extraType = U("RSI");
		break;
	default:
		LogE(L"Unknown chart index type: %d", nIndexType);
		return;
	}

	body[U("extra")] = json::value::string(extraType);

	std::vector<json::value> jarr;
	for (auto itr = items.begin(); itr != items.end(); ++itr)
	{
		Kospi200FutureItem& src = *itr;

		json::value jitm;
		jitm[U("day")] = json::value::string(src.day.c_str());
		jitm[U("start")] = json::value::string(src.start.c_str());
		jitm[U("high")] = json::value::string(src.high.c_str());
		jitm[U("low")] = json::value::string(src.low.c_str());
		jitm[U("end")] = json::value::string(src.end.c_str());
		jitm[U("quantity")] = json::value::string(src.quantity.c_str());

		switch (nIndexType)
		{
		case ApiMan::CHART_PRICE_MOVE_AVG:
			jitm[U("end_simple")] = json::value::string(src.value1.c_str());
			jitm[U("ma_10")] = json::value::string(src.value2.c_str());
			jitm[U("ma_20")] = json::value::string(src.value3.c_str());
			jitm[U("ma_60")] = json::value::string(src.value4.c_str());
			jitm[U("ma_120")] = json::value::string(src.value5.c_str());
			break;

		case ApiMan::CHART_MACD:
			jitm[U("macd_oscil")] = json::value::string(src.value1.c_str());
			jitm[U("macd_1226")] = json::value::string(src.value2.c_str());
			jitm[U("signal9")] = json::value::string(src.value3.c_str());
			break;

		case ApiMan::CHART_RSI:
			jitm[U("rsi14")] = json::value::string(src.value1.c_str());
			jitm[U("signal9")] = json::value::string(src.value2.c_str());
			break;
		}

		jarr.push_back(jitm);
	}

	body[U("items")] = json::value::array(jarr);

	http_client client(szURL);
	http_request req(methods::PUT);
	req.set_body(body);
	client.request(req).then([](task<http_response> responseTask)
	{
		try
		{
			http_response resp = responseTask.get();
			TRACE(L"PutKospiFutureChart result: %s", resp.extract_string().get().c_str());
		}
		catch (const http_exception &ex)
		{
			TRACE("PutKospiFutureChart failed: %s\n", ex.error_code().message().c_str());
		}
	});
}