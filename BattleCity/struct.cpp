#include "stdafx.h"
#include "struct.h"
#include "mci_sound.h"

namespace battlecity
{
// �ӵ��ṹ��̬����
IMAGE Bullet::bullet_image[4];
int Bullet::bullet_size[4][2] = { {4,3},{3,4},{4,3},{3,4} };    // ��ͬ�����ӵ��ߴ�(width, height)

// �ӵ����̹�����ĵ�ƫ����������ƫ��
int Bullet::devto_tank[4][2] = { {-kBoxSize,-1},{-2,-kBoxSize},{kBoxSize - 4,-1},{-2,kBoxSize - 4} };

// �ӵ�ͼƬ���Ͻ�����ת������ͷ��ƫ������������ƫ�Ʋ����±꣩
// ���ҷ�����͹���������ǵ㣻���·�������͹�����ұ��ǵ�
int Bullet::devto_head[4][2] = { {0,1},{2,0},{4,1}, {2,4} };

// ��ը����������ӵ�ͷ���±�ƫ����
int Bullet::bomb_center_dev[4][2] = { {1,0},{0,1},{0,0}, {0,0} };

//////////////////////////////////////////////////////////////////////////

IMAGE Bomb::bomb_image[3];

//////////////////////////////////////////////////////////////////////////

IMAGE Blast::image_[5];
Blast::Blast()
{
    Init();
    timer_.SetDrtTime(36);

    // ̹�˱�ըͼƬ�ṹ
    TCHAR buff[100];
    for (INT i = 0; i < 5; ++i)
    {
        _stprintf_s(buff, _T("./res/big/blast/%d.gif"), i);
        loadimage(&Blast::image_[i], buff);
    }
}

Blast::~Blast()
{

}

void Blast::Init()
{
    blast_x_ = -100;
    blast_y_ = -100;
    can_blast_ = false;
    counter_ = 0;
}

BlastState Blast::CampBlasting(const HDC &center_hdc)
{
    if (can_blast_)
    {
        int index[17] = { 0,0,0,1,1,2,2,3,3,4,4,4,4,3,2,1,0 };

        TransparentBlt(center_hdc, blast_x_ * kBoxSize, blast_y_ * kBoxSize, kBoxSize * 4, kBoxSize * 4,
            GetImageHDC(&Blast::image_[index[counter_ % 17]]), 0, 0, kBoxSize * 4, kBoxSize * 4, 0x000000);

        if (timer_.IsTimeOut() && counter_++ == 17)
        {
            can_blast_ = false;
            return BlastState::kBlastEnd;
        }
        return BlastState::kBlasting;
    }
    return BlastState::kNotBlast;
}

BlastState Blast::Blasting(const HDC &center_hdc)
{
    int index[13] = { 0,1,1,2,2,3,3,4,4,3,2,1,0 };
    if (can_blast_)
    {
        TransparentBlt(center_hdc, blast_x_ - kBoxSize * 2, blast_y_ - kBoxSize * 2, kBoxSize * 4, kBoxSize * 4,
            GetImageHDC(&Blast::image_[index[counter_ % 13]]), 0, 0, kBoxSize * 4, kBoxSize * 4, 0x000000);

        if (timer_.IsTimeOut())
        {
            if (counter_++ >= 12)
            {
                Init();
                return BlastState::kBlastEnd;
            }
        }
        return BlastState::kBlasting;
    }
    return BlastState::kNotBlast;
}

// ר���ڵл���ը
BlastState Blast::EnemyBlasting(const HDC &center_hdc, IMAGE *score)
{
    int index[13] = { 0,1,1,2,2,3,3,4,4,3,2,1,0 };
    if (can_blast_)
    {
        if (counter_ < 13)
        {
            TransparentBlt(center_hdc, blast_x_ - kBoxSize * 2, blast_y_ - kBoxSize * 2, kBoxSize * 4, kBoxSize * 4,
                GetImageHDC(&Blast::image_[index[counter_ % 13]]), 0, 0, kBoxSize * 4, kBoxSize * 4, 0x000000);
        }
        else
        {
            TransparentBlt(center_hdc, blast_x_ - 7, blast_y_ - 3, 14, 7, GetImageHDC(score), 0, 0, 14, 7, 0x000000);
        }

        if (timer_.IsTimeOut())
        {
            if (counter_++ >= 18)
            {
                Init();
                return BlastState::kBlastEnd;
            }
        }
        return BlastState::kBlasting;
    }
    return BlastState::kNotBlast;
}

void Blast::SetBlasting(int tank_x, int tank_y)
{
    blast_x_ = tank_x;
    blast_y_ = tank_y;
    can_blast_ = true;
}

bool Blast::IsBlasting()
{
    return can_blast_;
}

//////////////////////////////////////////////////////////////////////////
IMAGE Star::star_image_[4];
Star::Star()
{
    TCHAR buf[100];
    // ����ʱ�Ľ�����˸
    for (int i = 0; i < 4; ++i)
    {
        _stprintf_s(buf, _T("./res/big/star%d.gif"), i);
        loadimage(&Star::star_image_[i], buf);
    }

    Init();
}

void Star::Init()
{
    star_index_dev_ = -1;           // ���������±�����ӻ��Ǽ���
    star_index_ = 3;                 // star ͼƬ�����±�
    star_counter_ = 0;              // ���ٴθ��� star ͼƬ
    tank_out_after_counter_ = rand() % 10 + 10; 
    is_outed_ = false;              // ̹���Ƿ��Ѿ�����
    star_outed_ = false;
}

StarState Star::ShowStar(const HDC &center_hdc, int tank_x, int tank_y)
{
    // ̹���Ѿ����֣�������˸��ֱ�ӷ���
    if (is_outed_)
    {
        return StarState::kTankOut;
    }

    // ��ʼ��˸�Ľ���
    if (star_counter_++ % 2 == 0)
    {
        if (star_index_ + star_index_dev_ < 0)
        {
            star_index_ = 1;
            star_index_dev_ = 1;
        }
        else if (star_index_ + star_index_dev_ > 3)
        {
            star_index_ = 2;
            star_index_dev_ = -1;
        }
        else
        {
            star_index_ += star_index_dev_;
        }

        if (star_counter_ == 25)
        {
            is_outed_ = true;           // ������˸��TankMoving() ������ʼѭ����̹�˿�ʼ�ƶ�
            return StarState::kStarStop;
        }
    }

    TransparentBlt(center_hdc, tank_x - kBoxSize, tank_y - kBoxSize, kBoxSize * 2, kBoxSize * 2,
        GetImageHDC(&star_image_[star_index_]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);

    return StarState::kStarShowing;
}

bool Star::IsStop()
{
    return is_outed_;
}

StarState Star::EnemyShowStar(const HDC &center_hdc, int tank_x, int tank_y, const BoxMark *box_mark)
{
    // ̹���Ѿ����֣�������˸��ֱ�ӷ���
    if (is_outed_)
    {
        return StarState::kTankOut;
    }

    // һ��ʱ������ʾ�Ľ��ǣ�֮ǰ����
    if (tank_out_after_counter_-- > 0)
    {
        return StarState::kStarTiming;
    }

    // �Ľ��ǻ�û���ֲż�� box_4
    if (!star_outed_)
    {
        int i_y = tank_y / kSmallBoxSize - 2;
        int j_x = tank_x / kSmallBoxSize - 2;
        for (int i = i_y; i < i_y + 4; ++i)
        {
            for (int j = j_x; j < j_x + 4; ++j)
            {
                // ����Ľ��ǣ���ң��л�
                if (box_mark->box_4[i][j] != kSignStar && box_mark->box_4[i][j] > kElementForest)
                {
                    tank_out_after_counter_ = rand() % 100 + 10;
                    return StarState::kStarFailed;
                }
            }
        }

        // �Ľ��Ǹճ��֣������ϲ㺯�� ��� box_4
        star_outed_ = true;
        return StarState::kStarOut;
    }

    // ��ʼ��˸�Ľ���
    if (star_counter_++ % 2 == 0)
    {
        if (star_index_ + star_index_dev_ < 0)
        {
            star_index_ = 1;
            star_index_dev_ = 1;
        }
        else if (star_index_ + star_index_dev_ > 3)
        {
            star_index_ = 2;
            star_index_dev_ = -1;
        }
        else
        {
            star_index_ += star_index_dev_;
        }

        if (star_counter_ == 35)
        {
            is_outed_ = true;           // ������˸��TankMoving() ������ʼѭ����̹�˿�ʼ�ƶ�
            return StarState::kStarStop;
        }
    }

    TransparentBlt(center_hdc, tank_x - kBoxSize, tank_y - kBoxSize, kBoxSize * 2, kBoxSize * 2,
        GetImageHDC(&star_image_[star_index_]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);

    return StarState::kStarShowing;
}

//////////////////////////////////////////////////////////////////////////
IMAGE Ring::image_[2];
Ring::Ring()
{
    TCHAR buf[100];
    // ����������
    for (int i = 0; i < 2; ++i)
    {
        _stprintf_s(buf, _T("./res/big/ring%d.gif"), i);
        loadimage(&Ring::image_[i], buf);
    }
    Init();
}

void Ring::Init()
{
    can_show_ = true;
    index_counter_ = 0;
    timer_.SetDrtTime(3222);
}

bool Ring::ShowRing(const HDC &canvas_hdc, int tank_x, int tank_y)
{
    if (timer_.IsTimeOut())
    {
        index_counter_ = 0;
        can_show_ = false;
        return false;
    }

    TransparentBlt(canvas_hdc, (int)(tank_x - kBoxSize), (int)(tank_y - kBoxSize), kBoxSize * 2, kBoxSize * 2,
        GetImageHDC(&image_[index_counter_++ / 4 % 2]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
    return true;
}

void Ring::SetShowable(long drt_time)
{
    can_show_ = true;
    index_counter_ = 0;
    timer_.Init();
    timer_.SetDrtTime(drt_time);
}

//////////////////////////////////////////////////////////////////////////

Prop::Prop()
{

}

void Prop::Init(BoxMark *box_mark)
{
    box_mark_ = box_mark;
    left_x_ = -100;
    top_y_ = -100;
    index_counter_ = 0;
    can_show_ = false;
    show_score_ = false;
    score_counter_ = 0;

    loadimage(&score_, _T("./res/big/500.gif"));

    TCHAR buf[100];
    for (int i = 0; i < 6; ++i)
    {
        _stprintf_s(buf, _T("./res/big/prop/p%d.gif"), i);
        loadimage(&image_[i], buf);
    }
}

// GameControl ��ѭ�����ú���
void Prop::ShowProp(const HDC &canvas_hdc)
{
    if (!can_show_)
    {
        return;
    }

    if (show_score_)
    {
        score_counter_++;
        TransparentBlt(canvas_hdc, left_x_ + 8, top_y_ + 3, 14, 7, GetImageHDC(&score_), 0, 0, 14, 7, 0x000000);
        if (score_counter_ > 37)
        {
            show_score_ = false;
            can_show_ = false;
        }
    }
    else if (can_show_)
    {
        if ((++index_counter_ / 17) % 2 == 0)
        {
            TransparentBlt(canvas_hdc, left_x_, top_y_, kBoxSize * 2, kBoxSize * 2,
                GetImageHDC(&image_[prop_kind_]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
        }
    }

    // ����ʱ�� ��ʧ
    if (index_counter_ > 1300)
    {
        StopShowProp(false);
    }
}

// ���ߵл���������øú���
void Prop::StartShowProp(int x, int y)
{
    // �����һ�εı�ǣ���ֹ��һ�����߻�û��ʧ�����ֵڶ������ߵ�ʱ�򣬲�����һ�����ߵ� SIGN����ǣ�
    SignPropBox(kElementEmpty);

    left_x_ = y * kBoxSize; // (rand() % 25 +  1) * kBoxSize
    top_y_ = x * kBoxSize;  // (rand() % 25 + 1) * kBoxSize
    can_show_ = true;
    prop_kind_ = rand() % 6;    // �������һ������
    index_counter_ = 0;
    SignPropBox(kSignProp + prop_kind_);
}

// is_got == true ��ʾ��һ�õ��ߣ���ʾ������false ��ʱ����ʾ����
void Prop::StopShowProp(bool is_got)
{
    if (is_got)
    {
        show_score_ = true;
        score_counter_ = 0;
    }
    else
    {
        can_show_ = false;
    }
    SignPropBox(kElementEmpty);
}

void Prop::SignPropBox(int val)
{
    int i = top_y_ / kBoxSize;
    int j = left_x_ / kBoxSize;

    for (int ix = i; ix < i + 2; ++ix)
    {
        for (int jy = j; jy < j + 2; ++jy)
        {
            box_mark_->prop_8[ix][jy] = val;
        }
    }
}

//////////////////////////////////////////////////////////////////////////

int ScorePanel::end_counter_ = 0;
int ScorePanel::cur_line_ = 0;
bool ScorePanel::line_done_flag_[2] = { false,false };
int ScorePanel::player_number_ = 0;
int ScorePanel::who_bunds_[2] = { 0,0 };
IMAGE ScorePanel::white_number_;
IMAGE ScorePanel::bunds_;
IMAGE ScorePanel::background_;
IMAGE ScorePanel::yellow_number_;

ScorePanel::ScorePanel(int player_id)
{
    // gameover �ٴν�����Ϸ���ܻ���� 2 player_number_ ++ ?
    player_id_ = player_id;
    switch (player_id_)
    {
    case 0:
        player_x_ = 24;
        player_y_ = 47;
        points_x_ = 25;
        points_y_ = 85;

        loadimage(&player_, _T("./res/big/scorepanel/player-0.gif"));
        loadimage(&points_, _T("./res/big/scorepanel/pts-0.gif"));

        loadimage(&yellow_number_, _T("./res/big/yellow-number.gif"));
        loadimage(&white_number_, _T("./res/big/white-number.gif"));
        loadimage(&background_, _T("./res/big/scorepanel/background.gif"));
        loadimage(&bunds_, _T("./res/big/scorepanel/bunds.gif"));

        for (int i = 0; i < 4; ++i)
        {
            x_[i][0] = 34;  // ����
            x_[i][1] = 103; // ɱ����

            y_[i][0] = 88 + i * 24;
            y_[i][1] = 88 + i * 24;

            total_kill_x_ = 103;
            total_kill_y_ = 178;
        }

        total_score_x_ = 78;
        total_score_y_ = 58;
        break;

    case 1:
        player_x_ = 170;
        player_y_ = 47;
        points_x_ = 137;
        points_y_ = 85;

        loadimage(&player_, _T("./res/big/scorepanel/player-1.gif"));
        loadimage(&points_, _T("./res/big/scorepanel/pts-1.gif"));

        for (int i = 0; i < 4; ++i)
        {
            x_[i][0] = 177;  // ����
            x_[i][1] = 154; // ɱ����

            y_[i][0] = 88 + i * 24;
            y_[i][1] = 88 + i * 24;

            total_kill_x_ = 154;
            total_kill_y_ = 178;
        }

        total_score_x_ = 224;
        total_score_y_ = 58;
        break;

    default:
        break;
    }

    for (int i = 0; i < 4; ++i)
    {
        kill_num_[i] = 0;       // ���� PlayerBase ���ݹ���������
        kill_num2_[i] = -1;     // Ĭ��ɱ���� = -1 flag�� ��ʱ����ʾ
    }

    total_kill_num_ = 0;
    end_counter_ = 0;
    total_score_ = 0;
    stage_ = 1;

    who_bunds_[0] = 0;
    who_bunds_[1] = 0;
}

ScorePanel::~ScorePanel()
{

}

bool ScorePanel::Show(const HDC &image_hdc)
{
    BitBlt(image_hdc, player_x_, player_y_, player_.getwidth(), player_.getheight(), GetImageHDC(&player_), 0, 0, SRCCOPY);
    BitBlt(image_hdc, points_x_, points_y_, points_.getwidth(), points_.getheight(), GetImageHDC(&points_), 0, 0, SRCCOPY);

    // ����ÿ��ɱ�����Լ���ʾ ++ ��ʾ��һ�в���ʾ��һ��
    for (int i = 0; i < 4; ++i)
    {
        // ��ǰ��ʾ�ŵ���
        if (cur_line_ == i)
        {
            int temp = kill_num2_[i] + 1;
            if (temp <= kill_num_[i])
            {
                kill_num2_[i]++;
                MciSound::PlaySounds(kSoundScorePanelDi);
                break;
            }
            else // ���������Ҹ��з�����ʾ����û
            {
                line_done_flag_[player_id_] = true;
                bool flag = true;
                for (int j = 0; j < player_number_; ++j)
                {
                    if (!line_done_flag_[j])
                    {
                        flag = false;
                        break;
                    }
                }
                if (flag)
                {
                    cur_line_++;
                    for (int k = 0; k < player_number_; ++k)
                    {
                        line_done_flag_[k] = false;
                    }
                }
            }
        }
    }

    Sleep(100);

    // Stage
    TransparentBlt(image_hdc, 154, 32, kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_), 
        kBlackNumberSize * (stage_ % 10), 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
    if (stage_ > 10)
    {
        TransparentBlt(image_hdc, 154 - 8, 32, kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_), 
            kBlackNumberSize * (stage_ / 10), 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
    }

    // 0��
    TransparentBlt(image_hdc, total_score_x_, total_score_y_, kBlackNumberSize, kBlackNumberSize, GetImageHDC(&yellow_number_),
        0, 0, kBlackNumberSize, kBlackNumberSize, 0x000000);

    // ��λ��
    if (total_score_ > 90)
    {
        TransparentBlt(image_hdc, total_score_x_ - 8, total_score_y_, kBlackNumberSize, kBlackNumberSize, GetImageHDC(&yellow_number_),
            0, 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
        TransparentBlt(image_hdc, total_score_x_ - 16, total_score_y_, kBlackNumberSize, kBlackNumberSize, GetImageHDC(&yellow_number_),
            kBlackNumberSize * (total_score_ % 1000 / 100), 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
    }

    // ��λ��
    if (total_score_ > 900)
    {
        TransparentBlt(image_hdc, total_score_x_ - 24, total_score_y_, kBlackNumberSize, kBlackNumberSize, GetImageHDC(&yellow_number_),
            kBlackNumberSize * (total_score_ / 1000), 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
    }

    for (int i = 0; i < 4; ++i)
    {
        if (kill_num2_[i] == -1)
        {
            continue;
        }

        if (kill_num2_[i] < 10)
        {
            TransparentBlt(image_hdc, x_[i][1], y_[i][1], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                kBlackNumberSize * kill_num2_[i], 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
        }
        else
        {
            TransparentBlt(image_hdc, x_[i][1] - 8, y_[i][1], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                kBlackNumberSize * (kill_num2_[i] / 10), 0, kBlackNumberSize, kBlackNumberSize, 0x000000);

            TransparentBlt(image_hdc, x_[i][1], y_[i][1], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                kBlackNumberSize * (kill_num2_[i] % 10), 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
        }

        int score = (i + 1) * 100 * kill_num2_[i];      // һ��̹��һ��100�֣���������

        // ��������Ƕ���λ��
        int temp = score;
        int score_bit = 1;
        while (temp / 10 != 0)
        {
            score_bit++;
            temp /= 10;
        }

        // ������� 4λ��
        switch (score_bit)
        {
        case 1:
            TransparentBlt(image_hdc, x_[i][0] + 16, y_[i][0], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                0, 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
            break;
        case 2:
            break;
        case 3:
            TransparentBlt(image_hdc, x_[i][0], y_[i][0], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                kBlackNumberSize * (score / 100), 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
            TransparentBlt(image_hdc, x_[i][0] + 8, y_[i][0], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                0, 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
            TransparentBlt(image_hdc, x_[i][0] + 16, y_[i][0], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                0, 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
            break;
        case 4:
            TransparentBlt(image_hdc, x_[i][0] - 8, y_[i][0], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                kBlackNumberSize * (score / 1000), 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
            TransparentBlt(image_hdc, x_[i][0], y_[i][0], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                kBlackNumberSize *(score % 1000 / 100) , 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
            TransparentBlt(image_hdc, x_[i][0] + 8, y_[i][0], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                0, 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
            TransparentBlt(image_hdc, x_[i][0] + 16, y_[i][0], kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                0, 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
            break;
        default:
            break;
        }
    }

    if (cur_line_ == 4)
    {
        if (total_kill_num_ < 10)
        {
            TransparentBlt(image_hdc, total_kill_x_, total_kill_y_, kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                kBlackNumberSize * total_kill_num_, 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
        }
        else
        {
            TransparentBlt(image_hdc, total_kill_x_ - 8, total_kill_y_, kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                kBlackNumberSize * (total_kill_num_ / 10), 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
            TransparentBlt(image_hdc, total_kill_x_, total_kill_y_, kBlackNumberSize, kBlackNumberSize, GetImageHDC(&white_number_),
                kBlackNumberSize *(total_kill_num_ % 10), 0, kBlackNumberSize, kBlackNumberSize, 0x000000);
        }

        if (player_number_ == 2 && who_bunds_[0] > who_bunds_[1] && who_bunds_[0] > 1000)
        {
            TransparentBlt(image_hdc, 26, 190, 63, 15, GetImageHDC(&bunds_), 0, 0, 63, 15, 0x000000);
        }
        else if (player_number_ == 2 && who_bunds_[1] > who_bunds_[0] && who_bunds_[1] > 1000)
        {
            TransparentBlt(image_hdc, 170, 190, 63, 15, GetImageHDC(&bunds_), 0, 0, 63, 15, 0x000000);
        }

        // ֻ����һ��
        if (end_counter_ == 0 && player_number_ == 2 && who_bunds_[0] + who_bunds_[1] > 2000)
        {
            MciSound::PlaySounds(kSoundBouns1000);
        }

        if (end_counter_++ > 30)
        {
            return false; // ���ؽ�����־
        }
    }

    return true;
}

// ��Ϸ����ʱ�򣬻�ȡÿ����ҵ�ɱ������ֻ�ܵ���һ��
void ScorePanel::ResetData(const int *numbers, const int &players, const int &stage)
{
    // ����Ҫ���ã������´���ʾ��ʱ����ԭ�ȵ�����
    player_number_ = players;
    cur_line_ = 0;
    end_counter_ = 0;
    line_done_flag_[0] = false;
    line_done_flag_[1] = false;
    total_kill_num_ = 0;
    total_score_ = 0;
    stage_ = stage;
    who_bunds_[player_id_] = 0;     // �������õ���һ����ҵ�����

    for (int i = 0; i < 4; ++i)
    {
        kill_num_[i] = 0;           // ���� PlayerBase ���ݹ���������
        kill_num2_[i] = -1;         // Ĭ��ɱ����Ϊ -1 flag�� ��ʱ����ʾ
    }

    // ��ȡ������
    for (int i = 0; i < 4; ++i)
    {
        kill_num_[i] = numbers[i];
        total_kill_num_ += numbers[i];
    }

    total_score_ = numbers[0] * 100 + numbers[1] * 200 + numbers[2] * 300 + numbers[3] * 400;

    who_bunds_[player_id_] = total_score_;
}

//////////////////////////////////////////////////////////////////////////

IMAGE PlayerGameover::gameover_image_;
PlayerGameover::PlayerGameover()
{
    gameover_timer_.SetDrtTime(20);
    loadimage(&gameover_image_, _T("./res/big/gameover.gif"));
}

PlayerGameover::~PlayerGameover()
{

}

void PlayerGameover::Init(int player_id)
{
    // �����������ʾ���Ƶ� GAMEOVER ����
    switch (player_id)
    {
    case 0:        
        gameover_x_ = 0;
        gameover_dev_ = 3;
        gameover_end_x_ = 53;
        break;

    case 1:
        gameover_x_ = 220;
        gameover_dev_ = -3;
        gameover_end_x_ = 122;
        break;

    default:
        break;
    }

    // ��ұ��������ʾͼƬ GAMEOVER
    gameover_y_ = 191;
    gameover_counter_ = 0;
    show_gameover_ = false;
}

void PlayerGameover::SetShow()
{
    show_gameover_ = true;
}

void PlayerGameover::Show(const HDC &center_hdc)
{
    if (gameover_counter_ > 70)
    {
        show_gameover_ = false;
    }

    if (!show_gameover_)
    {
        return;
    }

    TransparentBlt(center_hdc, gameover_x_, gameover_y_, 31, 15, GetImageHDC(&gameover_image_), 0, 0, 31, 15, 0x000000);

    if (!gameover_timer_.IsTimeOut())
    {
        return;
    }

    if (abs(gameover_x_ - gameover_end_x_) < 5)
    {
        gameover_counter_++;
        gameover_x_ = gameover_end_x_;
    }
    else
    {
        gameover_x_ += gameover_dev_;
    }
}

} // namespace battlecity