#ifndef __DEBUG_TRACE_H__
#define __DEBUG_TRACE_H__

///////////////////////////////////////////////////////////////////////////////
// Debug Information Trace Helper

#ifdef _DEBUG
	#include <assert.h>
	#include <crtdbg.h>

	#define ASSERT(f)			assert(f)
	#define VERIFY(f)			ASSERT(f)
	#define TRACE				_TRACE

	bool IsValidAddress(const void *p, UINT nBytes, BOOL bReadWrite =TRUE);
	bool IsValidString(LPCTSTR psz, int nLength =-1);
	void INIT_CRT_DEBUG_REPORT();
	TCHAR* GetLastErrorMsg();

	void _TRACE(LPCWSTR szFormat, ...);
	void _TRACE(LPCSTR szFormat, ...);
	
#else	// _DEBUG
	#define ASSERT(f)			((void)0)
	#define VERIFY(f)			((void)(f))
	#define TRACE				1 ? (void)0 : _TRACE
	
	#define IsValidString		1 ? (void)0 : _TRACE
	#define IsValidAddress		1 ? (void)0 : _TRACE
	#define INIT_CRT_DEBUG_REPORT()
	#define GetLastErrorMsg()	""
	
	static inline void _TRACE(LPCWSTR, ...) { }
	static inline void _TRACE(LPCSTR, ...) { }
#endif

void OutputDebugPrint(LPCWSTR szFormat, ...);

#endif