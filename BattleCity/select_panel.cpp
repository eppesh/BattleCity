#include "select_panel.h"
#include "stdafx.h"
#include "game_control.h"


namespace battlecity
{
SelectPanel::SelectPanel(HDC des_hdc, HDC image_hdc)
{
    des_hdc_ = des_hdc;
    image_hdc_ = image_hdc;

    loadimage(&select_player_image_, _T("./res/big/select_player.gif"));

    // 选择坦克手柄游标
    loadimage(&select_tank_image_[0], _T("./res/big/0Player/m0-2-1.gif"));
    loadimage(&select_tank_image_[1], _T("./res/big/0Player/m0-2-2.gif"));

    // 灰色背景
    loadimage(&gray_background_image_, _T("./res/big/bg_gray.gif"));
    loadimage(&black_number_image_, _T("./res/big/black-number.gif"));

    select_tank_point_[0].x = 60;
    select_tank_point_[1].x = 60;
    select_tank_point_[2].x = 60;
    select_tank_point_[0].y = 123;
    select_tank_point_[1].y = 139;
    select_tank_point_[2].y = 156;

    Init();
}

SelectPanel::~SelectPanel()
{

}

void SelectPanel::Init()
{
    select_player_image_y_ = kWindowHeight;
    select_index_ = 1;
    counter_ = 1;
}

// 显示游戏开始时的控制面板
SelectResult SelectPanel::ShowSelectPanel()
{
    // 防止游戏失败分数面板后显示 GameOver 残留屏幕
    cleardevice();

    // 选择玩家面板 上升动画
    while (select_player_image_y_ > 0)
    {
        // 如果上升过程按下回车键，直接结束动画，显示最后定格的页面
        if (GetAsyncKeyState(VK_RETURN) & 0x8000)
        {
            select_player_image_y_ = 0;
        }

        Sleep(55);
        select_player_image_y_ -= 5;
        if (select_player_image_y_ < 0)
        {
            select_player_image_y_ = 0;
        }

        // 绘制在 image_hdc__上
        BitBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, GetImageHDC(&select_player_image_), 0, 0, SRCCOPY);

        // 将 image_hdc_ 绘制到主窗口 des_hdc_ 上
        StretchBlt(des_hdc_, 0, select_player_image_y_, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
        // 输出从 main.cpp 中的 BeginBatchDraw() 开始到此处的缓存绘图，才能显示到窗口；目的：避免闪烁
        FlushBatchDraw();
    }

    // 控制按键响应速度，不能太快
    int temp = 0;

    // 玩家可以进行 上下、回车 操作，选择游戏模式
    // 玩家开始选择游戏功能
    while (true)
    {
        Sleep(40);
        counter_++;

        BitBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, GetImageHDC(&select_player_image_), 0, 0, SRCCOPY);

        TransparentBlt(image_hdc_, select_tank_point_[select_index_].x, select_tank_point_[select_index_].y,
            16, 16, GetImageHDC(&select_tank_image_[counter_]), 0, 0, 16, 16, 0x000000);

        StretchBlt(des_hdc_, 0, select_player_image_y_, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
        FlushBatchDraw();

        temp++;
        if (GetAsyncKeyState(VK_UP) & 0x8000 && temp > 3)
        {
            temp = 0;
            select_index_ = (select_index_ - 1 >= 0) ? select_index_ - 1 : 2;
        }
        else if (GetAsyncKeyState(VK_DOWN) & 0x8000 && temp > 3)
        {
            temp = 0;
            select_index_ = (select_index_ + 1 < 3) ? select_index_ + 1 : 0;
        }
        else if (GetAsyncKeyState(VK_RETURN) & 0x8000 && temp > 3)
        {
            temp = 0;
            break;
        }
    }

    // 响应玩家的选择
    switch (select_index_)
    {
    case 0:
        // 重置数据，下次才能上升
        Init();
        return SelectResult::kOnePlayer;
    case 1:
        // 重置数据，下次才能上升
        Init();
        return SelectResult::kTwoPlayer;
    case 2:
        // 重置数据，下次才能上升
        Init();
        return SelectResult::kCustom;
    default:
        throw _T("void SelectPanel::ShowSelectPanel() 异常");
    }
    return SelectResult::kError;
}
} // namespace battlecity