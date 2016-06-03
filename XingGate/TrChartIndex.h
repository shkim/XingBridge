#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////
// 챠트지표데이터조회 ( HEADTYPE=B )
#pragma pack( push, 1 )

#define NAME_ChartIndex     "ChartIndex"

// In(*EMPTY*)                    
typedef struct _ChartIndexInBlock
{
	char    indexid[10];    // [long  ,   10] 지표ID                          StartPos 0, Length 10
	char    indexname[40];    // [string,   40] 지표명                          StartPos 10, Length 40
	char    indexparam[40];    // [string,   40] 지표조건설정                    StartPos 50, Length 40
	char    market[1];    // [string,    1] 시장구분                        StartPos 90, Length 1
	char    period[1];    // [string,    1] 주기구분                        StartPos 91, Length 1
	char    shcode[8];    // [string,    8] 단축코드                        StartPos 92, Length 8
	char    qrycnt[4];    // [long  ,    4] 요청건수(최대 500개)            StartPos 100, Length 4
	char    ncnt[4];    // [long  ,    4] 단위(n틱/n분)                   StartPos 104, Length 4
	char    sdate[8];    // [string,    8] 시작일자                        StartPos 108, Length 8
	char    edate[8];    // [string,    8] 종료일자                        StartPos 116, Length 8
	char    Isamend[1];    // [string,    1] 수정주가 반영 여부              StartPos 124, Length 1
	char    Isgab[1];    // [string,    1] 갭보정 여부                     StartPos 125, Length 1
	char    IsReal[1];    // [string,    1] 실시간 데이터수신 자동등록 여부 StartPos 126, Length 1
} ChartIndexInBlock, *LPChartIndexInBlock;
#define NAME_ChartIndexInBlock     "ChartIndexInBlock"

// Out(*EMPTY*)                   
typedef struct _ChartIndexOutBlock
{
	char    indexid[10];    // [long  ,   10] 지표ID                          StartPos 0, Length 10
	char    rec_cnt[5];    // [long  ,    5] 레코드갯수                      StartPos 10, Length 5
	char    validdata_cnt[2];    // [long  ,    2] 유효 데이터 컬럼 갯수           StartPos 15, Length 2
} ChartIndexOutBlock, *LPChartIndexOutBlock;
#define NAME_ChartIndexOutBlock     "ChartIndexOutBlock"

// Out(*EMPTY*)                   , occurs
typedef struct _ChartIndexOutBlock1
{
	char    date[8];    // [string,    8] 일자                            StartPos 0, Length 8
	char    time[6];    // [string,    6] 시간                            StartPos 8, Length 6
	char    open[10];    // [float ,   10] 시가                            StartPos 14, Length 10
	char    high[10];    // [float ,   10] 고가                            StartPos 24, Length 10
	char    low[10];    // [float ,   10] 저가                            StartPos 34, Length 10
	char    close[10];    // [float ,   10] 종가                            StartPos 44, Length 10
	char    volume[12];    // [float ,   12] 거래량                          StartPos 54, Length 12
	char    value1[10];    // [float ,   10] 지표값1                         StartPos 66, Length 10
	char    value2[10];    // [float ,   10] 지표값2                         StartPos 76, Length 10
	char    value3[10];    // [float ,   10] 지표값3                         StartPos 86, Length 10
	char    value4[10];    // [float ,   10] 지표값4                         StartPos 96, Length 10
	char    value5[10];    // [float ,   10] 지표값5                         StartPos 106, Length 10
	char    pos[8];    // [long  ,    8] 위치                            StartPos 116, Length 8
} ChartIndexOutBlock1, *LPChartIndexOutBlock1;
#define NAME_ChartIndexOutBlock1     "ChartIndexOutBlock1"

#pragma pack( pop )
///////////////////////////////////////////////////////////////////////////////////////////////////
