#ifndef BATTLECITY_STRUCT_H_
#define BATTLECITY_STRUCT_H_

#include "stdafx.h"
#include "time_clock.h"

namespace battlecity
{
//typedef unsigned char Byte;

const int kPlayerNumOne = 1;             // 玩家个数为1；不能修改该值!! 已经用于数组下标!!
const int kPlayerNumTwo = 2;             // 玩家个数为2

const int kWindowWidth = 512;           // 窗口大小，玩家可修改
const int kWindowHeight = 448;
const int kCanvasWidth = 256;           // 画布 image  大小，不会改变，左右绘图都在 image 上操作，然后一次性绘制到主窗口
const int kCanvasHeight = 224;
const int kCenterWidth = 208;           // 中间黑色游戏区域
const int kCenterHeight = 208;
const int kCenterX = 16;                // 黑色游戏区域相对左上角坐标
const int kCenterY = 9;

const int kGameoverWidth = 31;          // GameOver 字样大小
const int kGameoverHeight = 15;

// <<box_8>>, <<box_4>> 同时标记
// 地图中的场景元素
const int kElementClear = -1;           // 标记障碍物被消除的格子; 用于扫描时与 0 区分开，然后绘制黑色图片，避免对 0 也进行多余操作
const int kElementEmpty = 0;            // 空地
const int kElementForest = 1;           // 森林
const int kElementIce = 2;              // 冰
// 上述值小于3 坦克可以穿行
const int kElementWall = 3;             // 墙
const int kElementRiver = 4;            // 河流
const int kElementStone = 5;            // 石头

const int kSignCamp = 200;              // 大本营标记
const int kSignStar = 2000;             // 敌机出现四角星标记，此时敌机不能进来

const int kSignPlayer = 100;            // 

// 重新定义规则：规定标记是 10000 + 1000 *  mEnemyTankLevel + 100 * mEnemyKind + enemy_id
const int kSignEnemy = 10000;           // 4 * 4 或 8 * 8 格子标记，加上敌机自身 id 再标记

// 敌机子弹标记
// box_4[i][j] = kSignEnemyBullet + enemy_id
// 只标记弹头坐标所在的那一个 4*4 格子
const int kSignEnemyBullet = 300;

// 玩家子弹标记
// bullet_4[i][j] = kSignPlayerBullet + player_id * 10 + bullet_id
// 只标记弹头坐标所在的那一个 4*4 格子
const int kSignPlayerBullet = 400;

// 子弹击中子弹，将自身所在box_4 标记为 wait_unsign ,等待对方擦除
const int kSignWaitUnsign = 444;

const int kSignProp = 3000;             // 道具在 prop_8 的标记

const int kBoxSize = 8;                 // 26*26 的格子
const int kSmallBoxSize = 4;            // 52*52 的格子大小
const int kBlackNumberSize = 7;         // 黑色数字大小

const int kEnemyTankIcoSize = 7;        // 敌机图标大小
const int kPlayerTankIcoSizeX = 7;      // 玩家图标大小
const int kPlayerTankIcoSizeY = 8;
const int kPlayer2IcoSizeX = 14;        // 1P/2P 图标大小
const int kPlayer2IcoSizeY = 7;
const int kFlagIcoSizeX = 16;;          // 旗子大小
const int kFlagIcoSizeY = 15;
const int kPlayerLifeNumberSize = 7;    // 玩家生命数字大小

const int kDirLeft = 0;
const int kDirUp = 1;
const int kDirRight = 2;
const int kDirDown = 3;

const int kGrayTank = 0;                // 灰色坦克
const int kRedTank = 1;                 // 红色坦克
const int kYellowTank = 2;              // 黄色坦克
const int kGreenTank = 3;               // 绿色坦克

const int kShootTableX = -100;          // 规定子弹坐标 x = -100, 子弹可以发射
 
// 子弹击中的障碍物的类型，用在 EnemyBase::CheckBomb 返回值
enum BulletShootKind
{
    kNone,
    kPlayer1 = kSignPlayer,
    kPlayer2 = kSignPlayer + 1,
    kCamp,
    kOther
};

// 对应坐标关系
// box[0][0-25] : 第一行（y=0; x=[0-25]）
// box[1][0-25] : 第二行（y=1; x=[0-25]）
// ... 以此类推
// box[y/kBoxSize][x/kBoxSize]

struct BoxMark
{
    int box_8[26][26];                  // 8*8 格子的标记，坦克移动、击中用该格子检测
    int box_4[52][52];                  // 484 格子的标记，墙击中用该标记检测
    int prop_8[26][26];                 // 道具的格子标记
    int bullet_4[52][52];               // 子弹层标记
};

const int kTotalEnemyNumber = 20;       // 敌机总数
const bool kShowingStar = true;         // 正在显示四角星
const bool kStopShowStar = false;       // 四角星显示结束

// 指示敌机类型：道具坦克和普通坦克
enum TankKind
{
    kProp,
    kCommon
};

// 子弹
struct Bullet
{
    int x;                              // 子弹图片左上角坐标;根据不同方向指定不同的位置代表x,y;指定x=kShootTableX时可以发射子弹
    int y;
    int direction;                      // 子弹方向
    int speed[4];                       // 子弹速度;根据坦克级别给予不同速度系数，每次移动不能超过4个像素点；不然会跨越 4*4 的格子！检测bug

    // 如果玩家击中敌机，设置该值为敌机的id， GameControl 循环内检测该值，然后删除该敌机；
    // 如果是敌机击中玩家，设置该值为玩家id，游戏循环内再检测该值处理玩家数据；
    int killed_id;                      // 记录子弹击中的敌机/玩家 id

    static IMAGE bullet_image[4];       // 图片
    static int bullet_size[4][2];       // {{4,3},{3,4},{4,3},{3,4}} 尺寸：上下-3*4 / 左右-4*3
    static int devto_tank[4][2];        // 规定子弹的坐标相对于tank中心点的偏移量(deviation)
    static int devto_head[4][2];        // 规定子弹图片左上角相对于子弹头坐标的偏移量;上下方向弹头坐标在弹头的右边；左右则在弹头的上面那个点
    static int bomb_center_dev[4][2];   // 爆炸中心相对于子弹头的偏移量
};

struct Bomb
{
    static IMAGE bomb_image[3];         // 子弹爆炸图
    int bomb_x;                         // 爆炸点中心坐标
    int bomb_y;
    bool is_show_bomb;                  // 是否开始显示爆炸图片 flag
    int counter;                        // 取模计数器，多少次循环更换一张图片
};

// 坦克爆炸状态
enum class BlastState
{
    kBlasting,
    kBlastEnd,
    kNotBlast
};

// 爆炸相关的类
class Blast
{
public:
    Blast();
    ~Blast();

    void Init();

    // 大本营爆炸
    BlastState CampBlasting(const HDC &center_hdc);

    // 玩家坦克爆炸; PlayerBase 内调用，循环检测坦克爆炸；爆炸完毕返回 kBlastEnd; 其余情况按需返回
    BlastState Blasting(const HDC &center_hdc);

    // 敌机坦克爆炸; 跟玩家坦克爆炸不一样，还要显示分数
    BlastState EnemyBlasting(const HDC &center_hdc, IMAGE *score);

    // 设置开始爆炸标志 和 传递坐标
    void SetBlasting(int tank_x, int tank_y);

    // 返回当前是否在爆炸
    bool IsBlasting(); 

public:
    static IMAGE image_[5];
    int blast_x_;                       // 中心坐标， 32*32
    int blast_y_;
    bool can_blast_;
    int counter_;                       // 计数多少次换一张图片
    TimeClock timer_;
};

// 敌机或玩家的四角星闪烁返回类型
enum StarState
{
    kStarTiming,                        // 只用于敌机，表示当前还没有出现四角星，还在计时之中
    kStarFailed,                        // 只用于敌机，表示当前位置敌机出现失败
    kStarOut,                           // 四角星刚刚出现
    kStarShowing,                       // 正在闪烁四角星
    kStarStop,                          // 刚刚停止闪烁
    kTankOut                            // 坦克已经出现
};

// 四角星相关的类
class Star
{
public:
    Star();
    void Init();

    // 玩家坦克显示四角星; PlayerBase 内调用，参数是坦克坐标
    StarState ShowStar(const HDC &center_hdc, int tank_x, int tank_y);

    bool IsStop();                      // 四角星是否停止闪烁

    // 敌机坦克显示四角星; EnemyBase 内调用
    StarState EnemyShowStar(const HDC &center_hdc, int tank_x, int tank_y, const BoxMark *box_mark);

public:
    static IMAGE star_image_[4];        // 四角星图片，由玩家赋值
    int star_index_dev_;                // 索引的变化量: -1,1; -1表示star由小变化; 1表示star由大变小
    byte star_index_ : 2;               // 四角星下标索引变化规律 0-1-2-3-2-1-0-1-2-3-...
    int star_counter_;                  // 计数，多少次变更一次图像
    bool is_outed_;                     // 坦克是否已经出现；四角星消失的时候坦克出现，停止播放四角星闪烁图

    // Enemy 专用；Player用不到
    int tank_out_after_counter_;        // 一个随机计数之后，四角星开始闪烁，坦克出现
    bool star_outed_;                   // 四角星刚出现，只用于敌机
};

// 玩家坦克出现的时候显示的闪烁环状保护圈
class Ring
{
public:
    Ring();

    void Init();
    bool ShowRing(const HDC &canvas_hdc, int tank_x, int tank_y);

    // 设置可以显示环状;playerBase 内获取道具后调用;参数设置显示多久,出生显示较短,获得道具后显示较长
    void SetShowable(long drt_time);

public:
    static IMAGE image_[2];
    bool can_show_;                     // 是否可以显示环状
    int index_counter_;                 // 变化下标索引
    TimeClock timer_;                   // 定时器
};

/// 道具类型
const int kPropAdd = 0;                 // 加机
const int kPropStar = 1;                // 五角星
const int kPropTime = 2;                // 时钟
const int kPropBomb = 3;                // 地雷
const int kPropShovel = 4;              // 铲子
const int kPropCap = 5;                 // 帽子

// 道具类
// 在 PlayerBase 实例化成员指针; PlayerBase 构造函数内调用该类一个函数传递 BoxMark* 过来
// 该类所有函数都在 PlayerBase 内调用 
class Prop
{
public:
    Prop();

    void Init(BoxMark *box_mark);
    void ShowProp(const HDC &canvas_hdc);   // GameControl 内循环检测该函数

    // 参数是左上角的坐标
    void StartShowProp(int x, int y);       // 道具坦克被击中后调用该函数

    void StopShowProp(bool is_got);         // 停止显示道具;超时或者被玩家获得,参数指示是玩家获得还是超时

private:
    void SignPropBox(int val);              // 标记 prop_8 格子

private:
    int score_counter_;                     // 计数显示多久
    bool show_score_;
    IMAGE score_;                           // 500 分数

    BoxMark *box_mark_;
    int left_x_;                            // 道具中心点坐标
    int top_y_;
    int index_counter_;                     // 下标变换索引
    IMAGE image_[6];
    int prop_kind_;                         // 道具类型
    bool can_show_;                         // 是否可以显示道具
};

// 每一关胜利/失败后显示的分数面板
// PlayerBase 内实例化；一个玩家一个对象
class ScorePanel
{
public:
    ScorePanel(int player_id);
    ~ScorePanel();

    bool Show(const HDC &image_hdc);

    // 传递杀敌数、玩家数；在PlayerBase 内调用， PlayerBase 又在 GameControl 内调用
    void ResetData(const int *numbers, const int &players, const int &stage);       // 每次显示前需要重置
    
public:
    static IMAGE background_;

private:
    static IMAGE yellow_number_;
    static IMAGE white_number_;
    IMAGE player_;
    IMAGE points_;
    static IMAGE bunds_;                 // bunds 1000pts 字样

    // 检测哪个玩家分数多，就显示在哪边
    static int who_bunds_[2];           // 显示在玩家1还是玩家2处
    static int player_number_;          // 玩家数；初始化一个本类对象时要+1
    static bool line_done_flag_[2];     // 每一行数据两个玩家都显示完与否

    byte player_id_;
    int player_x_;
    int player_y_;
    int points_x_;
    int points_y_;
    int kill_num_[4];                   // 4种敌机杀敌数，kill_num_ = -1 的项先不显示
    int kill_num2_[4];

    // x_[..][0] 是分数， x_[..][1] 是杀敌数
    int x_[4][2];                       // 玩家1的分数和杀敌数？ 一个是分数，一个是杀敌数;分数等于 杀敌数*100,200,400...
    int y_[4][2];

    static int cur_line_;               // 当前正在更新显示的数据行索引

    int total_kill_num_;                // 总杀敌数
    int total_kill_x_;                  // 坐标
    int total_kill_y_;

    static int end_counter_;            // 分数面板显示完全后多久跳转

    int total_score_x_;
    int total_score_y_;
    int total_score_;                   //  总分
    int stage_;
};

// 某个玩家被消灭单独显示的 GameOver 字样
class PlayerGameover
{
public:
    PlayerGameover();
    ~PlayerGameover();

    void Init(int player_id);
    void SetShow();
    void Show(const HDC &center_hdc);

    TimeClock gameover_timer_;
    int gameover_dev_;                  // X 轴移动；玩家1是正数分量，玩家2是负数分量
    int gameover_x_;
    int gameover_y_;
    int gameover_end_x_;                // 图片停止的 x 坐标
    static IMAGE gameover_image_;       // 玩家生命值用完，显示一个水平移动的 GameOver 字样
    int gameover_counter_;              // 图片停止后多久消失
    bool show_gameover_;
};

} // namespace battlecity

#endif 