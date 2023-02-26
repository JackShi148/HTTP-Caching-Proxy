#ifndef __TIMEPROCESSING_HPP__
#define __TIMEPROCESSING_HPP__

#include <cstdlib>
#include <cstdio>
#include <time.h>
#include <map>
#include <string.h>

class DatetimeMap
{
public:
    std::map<std::string, int> datetime_map;
    DatetimeMap()
    {
        datetime_map["Sun"] = 0;
        datetime_map["Mon"] = 1;
        datetime_map["Tue"] = 2;
        datetime_map["Wed"] = 3;
        datetime_map["Thu"] = 4;
        datetime_map["Fri"] = 5;
        datetime_map["Sat"] = 6;
        datetime_map["Jan"] = 0;
        datetime_map["Feb"] = 1;
        datetime_map["Mar"] = 2;
        datetime_map["Apr"] = 3;
        datetime_map["May"] = 4;
        datetime_map["Jun"] = 5;
        datetime_map["Jul"] = 6;
        datetime_map["Aug"] = 7;
        datetime_map["Sep"] = 8;
        datetime_map["Oct"] = 9;
        datetime_map["Nov"] = 10;
        datetime_map["Dec"] = 11;
    }
};

class TimeInfo
{
    struct tm timeinfo;

public:
    TimeInfo() {}
    void parse(std::string time_str)
    {
        DatetimeMap dt_map;
        this->timeinfo.tm_sec = std::stoi(time_str.substr(23), nullptr);
        this->timeinfo.tm_min = std::stoi(time_str.substr(20), nullptr);
        this->timeinfo.tm_hour = std::stoi(time_str.substr(17), nullptr);
        this->timeinfo.tm_wday = dt_map.datetime_map[time_str.substr(0, 3)];
        this->timeinfo.tm_mday = std::stoi(time_str.substr(5), nullptr);
        this->timeinfo.tm_mon = dt_map.datetime_map[time_str.substr(8, 3)];
        this->timeinfo.tm_year = std::stoi(time_str.substr(12)) - 1900;
        this->timeinfo.tm_isdst = 0;
    }
    struct tm * convertGMT() {
        time_t t = mktime(&this->timeinfo);
        struct tm * gmt = gmtime(&t);
        return gmt;
    }
};

#endif
