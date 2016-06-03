#pragma once

#include "resource.h"
#include "ApiMan.h"
#include "SvrMan.h"

#define LogD(_fmt, ...)		g_mainwnd.LogMessage(RGB(128,128,128), _fmt, __VA_ARGS__)
#define LogI(_fmt, ...)		g_mainwnd.LogMessage(RGB(0,255,0), _fmt, __VA_ARGS__)
#define LogE(_fmt, ...)		g_mainwnd.LogMessage(RGB(255,0,0), _fmt, __VA_ARGS__)

class MainWnd : public ApiListener, public SvrListener
{
public:
	MainWnd();

	bool Create(int nCmdShow);
	void Destroy();

	void LogMessage(COLORREF clr, LPCTSTR pszFormat, ...);

	static LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
	static LPCTSTR szClassName;

private:
	void OnCommand(WPARAM wParam);
	void OnLogicFlow(WPARAM wParam);
	void OnSize(int cx, int cy);
	bool OnTimer(UINT nTimerID);
	
	virtual void API_OnLoginResult(bool success);
	virtual void API_OnLogout();

	virtual void SVR_OnShcodeResult(LPCTSTR pszShcode);

	void ClearMessages();

	HWND m_hWnd;
	HWND m_hOutput;

	std::string m_shcodeKospi200f;
};

extern MainWnd g_mainwnd;
extern HINSTANCE g_hInstance;
