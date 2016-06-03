#include "stdafx.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

#ifdef _DEBUG

/*
#pragma warning(disable:4074)//initializers put in compiler reserved initialization area
#pragma init_seg(compiler)//global objects in this file get constructed very early on

struct CrtBreakAllocSetter {
	CrtBreakAllocSetter() {
		_crtBreakAlloc = 290;

		_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	}
};

CrtBreakAllocSetter g_crtBreakAllocSetter;
*/

void INIT_CRT_DEBUG_REPORT()
{
/*
	int	DbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
//	DbgFlag |= _CRTDBG_LEAK_CHECK_DF;
//		DbgFlag |= _CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF;
		DbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
//		DbgFlag &= ~_CRTDBG_CHECK_ALWAYS_DF;
	_CrtSetDbgFlag(DbgFlag);
*/
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
}

bool IsValidAddress(const void *p, UINT nBytes, BOOL bReadWrite)
{
	return _CrtIsValidPointer(p, nBytes, bReadWrite) == TRUE;
}

bool IsValidString(LPCTSTR psz, int nLength)
{
	if(psz == NULL)	return false;
	return (::IsBadStringPtr(psz, nLength) == 0);
}

#ifdef _UNICODE

/*static void _OutputCounter()
{
	static int s_counter = 0;
	char buff[16];
	StringCchPrintfA(buff, 16, "%d:", ++s_counter);
	OutputDebugStringA(buff);
}*/

void _TRACE(LPCWSTR szFormat, ...)
{
	WCHAR szBuffer[1024];

	va_list ap;
	va_start(ap, szFormat);
	
	StringCbVPrintfW(szBuffer, sizeof(szBuffer), szFormat, ap);

	va_end(ap);

//	_OutputCounter();
	OutputDebugStringW(szBuffer);
}

void _TRACE(LPCSTR pszFormat, ...)
{
	CHAR szBuffer[1024];
	
	va_list ap;
	va_start(ap, pszFormat);
	
	StringCbVPrintfA(szBuffer, sizeof(szBuffer), pszFormat, ap);
	
	va_end(ap);
	
//	_OutputCounter();
	OutputDebugStringA(szBuffer);
}

#else

void _TRACE(LPCTSTR szFormat, ...)
{
	TCHAR szBuffer[1024];
	
	va_list ap;
	va_start(ap, szFormat);
	
	_vsntprintf(szBuffer, sizeof(szBuffer), szFormat, ap);
	
	va_end(ap);
	
	OutputDebugString(szBuffer);
}

#endif	// _UNICODE

TCHAR* GetLastErrorMsg()
{
	static TCHAR buff[256];

	FormatMessage( 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		buff, sizeof(buff), NULL);

	return buff;
}

#endif

void OutputDebugPrint(LPCWSTR szFormat, ...)
{
	WCHAR szBuffer[1024];

	va_list ap;
	va_start(ap, szFormat);
	
	StringCbVPrintfW(szBuffer, sizeof(szBuffer), szFormat, ap);

	va_end(ap);

	OutputDebugStringW(szBuffer);
}
