#pragma once

#include <stdint.h>
#include <algorithm>
#include <string>

class Timestamp
{
public:
	Timestamp() : microSecondsSinceEpoch_(0)
	{
	}

	explicit Timestamp(int64_t microSecondsSinceEpoch)
		: microSecondsSinceEpoch_(microSecondsSinceEpoch)
	{
	};
	//20200626 19:43:30.904104
	std::string toString() const;

	//内部接口
	int64_t microSecondsSinceEpoch() const 
	{ 
		return microSecondsSinceEpoch_; 
	}

	time_t secondsSinceEpoch() const
	{
		return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
	}

	//Get time of now
	static Timestamp now();
	
	static Timestamp addTime(Timestamp timestamp, int64_t microseconds)
	{
		//int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
		return Timestamp(timestamp.microSecondsSinceEpoch() + microseconds);
	}

	static Timestamp addTime(Timestamp lhs, Timestamp rhs)
	{
		//int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
		return Timestamp(lhs.microSecondsSinceEpoch() + rhs.microSecondsSinceEpoch());
	}

	static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
	int64_t     microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator>(Timestamp lhs, Timestamp rhs)
{
	return rhs < lhs;
}

