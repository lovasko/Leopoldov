

#include "Stopwatch.h"


ticks_t now()
{
#ifdef _WIN32
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return (ticks_t)ticks.QuadPart;
#else
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (ticks_t)ts.tv_sec * 1000000000UL + (ticks_t)ts.tv_nsec;
#endif
}


time_sec ticks_to_time(ticks_t ticks_diff)
{
#ifdef _WIN32
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return (time_sec)(ticks_diff) / (time_sec)(freq.QuadPart);
#else
	return (time_sec)(ticks_diff)*1E-9;
#endif
}
