#include "stdafx.h"
#include "tank.h"
#include "struct.h"

namespace battlecity
{
// 敌机坦克信息
TankInfo::TankInfo(byte tank_color, int enemy_level, bool notuse)
{
    TCHAR temp[100];
    switch (tank_color)
    {
    case kGrayTank:		// 灰色坦克
        for (int i = 0; i < 4; i++)
        {
            _stprintf_s(temp, L"./res/big/gray-tank/%d-%d-1.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][0], temp);
            _stprintf_s(temp, L"./res/big/gray-tank/%d-%d-2.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][1], temp);
        }
        break;
    case kRedTank:		// 红色坦克
        for (int i = 0; i < 4; i++)
        {
            _stprintf_s(temp, L"./res/big/red-tank/%d-%d-1.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][0], temp);
            _stprintf_s(temp, L"./res/big/red-tank/%d-%d-2.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][1], temp);
        }
        break;
    case kGreenTank:    // 绿色坦克
        for (int i = 0; i < 4; i++)
        {
            _stprintf_s(temp, L"./res/big/other-tank/g%d-%d-1.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][0], temp);
            _stprintf_s(temp, L"./res/big/other-tank/g%d-%d-2.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][1], temp);
        }
        break;
    case kYellowTank:   // 黄色坦克
        for (int i = 0; i < 4; i++)
        {
            _stprintf_s(temp, L"./res/big/other-tank/y%d-%d-1.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][0], temp);
            _stprintf_s(temp, L"./res/big/other-tank/y%d-%d-2.gif", enemy_level + 1, i + 1);
            loadimage(&tank_image_[i][1], temp);
        }
        break;
    default:
        throw _T("异常 TankInfo::TankInfo(byte enemy_kind, int enemy_level)");
    }
}

// 玩家坦克信息
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
        throw _T("数值越界, TankClass.cpp-> TankInfo construct function");
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
// 玩家坦克级别
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

// 定义随机索引, 每三次跟换一个动作
IMAGE PlayerTank::GetTankImage(byte level, byte dir, bool moving)
{							// 只会赋值一次
    move_index_ = moving ? move_index_ + 1 : 0;
    return tank_info_[level]->GetTankImage(dir, move_index_);
}

} // namespace battlecity