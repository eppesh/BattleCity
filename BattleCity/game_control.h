#ifndef BATTLECITY_GAME_CONTROL_H_
#define BATTLECITY_GAME_CONTROL_H_

#include "player_base.h"
#include "enemy_base.h"
#include "time_clock.h"

namespace battlecity
{
struct Map
{
    char buf[26][27];
};

// 类功能:
// 从 main 接收主窗口绘图接口; 定义一个 IMAGE 画布, 绘制地图/敌机列表信息/旗子/关卡;
// 存储格子标记; 存储玩家链表; 进行游戏循环; 

// 成员数据:
// EnemyList: 存储被消灭的 and 正在移动 and 等待出现的敌机


enum GameResult { kVictory, kFail };		        // 每一关胜利\失败

class GameControl
{
public:
    GameControl(HDC des_hdc, HDC image_hdc);
    ~GameControl();
    void Init();
    void AddPlayer(int player_num);					// 加载玩家进该类控制
    void LoadMap();									// 读取 .map 地图文件|解析绘制地图
    bool CreateMap(bool *out);				        // 玩家自定义地图
    void GameLoop();
    GameResult StartGame();							// 更新绘制游戏各种东西, 返回 false 结束游戏

private:
    void CutStage();		                        // 动画
    void ShowStage();				                // 开始每一关之前显示 STAGE 2 之类的字样
    void ClearSignBox();
    void InitSignBox();				                // 初始化格子标记
    void AddEnemy();						        // 游戏开始前 添加20 架敌机
    void SignBox_4(int i, int j, int sign_val);	    // 标记格子, LoadMap() 内调用
    bool RefreshData();
    void RefreshRightPanel();						// 刷新右边信息面板数据, 根据数据变化才调用更新
    void RefreshCenterPanel();						// 刷新中间游戏区域 208 * 208
    void CheckKillEnemy(PlayerBase *player_base);	// 检测玩家是否击中敌机, 击中则删除该敌机
    void IsGameOver();				                // 循环检测是否flag,  gameover
    void IsWinOver();				                // 消灭所有敌机胜利过关

public:
    static int current_stage_;						// [1-35] 当前关卡, SelectPanel 内使用, 本类内使用

private:
    // 窗口绘图接口
    IMAGE center_image_;				            // 用于保持 mCenter_hdc 存在有效性.
    HDC des_hdc_;					                // 主窗口hdc, main 传入
    HDC image_hdc_;					                // 256*224 的 hdc, main 传入
    HDC center_hdc_;				                // 208*208 的 hdc, 本类产生, 是游戏区域,分开,经常更新,坐标也容易设定

    BoxMark *box_mark_;			                    // 存储格子标记
    std::list<PlayerBase *> player_list_;
    std::list<EnemyBase *> enemy_list_;				// 敌机列表

    IMAGE gray_background_image_;		            // 游戏灰色背景图
    IMAGE black_background_image_;	                // 黑色背景图

    Map map_;						                // 存储地图数据结构
    IMAGE stone_image_;				                // 石头
    IMAGE forest_image_;				            // 树林
    IMAGE ice_image_;				                // 冰块
    IMAGE river_image_[2];			                // 河流
    IMAGE wall_image_;				                // 泥墙
    IMAGE camp_image_[2];					        // 大本营

    IMAGE enemy_tank_ico_image_;		            // 敌机坦克图标
    IMAGE flag_image_;				                // 旗子

    int cut_stage_counter_;
    IMAGE current_stage_image_;		                // STAGE 字样
    IMAGE black_number_image_;		                // 0123456789 当前关卡数
    IMAGE gameover_image_;			

    // 初始 mActiveEnemyTankNumber + mRemainEnemyTankNumber = 20
    int remain_enemy_tank_number_;		            // 余下未显示的敌机数量, 初始值=mBlast20, 出现一架敌机该值减一
    int cur_moving_tank_number_;		            // 当前出现的敌机数量
    int killed_enemy_number_;				        // 已经消灭的敌机数

    Blast blast_;				                    // 大本营爆炸
    bool is_camp_die_;					            // 大本营被击中, 不再 Camp 贴图 

    bool is_enemy_pause_;			                // 是否可以移动敌机
    TimeClock enemy_pause_timer_;		            // 敌机暂停多久

    TimeClock main_timer_;			                // 控制绘图频率
    TimeClock camp_timer_;		                    // Camp 爆炸速度

    int gameover_counter_;
    int gameover_x_;
    int gameover_y_;	// 
    bool gameover_flag_;			                // 游戏结束标记, 玩家生命用完, 鸟巢被击中
    TimeClock gameover_timer_;	                    // 上升控制

    IMAGE create_map_tank_image_;		            // 自定义地图中的 坦克游标图片
    int cmt_image_x_; 	                            // 游标中心点坐标
    int cmt_image_y_;

    bool has_custom_map_;			                // 当前是否有制作地图, 用于退出制作再次进入制作, 不会之前制作的数据
    bool show_score_panel_;		                    // 是否显示分数面板

    bool is_win_;			                        // 消灭完敌机,胜利
    int win_counter_;	                            // 消灭完后隔几秒才跳转

    bool show_gameover_after_score_panel_;
    IMAGE msgoas_image_;                            // 大号的Gameover
    int msgoas_y_;		                            // 上升坐标
    int msgoas_counter_;
};
} // namespace battlecity

#endif