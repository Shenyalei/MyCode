#include "Common.h"
#include "HRTimer.h"
#include <stdio.h>


LONG64 HRtimer::GetFrequency()
{
	LARGE_INTEGER proc_freq;
	::QueryPerformanceFrequency(&proc_freq);
	return proc_freq.QuadPart;
}
void HRtimer::Start()
{
	::QueryPerformanceCounter(&start);
}
void HRtimer::Stop()
{
	::QueryPerformanceCounter(&stop);
	LONGLONG timeDiff = stop.QuadPart - start.QuadPart;
	LONGLONG timeDiff2 = timeDiff * 1000 * 1000 / frequency;
	printf("HRtimer time %lld\n", timeDiff2);
}
LONG64 HRtimer::frequency = HRtimer::GetFrequency();