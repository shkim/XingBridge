#include "stdafx.h"
#include "XingGate.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")

HINSTANCE g_hInstance;
MainWnd g_mainwnd;
ApiMan* g_api = NULL;
SvrMan* g_svr = NULL;

static HMODULE s_hRichEdit;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	WNDCLASSEX wcex;

#ifdef _DEBUG
	INIT_CRT_DEBUG_REPORT();
	//_CrtSetBreakAlloc(290);
#endif

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainWnd::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_APIGATE);
	wcex.lpszClassName = MainWnd::szClassName;
	wcex.hIconSm = wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APIGATE));

	if (!RegisterClassEx(&wcex))
	{
		return -1;
	}

	g_hInstance = hInstance;

	//if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
	//	TRACE("AfxWinInit failed.\n");

	s_hRichEdit = LoadLibrary(_T("RICHED20.DLL"));

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
//	if (!AfxInitRichEdit2())
//		TRACE("AfxInitRichEdit failed.\n");

	g_api = new ApiMan();
	g_svr = new SvrMan();

	if (!g_mainwnd.Create(nCmdShow))
	{
		goto _finish;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_APIGATE));

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

_finish:
	g_mainwnd.Destroy();
	delete g_svr;
	delete g_api;
	FreeLibrary(s_hRichEdit);
	UnregisterClass(MainWnd::szClassName, hInstance);

	return 0;// (int)msg.wParam;
}
