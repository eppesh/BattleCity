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

    // ѡ��̹���ֱ��α�
    loadimage(&select_tank_image_[0], _T("./res/big/0Player/m0-2-1.gif"));
    loadimage(&select_tank_image_[1], _T("./res/big/0Player/m0-2-2.gif"));

    // ��ɫ����
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

// ��ʾ��Ϸ��ʼʱ�Ŀ������
SelectResult SelectPanel::ShowSelectPanel()
{
    // ��ֹ��Ϸʧ�ܷ���������ʾ GameOver ������Ļ
    cleardevice();

    // ѡ�������� ��������
    while (select_player_image_y_ > 0)
    {
        // ����������̰��»س�����ֱ�ӽ�����������ʾ��󶨸��ҳ��
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

        // ������ image_hdc__��
        BitBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, GetImageHDC(&select_player_image_), 0, 0, SRCCOPY);

        // �� image_hdc_ ���Ƶ������� des_hdc_ ��
        StretchBlt(des_hdc_, 0, select_player_image_y_, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
        // ����� main.cpp �е� BeginBatchDraw() ��ʼ���˴��Ļ����ͼ��������ʾ�����ڣ�Ŀ�ģ�������˸
        FlushBatchDraw();
    }

    // ���ư�����Ӧ�ٶȣ�����̫��
    int temp = 0;

    // ��ҿ��Խ��� ���¡��س� ������ѡ����Ϸģʽ
    // ��ҿ�ʼѡ����Ϸ����
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

    // ��Ӧ��ҵ�ѡ��
    switch (select_index_)
    {
    case 0:
        // �������ݣ��´β�������
        Init();
        return SelectResult::kOnePlayer;
    case 1:
        // �������ݣ��´β�������
        Init();
        return SelectResult::kTwoPlayer;
    case 2:
        // �������ݣ��´β�������
        Init();
        return SelectResult::kCustom;
    default:
        throw _T("void SelectPanel::ShowSelectPanel() �쳣");
    }
    return SelectResult::kError;
}
} // namespace battlecity