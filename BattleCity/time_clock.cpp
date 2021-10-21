#include "stdafx.h"
#include "time_clock.h"

namespace battlecity
{
TimeClock::TimeClock()
{
    QueryPerformanceFrequency(&li_tmp_);        // 获取时钟频率
    frequency_ = (double)li_tmp_.QuadPart;

    QueryPerformanceCounter(&li_tmp_);          // 获取时钟计数（频率与时间的乘积）
    quad_part_begin_ = li_tmp_.QuadPart;
}

TimeClock::~TimeClock()
{

}

void TimeClock::SetDrtTime(double time)
{
    drt_time_ = time;
}

void TimeClock::Timer1()
{
    QueryPerformanceCounter(&li_tmp_);
    quad_part_begin_ = li_tmp_.QuadPart;
}

void TimeClock::Init()
{
    QueryPerformanceCounter(&li_tmp_);
    quad_part_begin_ = li_tmp_.QuadPart;
    quad_part_end_ = li_tmp_.QuadPart;
}

bool TimeClock::IsTimeOut()
{
    QueryPerformanceCounter(&li_tmp_);
    quad_part_end_ = li_tmp_.QuadPart;

    if (((double)(quad_part_end_ - quad_part_begin_) * 1000) / frequency_ > drt_time_)
    {
        quad_part_begin_ = quad_part_end_;
        return true;
    }
    return false;
}

} // namespace battlecity