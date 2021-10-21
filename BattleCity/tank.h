#ifndef BATTLECITY_TANK_H_
#define BATTLECITY_TANK_H_

#include "stdafx.h"

namespace battlecity
{

// ����̹����Ϣ���������/�л�̹����Ϣ��
class TankInfo
{
public:
    TankInfo(byte tank_color, int enemy_level, bool notuse);	// tank_color [0-3]; enemy_level [0-4], notuse->�������̹��
    TankInfo(byte player, byte level);							// player [0-1]; level [0-3]
    ~TankInfo();

    IMAGE GetTankImage(byte dir, int index = 0);		        // index ���Ų�ͬ������ͼƬ���� [0-1]
private:
    IMAGE tank_image_[4][2];			                        // �ĸ�����,һ������������ͼʵ��̹���ƶ�����.
};

// ���̹�˼���
class PlayerTank
{
public:
    PlayerTank(byte player);	                                // player [0-1]
    ~PlayerTank();
    IMAGE GetTankImage(byte level, byte dir, bool moving = false);

private:
    TankInfo *tank_info_[4];		                            // 4 ������̹��
    byte move_index_ : 1;		                                // ����̹��ͼƬ���һ�������� �±�����,ֻռ 1 bit �ڴ棬ֵֻ��Ϊ 0��1
};
}   // namespace battlecity

#endif
