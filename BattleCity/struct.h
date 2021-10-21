#ifndef BATTLECITY_STRUCT_H_
#define BATTLECITY_STRUCT_H_

#include "stdafx.h"
#include "time_clock.h"

namespace battlecity
{
//typedef unsigned char Byte;

const int kPlayerNumOne = 1;             // ��Ҹ���Ϊ1�������޸ĸ�ֵ!! �Ѿ����������±�!!
const int kPlayerNumTwo = 2;             // ��Ҹ���Ϊ2

const int kWindowWidth = 512;           // ���ڴ�С����ҿ��޸�
const int kWindowHeight = 448;
const int kCanvasWidth = 256;           // ���� image  ��С������ı䣬���һ�ͼ���� image �ϲ�����Ȼ��һ���Ի��Ƶ�������
const int kCanvasHeight = 224;
const int kCenterWidth = 208;           // �м��ɫ��Ϸ����
const int kCenterHeight = 208;
const int kCenterX = 16;                // ��ɫ��Ϸ����������Ͻ�����
const int kCenterY = 9;

const int kGameoverWidth = 31;          // GameOver ������С
const int kGameoverHeight = 15;

// <<box_8>>, <<box_4>> ͬʱ���
// ��ͼ�еĳ���Ԫ��
const int kElementClear = -1;           // ����ϰ��ﱻ�����ĸ���; ����ɨ��ʱ�� 0 ���ֿ���Ȼ����ƺ�ɫͼƬ������� 0 Ҳ���ж������
const int kElementEmpty = 0;            // �յ�
const int kElementForest = 1;           // ɭ��
const int kElementIce = 2;              // ��
// ����ֵС��3 ̹�˿��Դ���
const int kElementWall = 3;             // ǽ
const int kElementRiver = 4;            // ����
const int kElementStone = 5;            // ʯͷ

const int kSignCamp = 200;              // ��Ӫ���
const int kSignStar = 2000;             // �л������Ľ��Ǳ�ǣ���ʱ�л����ܽ���

const int kSignPlayer = 100;            // 

// ���¶�����򣺹涨����� 10000 + 1000 *  mEnemyTankLevel + 100 * mEnemyKind + enemy_id
const int kSignEnemy = 10000;           // 4 * 4 �� 8 * 8 ���ӱ�ǣ����ϵл����� id �ٱ��

// �л��ӵ����
// box_4[i][j] = kSignEnemyBullet + enemy_id
// ֻ��ǵ�ͷ�������ڵ���һ�� 4*4 ����
const int kSignEnemyBullet = 300;

// ����ӵ����
// bullet_4[i][j] = kSignPlayerBullet + player_id * 10 + bullet_id
// ֻ��ǵ�ͷ�������ڵ���һ�� 4*4 ����
const int kSignPlayerBullet = 400;

// �ӵ������ӵ�������������box_4 ���Ϊ wait_unsign ,�ȴ��Է�����
const int kSignWaitUnsign = 444;

const int kSignProp = 3000;             // ������ prop_8 �ı��

const int kBoxSize = 8;                 // 26*26 �ĸ���
const int kSmallBoxSize = 4;            // 52*52 �ĸ��Ӵ�С
const int kBlackNumberSize = 7;         // ��ɫ���ִ�С

const int kEnemyTankIcoSize = 7;        // �л�ͼ���С
const int kPlayerTankIcoSizeX = 7;      // ���ͼ���С
const int kPlayerTankIcoSizeY = 8;
const int kPlayer2IcoSizeX = 14;        // 1P/2P ͼ���С
const int kPlayer2IcoSizeY = 7;
const int kFlagIcoSizeX = 16;;          // ���Ӵ�С
const int kFlagIcoSizeY = 15;
const int kPlayerLifeNumberSize = 7;    // ����������ִ�С

const int kDirLeft = 0;
const int kDirUp = 1;
const int kDirRight = 2;
const int kDirDown = 3;

const int kGrayTank = 0;                // ��ɫ̹��
const int kRedTank = 1;                 // ��ɫ̹��
const int kYellowTank = 2;              // ��ɫ̹��
const int kGreenTank = 3;               // ��ɫ̹��

const int kShootTableX = -100;          // �涨�ӵ����� x = -100, �ӵ����Է���
 
// �ӵ����е��ϰ�������ͣ����� EnemyBase::CheckBomb ����ֵ
enum BulletShootKind
{
    kNone,
    kPlayer1 = kSignPlayer,
    kPlayer2 = kSignPlayer + 1,
    kCamp,
    kOther
};

// ��Ӧ�����ϵ
// box[0][0-25] : ��һ�У�y=0; x=[0-25]��
// box[1][0-25] : �ڶ��У�y=1; x=[0-25]��
// ... �Դ�����
// box[y/kBoxSize][x/kBoxSize]

struct BoxMark
{
    int box_8[26][26];                  // 8*8 ���ӵı�ǣ�̹���ƶ��������øø��Ӽ��
    int box_4[52][52];                  // 484 ���ӵı�ǣ�ǽ�����øñ�Ǽ��
    int prop_8[26][26];                 // ���ߵĸ��ӱ��
    int bullet_4[52][52];               // �ӵ�����
};

const int kTotalEnemyNumber = 20;       // �л�����
const bool kShowingStar = true;         // ������ʾ�Ľ���
const bool kStopShowStar = false;       // �Ľ�����ʾ����

// ָʾ�л����ͣ�����̹�˺���̹ͨ��
enum TankKind
{
    kProp,
    kCommon
};

// �ӵ�
struct Bullet
{
    int x;                              // �ӵ�ͼƬ���Ͻ�����;���ݲ�ͬ����ָ����ͬ��λ�ô���x,y;ָ��x=kShootTableXʱ���Է����ӵ�
    int y;
    int direction;                      // �ӵ�����
    int speed[4];                       // �ӵ��ٶ�;����̹�˼�����費ͬ�ٶ�ϵ����ÿ���ƶ����ܳ���4�����ص㣻��Ȼ���Խ 4*4 �ĸ��ӣ����bug

    // �����һ��ел������ø�ֵΪ�л���id�� GameControl ѭ���ڼ���ֵ��Ȼ��ɾ���õл���
    // ����ǵл�������ң����ø�ֵΪ���id����Ϸѭ�����ټ���ֵ����������ݣ�
    int killed_id;                      // ��¼�ӵ����еĵл�/��� id

    static IMAGE bullet_image[4];       // ͼƬ
    static int bullet_size[4][2];       // {{4,3},{3,4},{4,3},{3,4}} �ߴ磺����-3*4 / ����-4*3
    static int devto_tank[4][2];        // �涨�ӵ������������tank���ĵ��ƫ����(deviation)
    static int devto_head[4][2];        // �涨�ӵ�ͼƬ���Ͻ�������ӵ�ͷ�����ƫ����;���·���ͷ�����ڵ�ͷ���ұߣ��������ڵ�ͷ�������Ǹ���
    static int bomb_center_dev[4][2];   // ��ը����������ӵ�ͷ��ƫ����
};

struct Bomb
{
    static IMAGE bomb_image[3];         // �ӵ���ըͼ
    int bomb_x;                         // ��ը����������
    int bomb_y;
    bool is_show_bomb;                  // �Ƿ�ʼ��ʾ��ըͼƬ flag
    int counter;                        // ȡģ�����������ٴ�ѭ������һ��ͼƬ
};

// ̹�˱�ը״̬
enum class BlastState
{
    kBlasting,
    kBlastEnd,
    kNotBlast
};

// ��ը��ص���
class Blast
{
public:
    Blast();
    ~Blast();

    void Init();

    // ��Ӫ��ը
    BlastState CampBlasting(const HDC &center_hdc);

    // ���̹�˱�ը; PlayerBase �ڵ��ã�ѭ�����̹�˱�ը����ը��Ϸ��� kBlastEnd; ����������践��
    BlastState Blasting(const HDC &center_hdc);

    // �л�̹�˱�ը; �����̹�˱�ը��һ������Ҫ��ʾ����
    BlastState EnemyBlasting(const HDC &center_hdc, IMAGE *score);

    // ���ÿ�ʼ��ը��־ �� ��������
    void SetBlasting(int tank_x, int tank_y);

    // ���ص�ǰ�Ƿ��ڱ�ը
    bool IsBlasting(); 

public:
    static IMAGE image_[5];
    int blast_x_;                       // �������꣬ 32*32
    int blast_y_;
    bool can_blast_;
    int counter_;                       // �������ٴλ�һ��ͼƬ
    TimeClock timer_;
};

// �л�����ҵ��Ľ�����˸��������
enum StarState
{
    kStarTiming,                        // ֻ���ڵл�����ʾ��ǰ��û�г����Ľ��ǣ����ڼ�ʱ֮��
    kStarFailed,                        // ֻ���ڵл�����ʾ��ǰλ�õл�����ʧ��
    kStarOut,                           // �Ľ��Ǹոճ���
    kStarShowing,                       // ������˸�Ľ���
    kStarStop,                          // �ո�ֹͣ��˸
    kTankOut                            // ̹���Ѿ�����
};

// �Ľ�����ص���
class Star
{
public:
    Star();
    void Init();

    // ���̹����ʾ�Ľ���; PlayerBase �ڵ��ã�������̹������
    StarState ShowStar(const HDC &center_hdc, int tank_x, int tank_y);

    bool IsStop();                      // �Ľ����Ƿ�ֹͣ��˸

    // �л�̹����ʾ�Ľ���; EnemyBase �ڵ���
    StarState EnemyShowStar(const HDC &center_hdc, int tank_x, int tank_y, const BoxMark *box_mark);

public:
    static IMAGE star_image_[4];        // �Ľ���ͼƬ������Ҹ�ֵ
    int star_index_dev_;                // �����ı仯��: -1,1; -1��ʾstar��С�仯; 1��ʾstar�ɴ��С
    byte star_index_ : 2;               // �Ľ����±������仯���� 0-1-2-3-2-1-0-1-2-3-...
    int star_counter_;                  // ���������ٴα��һ��ͼ��
    bool is_outed_;                     // ̹���Ƿ��Ѿ����֣��Ľ�����ʧ��ʱ��̹�˳��֣�ֹͣ�����Ľ�����˸ͼ

    // Enemy ר�ã�Player�ò���
    int tank_out_after_counter_;        // һ���������֮���Ľ��ǿ�ʼ��˸��̹�˳���
    bool star_outed_;                   // �Ľ��Ǹճ��֣�ֻ���ڵл�
};

// ���̹�˳��ֵ�ʱ����ʾ����˸��״����Ȧ
class Ring
{
public:
    Ring();

    void Init();
    bool ShowRing(const HDC &canvas_hdc, int tank_x, int tank_y);

    // ���ÿ�����ʾ��״;playerBase �ڻ�ȡ���ߺ����;����������ʾ���,������ʾ�϶�,��õ��ߺ���ʾ�ϳ�
    void SetShowable(long drt_time);

public:
    static IMAGE image_[2];
    bool can_show_;                     // �Ƿ������ʾ��״
    int index_counter_;                 // �仯�±�����
    TimeClock timer_;                   // ��ʱ��
};

/// ��������
const int kPropAdd = 0;                 // �ӻ�
const int kPropStar = 1;                // �����
const int kPropTime = 2;                // ʱ��
const int kPropBomb = 3;                // ����
const int kPropShovel = 4;              // ����
const int kPropCap = 5;                 // ñ��

// ������
// �� PlayerBase ʵ������Աָ��; PlayerBase ���캯���ڵ��ø���һ���������� BoxMark* ����
// �������к������� PlayerBase �ڵ��� 
class Prop
{
public:
    Prop();

    void Init(BoxMark *box_mark);
    void ShowProp(const HDC &canvas_hdc);   // GameControl ��ѭ�����ú���

    // ���������Ͻǵ�����
    void StartShowProp(int x, int y);       // ����̹�˱����к���øú���

    void StopShowProp(bool is_got);         // ֹͣ��ʾ����;��ʱ���߱���һ��,����ָʾ����һ�û��ǳ�ʱ

private:
    void SignPropBox(int val);              // ��� prop_8 ����

private:
    int score_counter_;                     // ������ʾ���
    bool show_score_;
    IMAGE score_;                           // 500 ����

    BoxMark *box_mark_;
    int left_x_;                            // �������ĵ�����
    int top_y_;
    int index_counter_;                     // �±�任����
    IMAGE image_[6];
    int prop_kind_;                         // ��������
    bool can_show_;                         // �Ƿ������ʾ����
};

// ÿһ��ʤ��/ʧ�ܺ���ʾ�ķ������
// PlayerBase ��ʵ������һ�����һ������
class ScorePanel
{
public:
    ScorePanel(int player_id);
    ~ScorePanel();

    bool Show(const HDC &image_hdc);

    // ����ɱ���������������PlayerBase �ڵ��ã� PlayerBase ���� GameControl �ڵ���
    void ResetData(const int *numbers, const int &players, const int &stage);       // ÿ����ʾǰ��Ҫ����
    
public:
    static IMAGE background_;

private:
    static IMAGE yellow_number_;
    static IMAGE white_number_;
    IMAGE player_;
    IMAGE points_;
    static IMAGE bunds_;                 // bunds 1000pts ����

    // ����ĸ���ҷ����࣬����ʾ���ı�
    static int who_bunds_[2];           // ��ʾ�����1�������2��
    static int player_number_;          // ���������ʼ��һ���������ʱҪ+1
    static bool line_done_flag_[2];     // ÿһ������������Ҷ���ʾ�����

    byte player_id_;
    int player_x_;
    int player_y_;
    int points_x_;
    int points_y_;
    int kill_num_[4];                   // 4�ֵл�ɱ������kill_num_ = -1 �����Ȳ���ʾ
    int kill_num2_[4];

    // x_[..][0] �Ƿ����� x_[..][1] ��ɱ����
    int x_[4][2];                       // ���1�ķ�����ɱ������ һ���Ƿ�����һ����ɱ����;�������� ɱ����*100,200,400...
    int y_[4][2];

    static int cur_line_;               // ��ǰ���ڸ�����ʾ������������

    int total_kill_num_;                // ��ɱ����
    int total_kill_x_;                  // ����
    int total_kill_y_;

    static int end_counter_;            // ���������ʾ��ȫ������ת

    int total_score_x_;
    int total_score_y_;
    int total_score_;                   //  �ܷ�
    int stage_;
};

// ĳ����ұ����𵥶���ʾ�� GameOver ����
class PlayerGameover
{
public:
    PlayerGameover();
    ~PlayerGameover();

    void Init(int player_id);
    void SetShow();
    void Show(const HDC &center_hdc);

    TimeClock gameover_timer_;
    int gameover_dev_;                  // X ���ƶ������1���������������2�Ǹ�������
    int gameover_x_;
    int gameover_y_;
    int gameover_end_x_;                // ͼƬֹͣ�� x ����
    static IMAGE gameover_image_;       // �������ֵ���꣬��ʾһ��ˮƽ�ƶ��� GameOver ����
    int gameover_counter_;              // ͼƬֹͣ������ʧ
    bool show_gameover_;
};

} // namespace battlecity

#endif 