#include "Timestamp.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdio.h>

using namespace std;

static_assert(sizeof(Timestamp) == sizeof(int64_t), "sizeof(Timestamp) error");

string Timestamp::toString() const
{
	time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
	struct tm tm_time;


	struct tm* ptm;
	ptm = localtime(&seconds);
	tm_time = *ptm;

	char buf[32] = { 0 };


	int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);

	snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
			tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
			microseconds);

	return buf;
}

Timestamp Timestamp::now()
{
	chrono::time_point<chrono::system_clock,chrono::microseconds> now = chrono::time_point_cast<chrono::microseconds>(
		chrono::system_clock::now());

	int64_t microSeconds = now.time_since_epoch().count();
	Timestamp time(microSeconds);

	return time;
}
