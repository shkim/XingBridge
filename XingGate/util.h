#pragma once

#define USES_CONVERSION

class CString
{
public:
	CString(const char*);
	CString(const WCHAR*);
};

LPWSTR A2W(LPCSTR pszSrc);
LPSTR W2A(LPCWSTR pszSrc);

LPSTR W2K(LPCWSTR pszSrc);

std::wstring KRtoWide(const char* pKrStr, int len = -1);
std::string KRtoAnsi(const char* pKrStr, int len = -1);

double ParseFloat(const char* pData, int nDataLen, int nDotPos);
int ParseInteger(const char* pData, int nDataLen);
