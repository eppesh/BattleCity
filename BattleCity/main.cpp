// ���ƣ�̹�˴�ս��Battle City��
// ��Ȩ������ѧϰ
// ���ߣ�Sean (eppesh@163.com)
// ������VS2019;EasyX 2021
// ʱ�䣺2021/08/26
// ˵����ģ��FC��̹�˴�ս��Ϸ��

// ��л��xiongfj (https://codebus.cn/xiongfj/fc-tank 837943056@qq.com)

#include "stdafx.h"
#include "struct.h"
#include "select_panel.h"
#include "game_control.h"
#include "mci_sound.h"

using namespace battlecity;

int main()
{
    srand((unsigned)time(nullptr));
    bool is_custom_map = false;             // �Զ����ͼ
    MciSound::InitSound();


    // ��ɫ����
    initgraph(kWindowWidth, kWindowHeight);
    BeginBatchDraw();

    // ���� image ����
    IMAGE canvas_img(kCanvasWidth, kCanvasHeight);

    // ��ȡ graphics ��ͼ����
    HDC des_hdc = GetImageHDC();
    HDC canvas_hdc = GetImageHDC(&canvas_img);

    SelectPanel *selecter = new SelectPanel(des_hdc, canvas_hdc);	// ��ʾ��ҹ���ѡ�����
    GameControl *control = NULL;
    SelectResult result;

    while (_kbhit() != 27)
    {
        result = selecter->ShowSelectPanel();		// ��ȡ���ѡ����

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
            control->AddPlayer(kPlayerNumOne);		// һ�����

            if (is_custom_map)
            {
                control->GameLoop();
            }
            else
            {
                control->LoadMap();
                control->GameLoop();
            }
            is_custom_map = false;		// �Զ����ͼ�ؿ������Զ����к���Ĺؿ�
            break;

        case SelectResult::kTwoPlayer:
            control->AddPlayer(kPlayerNumTwo);		// �������

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

        case SelectResult::kCustom:							// ����Զ����ͼ
            control->CreateMap(&is_custom_map);
            break;

        default:
            return 0;
        }
    }

    closegraph();
    return 0;
}