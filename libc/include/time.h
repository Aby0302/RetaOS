#ifndef _TIME_H
#define _TIME_H

#include <sys/types.h>  // For time_t, size_t

// Basic time-related types
typedef long clock_t;     // CPU clock ticks
typedef int clockid_t;    // Clock IDs for clock_gettime family

// Clock ID constants for clock_gettime()
#define CLOCK_REALTIME            0
#define CLOCK_MONOTONIC           1
#define CLOCK_PROCESS_CPUTIME_ID  2
#define CLOCK_THREAD_CPUTIME_ID   3

// Time structure
struct tm {
    int tm_sec;     // Seconds (0-60)
    int tm_min;     // Minutes (0-59)
    int tm_hour;    // Hours (0-23)
    int tm_mday;    // Day of the month (1-31)
    int tm_mon;     // Month (0-11)
    int tm_year;    // Year - 1900
    int tm_wday;    // Day of the week (0-6, Sunday = 0)
    int tm_yday;    // Day in the year (0-365, 1 Jan = 0)
    int tm_isdst;   // Daylight saving time flag
};

// Timezone structure (for gettimeofday)
struct timezone {
    int tz_minuteswest;  // Minutes west of GMT
    int tz_dsttime;      // Type of DST correction
};

// Time value structure (for gettimeofday)
struct timeval {
    time_t      tv_sec;   // Seconds
    suseconds_t tv_usec;  // Microseconds
};

// Timespec structure (for nanosleep, clock_gettime)
struct timespec {
    time_t tv_sec;   // Seconds
    long   tv_nsec;  // Nanoseconds (0 - 999,999,999)
};

// Time manipulation functions
clock_t clock(void);
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm *timeptr);
time_t time(time_t *timer);
int timespec_get(struct timespec *ts, int base);

// Time conversion functions
char *asctime(const struct tm *timeptr);
char *ctime(const time_t *timer);
struct tm *gmtime(const time_t *timer);
struct tm *localtime(const time_t *timer);
size_t strftime(char *s, size_t maxsize, const char *format, const struct tm *timeptr);

// POSIX.1b extensions
int clock_getres(clockid_t clock_id, struct timespec *res);
int clock_gettime(clockid_t clock_id, struct timespec *tp);
int clock_settime(clockid_t clock_id, const struct timespec *tp);
int nanosleep(const struct timespec *req, struct timespec *rem);

// Non-standard but common extensions
time_t timegm(struct tm *tm);
int gettimeofday(struct timeval *tv, struct timezone *tz);

// Timezone and daylight saving time
extern long timezone;  // Seconds west of UTC
extern int daylight;   // Nonzero if daylight saving time is in effect

extern char *tzname[2];  // Timezone names

// For compatibility with older code
#define CLK_TCK CLOCKS_PER_SEC

// Constants
#define CLOCKS_PER_SEC 1000000  // Microseconds per second

// For C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

// Additional POSIX functions
int stime(const time_t *t);
void tzset(void);

// BSD extensions
int adjtime(const struct timeval *delta, struct timeval *olddelta);
int settimeofday(const struct timeval *tv, const struct timezone *tz);

#ifdef __cplusplus
}
#endif

#endif /* _TIME_H */
