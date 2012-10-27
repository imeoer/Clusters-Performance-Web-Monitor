#pragma once

//使用性能计数器
#include "Pdh.h"
#pragma comment ( lib , "Pdh.lib" )

class CPerformanceCounter
{
	public:
		CPerformanceCounter(CString strCounter);
		~CPerformanceCounter(void);
		double GetCurrentValue();
	private:
		//性能计数器
		PDH_HQUERY hQuery;
		PDH_HCOUNTER hTotal;
};

