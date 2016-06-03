#include "stdafx.h"
#include "XingGate.h"
#include "ApiMan.h"
#include "SvrMan.h"

#define IDW_LOGMSG		1234

LPCTSTR MainWnd::szClassName = _T("XingTest1");

#define WM_LOGIC_FLOW		(WM_APP+1)
enum
{
	FLOW_GET_SHCODE = 1,
	FLOW_LOGIN_XING,
	FLOW_REQUEST_CURRENT_PRICE,
	FLOW_TODO
};
#define PostLogicFLow(flowCode)	PostMessage(m_hWnd, WM_LOGIC_FLOW, flowCode, 0)

enum TimerIDs
{
	IDT_EXIT =1,
	IDT_TODO
};

#define PostTimer(_TimerID, _Millis)	SetTimer(m_hWnd, _TimerID, _Millis, NULL)

MainWnd::MainWnd()
{
	m_hWnd = NULL;
	m_hOutput = NULL;
}

bool MainWnd::Create(int nCmdShow)
{
	m_hWnd = CreateWindow(szClassName, _T("Xing Test"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 400, NULL, NULL, g_hInstance, (void*)this);

	if (!m_hWnd)
	{
		return false;
	}

	m_hOutput = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
		0, 0, 32, 32,
		m_hWnd, (HMENU)IDW_LOGMSG,
		g_hInstance, NULL);

	if (m_hOutput == NULL)
	{
		TRACE("RichEdit CreateWindow failed.\n");
		return false;
	}

	SendMessage(m_hOutput, EM_SETBKGNDCOLOR, 0, RGB(0, 0, 0));

	ShowWindow(m_hWnd, nCmdShow);
	UpdateWindow(m_hWnd);

	if (g_api->Create(m_hWnd, this))
	{
		PostLogicFLow(FLOW_GET_SHCODE);
	}

	return true;
}

void MainWnd::Destroy()
{
	g_api->Destroy();

	DestroyWindow(m_hOutput);
	DestroyWindow(m_hWnd);
}

void MainWnd::API_OnLoginResult(bool success)
{
	if (success)
	{
		//PostLogicFLow(FLOW_REQUEST_CURRENT_PRICE);
		PostMessage(m_hWnd, WM_COMMAND, IDM_API_CHART_PMA, 0);
		//PostMessage(m_hWnd, WM_COMMAND, IDM_API_CHART_MACD, 0);
		//PostMessage(m_hWnd, WM_COMMAND, IDM_API_CHART_RSI, 0);
	}
}

void MainWnd::API_OnLogout()
{
	LogI(L"Got Logout event");
}

void MainWnd::SVR_OnShcodeResult(LPCTSTR pszShcode)
{
	LogI(L"KOSPI200 Future shcode: %s", pszShcode);
	m_shcodeKospi200f = W2A(pszShcode);
	PostLogicFLow(FLOW_LOGIN_XING);
}

void MainWnd::LogMessage(COLORREF clr, LPCTSTR pszFormat, ...)
{
	TCHAR szBuffer[1024];

	va_list ap;
	va_start(ap, pszFormat);
	StringCbVPrintf(szBuffer, sizeof(szBuffer), pszFormat, ap);
	va_end(ap);

	CHARFORMAT2 cf;
	ZeroMemory(&cf, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_SIZE | CFM_FACE;//| CFM_OFFSET 
	cf.yHeight = 160;
	cf.crTextColor = clr;
	StringCchCopy(cf.szFaceName, LF_FACESIZE, _T("FixedSys"));

	SendMessage(m_hOutput, EM_SETSEL, -1, -1);
	SendMessage(m_hOutput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	SendMessage(m_hOutput, EM_REPLACESEL, FALSE, (LPARAM)szBuffer);

	SendMessage(m_hOutput, EM_SETSEL, -1, -1);
	SendMessage(m_hOutput, EM_REPLACESEL, FALSE, (LPARAM)_T("\r\n"));

	SendMessage(m_hOutput, EM_SCROLL, SB_PAGEDOWN, 0);
}

void MainWnd::ClearMessages()
{
	SetWindowText(m_hOutput, _T(""));
}

void MainWnd::OnCommand(WPARAM wParam)
{
	switch (LOWORD(wParam))
	{
	case IDM_EXIT:
		g_api->Logout();
		PostTimer(IDT_EXIT, 1500);
		break;

	case IDM_API_CHART_PMA:
		g_api->Request_ChartIndex(m_shcodeKospi200f.c_str(), ApiMan::CHART_PRICE_MOVE_AVG);
		break;
	case IDM_API_CHART_MACD:
		g_api->Request_ChartIndex(m_shcodeKospi200f.c_str(), ApiMan::CHART_MACD);
		break;
	case IDM_API_CHART_RSI:
		g_api->Request_ChartIndex(m_shcodeKospi200f.c_str(), ApiMan::CHART_RSI);
		break;
	}
}

void MainWnd::OnLogicFlow(WPARAM wParam)
{
	switch (LOWORD(wParam))
	{
	case FLOW_GET_SHCODE:
		g_svr->RequestKospiFutureShcode(this);
		break;

	case FLOW_LOGIN_XING:
		g_api->Login();
		break;

	case FLOW_REQUEST_CURRENT_PRICE:
		g_api->Request_T2101(m_shcodeKospi200f.c_str());
		break;
	}
}

bool MainWnd::OnTimer(UINT nTimerID)
{
	switch (nTimerID)
	{
	case IDT_EXIT:
		PostQuitMessage(0);
		break;
	}

	return false;	// kill timer
}

void MainWnd::OnSize(int cx, int cy)
{
	MoveWindow(m_hOutput, 0, 0, cx, cy, FALSE);
}

LRESULT CALLBACK MainWnd::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static MainWnd* pWnd;

	if (g_api->HandleMessage(message, wParam, lParam))
		return 0;

	switch (message)
	{
	case WM_TIMER:
		if (!pWnd->OnTimer(wParam))
		{
			KillTimer(hWnd, wParam);
		}
		break;

	case WM_COMMAND:
		pWnd->OnCommand(wParam);
		break;

	case WM_LOGIC_FLOW:
		pWnd->OnLogicFlow(wParam);
		break;

	case WM_SIZE:
		pWnd->OnSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case WM_CLOSE:
		pWnd->OnCommand(IDM_EXIT);
		break;

	case WM_CREATE:
		pWnd = (MainWnd*)((CREATESTRUCT*)lParam)->lpCreateParams;
		//pWnd->OnCreate();
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
