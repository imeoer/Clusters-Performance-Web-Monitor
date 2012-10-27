#include "StdAfx.h"
#include "PerformanceCounter.h"


CPerformanceCounter::CPerformanceCounter(CString strCounter)
{
	PdhOpenQuery(NULL, NULL, &hQuery);
	PdhAddCounter(hQuery, strCounter, NULL, &hTotal);
	PdhCollectQueryData(hQuery);
}

CPerformanceCounter::~CPerformanceCounter(void)
{
}

double CPerformanceCounter::GetCurrentValue()
{
	PDH_FMT_COUNTERVALUE counterVal;
	PdhCollectQueryData(hQuery);
	PdhGetFormattedCounterValue(hTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	return counterVal.doubleValue;
}