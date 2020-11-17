#include <cstdio>
#include <cstdint>
#include <ctime>

#include "ppsspp_config.h"

#include "Common/TimeUtil.h"

#ifdef HAVE_LIBNX
;fdksa;lfk;asldkf;aslkdf;laskdf;laskdf;lkdgh kdf;hk ;fgk erpthjerjgblsdkv ;sdl
#include <switch.h>
#endif // HAVE_LIBNX

#include <sys/time.h>
#include <unistd.h>

static double curtime = 0;

double time_now_d() {
	static time_t start;
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	if (start == 0) {
		start = tv.tv_sec;
	}
	return (double)(tv.tv_sec - start) + (double)tv.tv_usec * (1.0 / 1000000.0);
}

void sleep_ms(int ms) {

	usleep(ms * 1000);

}

// Return the current time formatted as Minutes:Seconds:Milliseconds
// in the form 00:00:000.
void GetTimeFormatted(char formattedTime[13]) {
	time_t sysTime;
	struct tm * gmTime;
	char tmp[13];

	time(&sysTime);
	gmTime = localtime(&sysTime);

	strftime(tmp, 6, "%M:%S", gmTime);

	// Now tack on the milliseconds
	struct timeval t;
	(void)gettimeofday(&t, NULL);
	snprintf(formattedTime, 13, "%s:%03d", tmp, (int)(t.tv_usec / 1000));

}
