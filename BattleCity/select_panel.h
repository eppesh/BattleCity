#ifndef BATTLECITY_SELECT_PANEL_H_
#define BATTLECITY_SELECT_PANEL_H_

#include "stdafx.h"

namespace battlecity
{
// ָʾ���ѡ��
enum class SelectResult
{
    kOnePlayer = 1,                 // 1�����
    kTwoPlayer,                 // 2�����
    kCustom,                    // �Զ����ͼ
    kError
};

class SelectPanel
{
public:
    SelectPanel(HDC des_hdc, HDC image_hdc);
    ~SelectPanel();

    void Init();
    SelectResult ShowSelectPanel();         // ��ʾ���ѡ����壬������ҵ�ѡ��

private:
    HDC des_hdc_;
    HDC image_hdc_;

    // ����������ѡ����/˫��/�Զ��� ��Ϸ
    IMAGE select_player_image_;
    int select_player_image_y_;             // ͼƬ�ӵײ��������ƣ���ʼֵΪ672�����ڵײ�

    // ��ʼѡ ���1��2����ͼ
    byte counter_ : 1;                      // �����л�select_tank_image_�±�
    IMAGE select_tank_image_[2];
    POINT select_tank_point_[3];            // ����ѡ������
    int select_index_;                      // ѡ������0��1��2

    // ѡ����Һ���ʾ�ؿ���Ϣ
    IMAGE gray_background_image_;           // ��Ϸ��ɫ����ͼ
    IMAGE black_number_image_;              // ���� 1234567890 ͼƬ
};

} // namespace battlecity
#endif