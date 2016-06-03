#include "stdafx.h"
#include "util.h"

#define CP_949		949

#define MAX_A2W_LENGTH		1024

WCHAR a2w_buff[MAX_A2W_LENGTH];

void* GetTempBuffer()
{
	return a2w_buff;
}

LPWSTR A2W(LPCSTR pszSrc)
{
	MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, a2w_buff, MAX_A2W_LENGTH);
	return a2w_buff;
}

LPSTR W2A(LPCWSTR pszSrc)
{
	char* buff = (char*)a2w_buff;
	WideCharToMultiByte(CP_ACP, 0, pszSrc, -1, buff, MAX_A2W_LENGTH, NULL, NULL);
	return buff;
}

LPSTR W2K(LPCWSTR pszSrc)
{
	char* buff = (char*)a2w_buff;
	WideCharToMultiByte(CP_949, 0, pszSrc, -1, buff, MAX_A2W_LENGTH, NULL, NULL);
	return buff;
}

std::wstring KRtoWide(const char* pKrStr, int len)
{
	// Trim Right
	while (len > 0)
	{
		int ch = pKrStr[len - 1];
		if (ch == ' ' || ch == 0)
			--len;
		else
			break;
	}

	int nReq = MultiByteToWideChar(CP_949, 0, pKrStr, len, NULL, 0);
	int cbAlloc = ++nReq * sizeof(WCHAR);
	WCHAR* pBuff = (WCHAR*)_malloca(cbAlloc);
	int wlen = MultiByteToWideChar(CP_949, 0, pKrStr, len, pBuff, nReq);
	ASSERT(wlen >= 0);
	std::wstring ret(pBuff, &pBuff[wlen]);
	_freea(pBuff);

	return ret;
}

std::string KRtoAnsi(const char* pKrStr, int len)
{
	// Trim Right
	while (len > 0)
	{
		int ch = pKrStr[len - 1];
		if (ch == ' ' || ch == 0)
			--len;
		else
			break;
	}

	if (len <= 0)
		return pKrStr;

	std::string ret(pKrStr, &pKrStr[len]);
	return ret;
}

double ParseFloat(const char* pData, int nDataLen, int nDotPos)
{
	char buff[32];
	ASSERT(nDataLen < 32 && nDotPos < nDataLen);

	char* pBuff = buff;
	int nPreDotLen = nDataLen - nDotPos;
	int i = 0;
	for (; i < nPreDotLen; i++)
	{
		*pBuff++ = pData[i];
	}
	*pBuff++ = '.';

	for (; i < nDataLen; i++)
	{
		*pBuff++ = pData[i];;
	}
	*pBuff++ = 0;

	return atof(buff);
}

int ParseInteger(const char* pData, int nDataLen)
{
	char buff[32];
	ASSERT(nDataLen < 32);

	memcpy(buff, pData, nDataLen);
	buff[nDataLen] = 0;

	return atoi(buff);
}
