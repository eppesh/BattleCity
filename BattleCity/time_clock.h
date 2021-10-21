#ifndef BATTLECITY_TIME_CLOCK_H_
#define BATTLECITY_TIME_CLOCK_H_

namespace battlecity
{
// Timer1() 开始调用一次
// Timer2() 循环调用
// IsTimeOut() 循环调用，检测时间差是否大于设置的值
class TimeClock
{
public:
    TimeClock();
    ~TimeClock();

    // 设置时间差，调节速度
    void SetDrtTime(double time);

    // 更新 QPart1
    void Timer1();

    void Init();                            // 重置 t1 = t2

    // 比较两个计时时间差，返回true/false, 并重置时间差
    bool IsTimeOut();                       // 是否时间差已到

private:
    LARGE_INTEGER li_tmp_;                  // 定义该结构体的一个对象，用于获取精确时间
    LONGLONG quad_part_begin_;              // 用于获取结构体里面的 QuadPart 成员
    LONGLONG quad_part_end_;
    double frequency_;                      // 时钟频率
    double drt_time_;                       // 定义时间差，决定物体运动速度
};

} // namespace battlecity

#endif
