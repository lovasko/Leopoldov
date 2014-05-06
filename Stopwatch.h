#ifndef STOPWATCH_H
#define STOPWATCH_H


#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#else
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>
#endif


typedef unsigned long long ticks_t;
typedef double time_sec;


ticks_t now();


time_sec ticks_to_time(ticks_t ticks_diff);


#endif
