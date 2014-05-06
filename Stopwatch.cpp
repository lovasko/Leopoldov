#include "Stopwatch.h"

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif


ticks_t now()
{
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);

	return (ticks_t)mts.tv_sec * 1000000000UL + (ticks_t)mts.tv_nsec;
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
