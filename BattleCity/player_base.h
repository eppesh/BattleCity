#ifndef BATTLECITY_PLAYER_BASE_H_
#define BATTLECITY_PLAYER_BASE_H_

#include "tank.h"
#include "struct.h"
#include "time_clock.h"

namespace battlecity
{

// 玩家控制
// 一个玩家实例化一个对象；各自管理自己的数据
class PlayerBase
{
public:
	PlayerBase(byte player, BoxMark *box_mark);								// player [0-1]
	~PlayerBase();

	void Init();
	void DrawPlayerTankIco(const HDC &right_panel_hdc);						// GameControl 内循环调用
	bool ShowStar(const HDC &center_hdc);									// 玩家坦克闪烁出现
	void DrawPlayerTank(const HDC &canvas_hdc);								// 纯绘制坦克操作; GameControl 内循环调用
	void DrawBullet(const HDC &center_hdc);									// 绘制子弹,需要与移动分开, 不然主循环内会失帧, 因为不一定每次绘图都会移动
	bool PlayerControl();													// 检测按键; GameControl 内循环调用

	BulletShootKind BulletMoving(const HDC &center_hdc);					// bullet_[i].x = kShootTableX 子弹才运动; GameControl 内循环调用
	void Bombing(const HDC &center_hdc);									// 函数内是否显示爆炸效果由 检测子弹击中 的函数决定; GameControl 内循环调用
	void GetKillEnemy(int &bullet1, int &bullet2);							// GameControl 内调用, 通过参数获取被击中的敌机 id; bullet1 : 1 号子弹击中敌机的 id, bullet2 : 2 号子弹击中敌机的 id; 

	void BeKilled();														// BeKilled() 控制爆炸图显示; GameControl 内循环调用 EnemyBase->GetId() 获取被击中玩家的 id,然后调用对应玩家的 BeKilled 函数; 

	bool Blasting(const HDC &center_hdc);									// GameControl 内循环调用; BeKill 被调用后设置一个 flag = true,该函数检测这个 flag 然后显示爆炸图,返回 true 玩家生命用光

	const int &GetID();
	static bool IsGetTimeProp();											// GameControl 内调用, 检测玩家是否获得时间道具
	static bool IsGetShvelProp();											// 铲子道具循环
	bool IsGetBombProp();

	static void ShowProp(const HDC &center_hdc);							// 在 gameControl 内循环调用 检测并显示闪烁道具
	bool ShowScorePanel(const HDC &image_hdc);								// GameCnotrol 内循环调用, 函数内调用成员ScorePanel 函数显示结果
	void SetPause();														// 设置玩家暂停移动; GameControl 内检测子弹击中结果判断调用
	static void SetShowProp();												// 如果成功消灭道具敌机, 就显示道具; GameControl 内 CheckKillEnemy() 内调用
	void AddKillEnemyNum(byte enemy_level);									// 记录玩家消灭不同级别类型的敌机的数量; GameControl 内CheckKillEnemy() 内调用
	void ResetScorePanelData(const int &player_num, const int &stage);		// GameControl 内每一关结束显示分数面板的时候调用
	bool IsLifeEnd();														// 胜利进入下一关时检测, 死亡的玩家不重新初始化, 下一关不会出现
	void CheckShowGameOver(const HDC &center_hdc);							// GameCOntrol 内循环调用, 判断玩家生命值, 显示GAMEOVER 字样
	void SetShowGameover();													// GameControl 内玩家 blasting 结束后调用

private:
	void SignBullet(int left_x, int top_y, byte dir, int val);				// 标记子弹头所在的一个 4*4 格子, 参数是子弹图片左上角坐标	
	void Reborn();															// 如果有生命重新出生
	void DispatchProp(int prop_kind);										// 获取到道具, 坦克移动内检测调用	
	void Move(int new_dir);													// 判断并更改方向或者移动,同时调整坐标到格子整数处; PlayerControl() 内调用
	bool CheckMoveable();													// 检测当前方向移动的可能性; Move() 内调用
	bool ShootBullet(int bullet_id);										// 发射 id 号子弹[0,1],按下 J 或 1 键调用; PlayerControl() 内调用
	BulletShootKind CheckBomb(int i);										// 检测子弹移动过程是否击中东西; BulletMoving() 内调用
	void ClearWallOrStone(int id, int x, int y);							// id:子弹id; x,y:子弹凸出点的坐标, 根据不同方向位置不一样; CheckBomb() 内击中障碍物时调用
	void SignBox_8(int x, int y, int val);									// 标记 4 个 8*8 的格子; xx,y : 16*16 格子的中心点坐标;val : 需要标记的值	
	void SignBox_4(int cx, int cy, int val);								// 根据坦克中心坐标, 标记16个 4*4 格子
	static bool CheckBox_8(int iy, int jx);									// 参数是 [26][26] 的下标索引
	static void ProtectCamp(int val);

public:
	static std::list<PlayerBase *> *player_list_;							// 在GameControl内 赋值

private:
	bool is_died_;															// 生命是否用完,死亡
	byte player_id_ : 1;													    // [0-1] 玩家
	PlayerTank *player_tank_;												// 坦克类
	static BoxMark *box_mark_;

	IMAGE p12_image_;														// 1P\2P 图标
	int p12_image_x_;														// 图标坐标
	int p12_image_y_;

	IMAGE player_tank_ico_image_;											// 玩家坦克图标
	int player_tank_ico_image_x_;											// 图标坐标
	int player_tank_ico_image_y_;

	int player_life_;														// 玩家生命, 默认3
	int player_life_x_; 													// 图标坐标
	int	player_life_y_;

	IMAGE black_number_image_;												// 0123456789 黑色数字
	int tank_x_;															// 坦克中心点坐标
	int tank_y_;
	byte player_tank_level_ : 2;											// [0-3] 坦克级别,获得道具后升级坦克
	byte tank_dir_ : 2;														// 当前坦克方向0-左,1-上,2右,3-下
	static int dev_xy_[4][2];												// 四个方向的偏移量
	int speed_[4];															// speed_ * dev_xy_ 得到运动速度, 下标对应 player_tank_level_, 不同级别速度不一样

	Bullet bullet_[2];														// 两颗子弹
	int bullet1_counter_;													// 子弹 1 的计数, 子弹 1 发射多久后才能发射子弹 2
	bool is_moving_;														// 指示坦克是否移动, 传递到 GetTankImage() 获取移动的坦克

	Bomb bombs_[2];															// 爆炸结构体
	Blast blast_;															// 坦克爆炸结构, 

	Star star_;																// 坦克出现前的四角星闪烁
	Ring ring_;																// 保护圈类

	static Prop prop_;														// 道具类

	// GameControl 内循环检测该值, 然后在设置 EnemyBase 停止移动
	static bool get_time_prop_;												// 记录是否获得 时钟道具
	bool get_bomb_prop_;													// 地雷道具, 逻辑与 get_time_prop_ 相同
	static bool get_shovel_prop_;											// 玩家是否拥有铲子道具
	static int shovel_prop_counter_;

	TimeClock tank_timer_;													// 坦克移动计时器
	TimeClock bullet_timer_;												// 子弹移动速度
	TimeClock bomb_timer_;													// 子弹爆炸速度

	ScorePanel *score_panel_;												// 关卡结束后显示分数结果

	int kill_enemy_number_[4];												// 四种敌机类型,杀敌数量

	bool is_pause_;															// 被另外一个玩家击中, 暂停一段时间
	int pause_counter_;														// Move() 内计数停止移动, DrawTank() 内取模计数,实现坦克闪烁

	bool has_send_killednum_;												// has_send_kill_num_to_score_panel_; 防止 胜利后集中大本营或其它.多次发送数据到分数面板

	// CheckMoveable() 内检测; 
	// 坦克是否在 _ICE 上,进入 _ICE 的时候则设置为 true, 根据该 flag 设置 mTankTimer 时间差, 加快坦克移动, 出来则设置 false, 调回原来时间差
	bool on_ice_;
	bool automove_;
	int automove_counter_;
	int rand_counter_;														// 每次自动移动次数随机

	PlayerGameover player_gameover_;										// 玩家被消灭消失 gameover 字样

	static int move_speed_dev_[4];											// 四个级别坦克移动时间间隔
	static int bullet_speed_dev_[4];										// 不同级别子弹时间间隔速度
};
} // namespace battlecity


#endif