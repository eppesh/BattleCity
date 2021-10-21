#ifndef BATTLECITY_SELECT_PANEL_H_
#define BATTLECITY_SELECT_PANEL_H_

#include "stdafx.h"

namespace battlecity
{
// 指示玩家选择
enum class SelectResult
{
    kOnePlayer = 1,                 // 1个玩家
    kTwoPlayer,                 // 2个玩家
    kCustom,                    // 自定义地图
    kError
};

class SelectPanel
{
public:
    SelectPanel(HDC des_hdc, HDC image_hdc);
    ~SelectPanel();

    void Init();
    SelectResult ShowSelectPanel();         // 显示玩家选择面板，返回玩家的选择

private:
    HDC des_hdc_;
    HDC image_hdc_;

    // 上升动画；选择单人/双人/自定义 游戏
    IMAGE select_player_image_;
    int select_player_image_y_;             // 图片从底部上升控制，初始值为672，窗口底部

    // 开始选 玩家1、2、地图
    byte counter_ : 1;                      // 用来切换select_tank_image_下标
    IMAGE select_tank_image_[2];
    POINT select_tank_point_[3];            // 三个选项坐标
    int select_index_;                      // 选择结果：0，1，2

    // 选择玩家后显示关卡信息
    IMAGE gray_background_image_;           // 游戏灰色背景图
    IMAGE black_number_image_;              // 数字 1234567890 图片
};

} // namespace battlecity
#endif