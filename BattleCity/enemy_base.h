#ifndef BATTLECITY_ENEMY_BASE_H_
#define BATTLECITY_ENEMY_BASE_H_

#include "tank.h"
#include "struct.h"
#include "time_clock.h"

namespace battlecity
{

// 敌机坦克
// 一个敌机实例化一个对象; 提供敌机类别(是否是道具坦克), 敌机级别[0-3];
// GameControl 内敌机被消灭, 不会将其从 EnemyList 内移除, 因为敌机子弹可能还会在运动;游戏结束分数面板显示完后释放敌机链表资源
class EnemyBase
{
public:
	EnemyBase(TankKind kind, byte level, BoxMark *box_mark);	// kind[0-1]; level [0-4]
	virtual ~EnemyBase();

	StarState ShowStar(const HDC &center_hdc, int &remain_number);// 显示闪烁四角星, true-正在显示, false-显示完毕
	void TankMoving(const HDC &center_hdc);						// 敌机移动
	virtual void DrawTank(const HDC &center_hdc);				// 纯绘制坦克, 子类覆绘制
	void DrawBullet(const HDC &center_hdc);						// 绘制子弹, 需要与子弹移动分开,
	bool ShootBullet();											// 发射子弹
	BulletShootKind BulletMoving();								// 子弹移动
	void Bombing(const HDC &center_hdc);
	virtual bool BeKilled(bool kill_anyway);					// 敌机被消灭, 清除 SignBox 标记, 参数表示玩家获得地雷直接kill掉BigestTank
	bool Blasting(const HDC &center_hdc);						// 显示坦克爆炸图, true 标识爆炸完,GameControl 获取返回值然后将该敌机删除

	
	static void SetPause(bool);									// 由 GameControl 内设置, 设置 mPause, 然后 ShootBullet() 检测停止发射子弹

	int GetId();												// 返回敌机 id
	TankKind GetKind();											// 返回敌机类型, 是否是道具坦克
	byte GetLevel();

private:
	void SignBox_8(int x, int y, int value);
	void SignBox_4(int x, int y, int value);					// 标记或取消 4*4 大小的格子为坦克;
		
	void SignBullet(int left_x, int top_y, byte dir, int val);	// 标记子弹头所在的一个 4 * 4 格子; 参数是子弹图片左上角坐标

	bool CheckBox_8();											// 检测某个 box_8 是否可以放置坦克, 参数是 16*16 格子的中心点, 与坦克坐标规则相同
	bool CheckMoveable();
	void RejustDirPosition();									// 重新定位坦克方向, 调正坦克位置, 保持在格子上
	BulletShootKind CheckBomb();								// 移动子弹（子弹碰撞检测）
	void ShootWhat(int, int);									// 检测射中何物

	void ShootBack();											// 回头射击; 在TankMoving()内调用

protected:
	int enemy_id_;												// 区别敌机与敌机
	TankKind enemy_tank_kind_;									// 敌机类别, 道具坦克和普通坦克两种, [0-1]
	byte enemy_tank_level_ : 2;									// 敌机坦克4个级别 [0-3]
	bool is_died_;												// 是否被被消灭, 被击中后设置为 true, 敌机检测改值不能移动
	BoxMark *box_mark_;											// 指向格子标记结构, 由 GameControl 传递进来

	int tank_x_; 												// 坦克坐标, 坦克的中心点
	int tank_y_;
	byte tank_dir_ : 2;											// 坦克方向
	byte tank_image_index_ : 6;									// 坦克移动切换图片
	int step_numbers_;											// 当前方向移动的步数, 一定步数后或者遇到障碍物变换方向并重新计算;
	static int dev_xy_[4][2];									// 四个方向的偏移量
	int speed_[4];												// speed_ * dev_xy_ 得到运动速度, 下标对应 player_tank_level_, 不同级别速度不一样

	static bool is_pause_;										// 由 GameControl 控制, 然后在发射的时候检测这个值, 如果敌机被暂停则停止发射子弹

	Bullet bullet_;
	int shoot_counter_;											// 设置一个随机数, 随机计数差后发射子弹

	Bomb bombs_;												// 爆炸结构体
	Blast blast_;												// 坦克爆炸结构
	Star star_;													// 四角星闪烁类

	TimeClock tank_timer_;										// 敌机移动速度
	TimeClock bullet_timer_;									// 子弹移动速度
	TimeClock shoot_timer_;										// 发射子弹频率
	TimeClock bomb_timer_;										// 子弹爆炸速度

	TimeClock shoot_back_timer_;								// 坦克回头射击,然后短距离再次变向

	IMAGE score_image_[4];										// 100\200.. 分, 敌机爆炸完后显示的
};

// 前三种普通坦克
class CommonTank : public EnemyBase
{
public:
	CommonTank(byte level, BoxMark *box_mark);
	~CommonTank();
	void DrawTank(const HDC &center_hdc);						// 纯绘制坦克

public:
	TankInfo *tank_info_;										// 灰色坦克
};

// 前三种道具坦克
class PropTank : public EnemyBase
{
public:
	PropTank(byte level, BoxMark *box_mark);
	~PropTank();
	void DrawTank(const HDC &center_hdc);						// 纯绘制坦克

public:
	TankInfo *tank_info_[2];									// 存储灰色和红色的坦克
	byte index_counter_ : 6;									// 下标索引变色
};

// 第四种最大坦克 (道具是红黄灰, 普通是绿黄灰)
class BigestTank : public EnemyBase
{
public:
	BigestTank(TankKind kind, BoxMark *box_mark);
	~BigestTank();
	void DrawTank(const HDC &center_hdc);						// 纯绘制坦克
	bool BeKilled(bool kill_anyway);

public:
	TankInfo *tank_info_[4];									// 灰,红黄,绿
	byte index_counter_ : 6;									// 下标索引变色
	int hp_;													// 血量；这种坦克需要击中四次才会爆炸
};
} // namespace battlecity


#endif