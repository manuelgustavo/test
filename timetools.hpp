#ifndef TIME_TOOLS_HPP
#define TIME_TOOLS_HPP

#include <mutex>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdio>

// thanks to https://stackoverflow.com/a/36118726

std::mutex gmtime_call_mutex;

template< size_t For_Separating_Instantiations >
std::tm const * UtcTmImpl(std::chrono::system_clock::time_point const & tp)
{
	thread_local static std::tm tm = {};
    std::time_t const time = std::chrono::system_clock::to_time_t(tp);
#ifdef _MSC_VER // for VISUAL STUDIO compilers only
    gmtime_s(&tm, &time);
#else 
	{
		std::unique_lock< std::mutex > ul(gmtime_call_mutex);
		tm = *std::gmtime(&time);
	}
#endif
	return &tm;
}


template< size_t For_Separating_Instantiations >
std::tm const * LocalTmImpl(std::chrono::system_clock::time_point const & tp)
{
	thread_local static std::tm tm = {};
	std::time_t const time = std::chrono::system_clock::to_time_t(tp);
#ifdef _MSC_VER // for VISUAL STUDIO compilers only
    localtime_s(&tm, &time);
#else 
	{
		std::unique_lock< std::mutex > ul(gmtime_call_mutex);
		tm = *std::localtime(&time);
	}
#endif
	return &tm;
}

// converts a tm structure into a string
// outputs a string in the format YYYY/MM/DD-HH:MM:SS
std::string TmToString(const std::tm& t) {
	char buff[20];

	std::snprintf(buff, sizeof(buff), "%04d/%02d/%02d-%02d:%02d:%02d", (t.tm_year + 1900), (t.tm_mon + 1), t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

	return std::string(buff);
}

void TmToString(const std::tm& t, char *buff) {
	std::snprintf(buff, 20, "%04d/%02d/%02d-%02d:%02d:%02d", (t.tm_year + 1900), (t.tm_mon + 1), t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
}


#ifdef __COUNTER__
#define UTC_TM( arg ) UtcTmImpl<__COUNTER__>( (arg) )
#else
#define UTC_TM( arg ) UtcTmImpl<__LINE__>( (arg) )
#endif 

#ifdef __COUNTER__
#define LOCAL_TM( arg ) LocalTmImpl<__COUNTER__>( (arg) )
#else
#define LOCAL_TM( arg ) LocalTmImpl<__LINE__>( (arg) )
#endif

#endif // TIME_TOOLS_HPP