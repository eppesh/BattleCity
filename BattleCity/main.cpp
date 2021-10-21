// 名称：坦克大战（Battle City）
// 版权：仅供学习
// 作者：Sean (eppesh@163.com)
// 环境：VS2019;EasyX 2021
// 时间：2021/08/26
// 说明：模拟FC的坦克大战游戏；

// 感谢：xiongfj (https://codebus.cn/xiongfj/fc-tank 837943056@qq.com)

#include "stdafx.h"
#include "struct.h"
#include "select_panel.h"
#include "game_control.h"
#include "mci_sound.h"

using namespace battlecity;

int main()
{
    srand((unsigned)time(nullptr));
    bool is_custom_map = false;             // 自定义地图
    MciSound::InitSound();


    // 灰色背景
    initgraph(kWindowWidth, kWindowHeight);
    BeginBatchDraw();

    // 定义 image 画布
    IMAGE canvas_img(kCanvasWidth, kCanvasHeight);

    // 获取 graphics 绘图对象
    HDC des_hdc = GetImageHDC();
    HDC canvas_hdc = GetImageHDC(&canvas_img);

    SelectPanel *selecter = new SelectPanel(des_hdc, canvas_hdc);	// 显示玩家功能选择面板
    GameControl *control = NULL;
    SelectResult result;

    while (_kbhit() != 27)
    {
        result = selecter->ShowSelectPanel();		// 获取玩家选择结果

        if (!is_custom_map)
        {
            if (control != NULL)
            {
                delete control;
            }
            control = new GameControl(des_hdc, canvas_hdc);
        }

        switch (result)
        {
        case SelectResult::kOnePlayer:
            control->AddPlayer(kPlayerNumOne);		// 一个玩家

            if (is_custom_map)
            {
                control->GameLoop();
            }
            else
            {
                control->LoadMap();
                control->GameLoop();
            }
            is_custom_map = false;		// 自定义地图关卡过后自动进行后面的关卡
            break;

        case SelectResult::kTwoPlayer:
            control->AddPlayer(kPlayerNumTwo);		// 两个玩家

            if (is_custom_map)
            {
                control->GameLoop();
            }
            else
            {
                control->LoadMap();
                control->GameLoop();
            }
            is_custom_map = false;
            break;

        case SelectResult::kCustom:							// 玩家自定义地图
            control->CreateMap(&is_custom_map);
            break;

        default:
            return 0;
        }
    }

    closegraph();
    return 0;
}