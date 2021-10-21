#ifndef BATTLECITY_TIME_CLOCK_H_
#define BATTLECITY_TIME_CLOCK_H_

namespace battlecity
{
// Timer1() ��ʼ����һ��
// Timer2() ѭ������
// IsTimeOut() ѭ�����ã����ʱ����Ƿ�������õ�ֵ
class TimeClock
{
public:
    TimeClock();
    ~TimeClock();

    // ����ʱ�������ٶ�
    void SetDrtTime(double time);

    // ���� QPart1
    void Timer1();

    void Init();                            // ���� t1 = t2

    // �Ƚ�������ʱʱ������true/false, ������ʱ���
    bool IsTimeOut();                       // �Ƿ�ʱ����ѵ�

private:
    LARGE_INTEGER li_tmp_;                  // ����ýṹ���һ���������ڻ�ȡ��ȷʱ��
    LONGLONG quad_part_begin_;              // ���ڻ�ȡ�ṹ������� QuadPart ��Ա
    LONGLONG quad_part_end_;
    double frequency_;                      // ʱ��Ƶ��
    double drt_time_;                       // ����ʱ�����������˶��ٶ�
};

} // namespace battlecity

#endif
