#include "stdafx.h"
#include "tank.h"
#include "struct.h"

namespace battlecity
{
// �л�̹����Ϣ
TankInfo::TankInfo(byte tank_color, int enemy_level, bool notuse)
{
    TCHAR temp[100];
    switch (tank_color)
    {
    case kGrayTank:		// ��ɫ̹��
        for (int i = 0; i < 4; i++)
        {
            _stprintf_s(temp, L"./res/big/gray-tank/%d-%d-1.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][0], temp);
            _stprintf_s(temp, L"./res/big/gray-tank/%d-%d-2.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][1], temp);
        }
        break;
    case kRedTank:		// ��ɫ̹��
        for (int i = 0; i < 4; i++)
        {
            _stprintf_s(temp, L"./res/big/red-tank/%d-%d-1.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][0], temp);
            _stprintf_s(temp, L"./res/big/red-tank/%d-%d-2.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][1], temp);
        }
        break;
    case kGreenTank:    // ��ɫ̹��
        for (int i = 0; i < 4; i++)
        {
            _stprintf_s(temp, L"./res/big/other-tank/g%d-%d-1.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][0], temp);
            _stprintf_s(temp, L"./res/big/other-tank/g%d-%d-2.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][1], temp);
        }
        break;
    case kYellowTank:   // ��ɫ̹��
        for (int i = 0; i < 4; i++)
        {
            _stprintf_s(temp, L"./res/big/other-tank/y%d-%d-1.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][0], temp);
            _stprintf_s(temp, L"./res/big/other-tank/y%d-%d-2.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][1], temp);
        }
        break;
    default:
        throw _T("�쳣 TankInfo::TankInfo(byte enemy_kind, int enemy_level)");
    }
}

// ���̹����Ϣ
TankInfo::TankInfo(byte player, byte level)
{
    switch (player)
    {
    case 0:
    case 1:
    {
        TCHAR temp[50];
        for (int i = 0; i < 4; i++)
        {
            _stprintf_s(temp, L"./res/big/%dPlayer/m%d-%d-1.gif", player, level, i);
            loadimage(&tank_image_[i][0], temp);
            _stprintf_s(temp, L"./res/big/%dPlayer/m%d-%d-2.gif", player, level, i);
            loadimage(&tank_image_[i][1], temp);
        }
    }
    break;
    default:
        throw _T("��ֵԽ��, TankClass.cpp-> TankInfo construct function");
    }
}

TankInfo::~TankInfo()
{
}

IMAGE TankInfo::GetTankImage(byte dir, int index)
{
    return tank_image_[dir][index];
}

/////////////////////////////////////////////////////////
// ���̹�˼���
PlayerTank::PlayerTank(byte player)
{
    move_index_ = 0;
    for (int i = 0; i < 4; i++)
    {
        tank_info_[i] = new TankInfo(player, i);
    }
}

PlayerTank::~PlayerTank()
{
    for (int i = 0; i < 4; i++)
    {
        delete tank_info_[i];
    }
    printf("PlayerTank::~PlayerTank() \n");
}

// �����������, ÿ���θ���һ������
IMAGE PlayerTank::GetTankImage(byte level, byte dir, bool moving)
{							// ֻ�ḳֵһ��
    move_index_ = moving ? move_index_ + 1 : 0;
    return tank_info_[level]->GetTankImage(dir, move_index_);
}

} // namespace battlecity