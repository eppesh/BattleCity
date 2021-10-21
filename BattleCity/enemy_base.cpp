#include "stdafx.h"
#include "enemy_base.h"
#include "mci_sound.h"

namespace battlecity
{
//////////////////////////////////////////////////////////////////////////

// 敌机坦克控制

bool EnemyBase::is_pause_ = false;
int EnemyBase::dev_xy_[4][2] = { { -1, 0 },{ 0, -1 },{ 1, 0 },{ 0, 1 } };	// 依次左上右下

// 生成某个类别级别的敌机
EnemyBase::EnemyBase(TankKind kind, byte level, BoxMark *box_mark)
{
	enemy_tank_kind_ = kind;
	enemy_tank_level_ = level;
	is_died_ = false;
	box_mark_ = box_mark;

	int temp_x[3] = { kBoxSize, 13 * kBoxSize, 25 * kBoxSize };	// 坦克随机出现的三个位置 x 坐标
	tank_x_ = temp_x[rand() % 3];
	tank_y_ = kBoxSize;
	tank_dir_ = kDirDown;
	tank_image_index_ = 0;

	step_numbers_ = rand() % 200;					// 当前方向随机移动的步数

	// 不同级别坦克移动速度系数
	int temp[4] = { 1, 1, 1, 1 };
	for (int i = 0; i < 4; i++)
	{
		speed_[i] = temp[i];
	}

	// 子弹初始化; 其它数据在 PlayerBase 加载了
	bullet_.x = kShootTableX;
	bullet_.y = -100;
	bullet_.direction = kDirDown;
	for (int i = 0; i < 4; i++)
	{
		bullet_.speed[i] = 3;		// 不能超过 4
	}
	bullet_.killed_id = 0;			// 记录击中玩家坦克的id

	shoot_counter_ = rand() % 100 + 100;	// 随机间隔发射子弹

	// 爆炸图片
	bombs_.bomb_x = -100;
	bombs_.bomb_y = -100;
	bombs_.is_show_bomb = false;
	bombs_.counter = 0;

	// 不同级别敌机移动时间间隔
	int move_level[4] = { 30, 15, 27, 25 };
	tank_timer_.SetDrtTime(move_level[enemy_tank_level_]);

	// 子弹移动时间间隔
	bullet_timer_.SetDrtTime(30);

	// 发射子弹频率
	shoot_timer_.SetDrtTime(rand() % 1000 + 700);

	// 子弹爆炸速度
	bomb_timer_.SetDrtTime(37);

	// 坦克爆炸速率
	//mBlastTimer.SetDrtTime(37);

	// 设置回头射击频率
	shoot_back_timer_.SetDrtTime(rand() % 5000 + 9000);

	loadimage(&score_image_[0], _T("./res/big/100.gif"));
	loadimage(&score_image_[1], _T("./res/big/200.gif"));
	loadimage(&score_image_[2], _T("./res/big/300.gif"));
	loadimage(&score_image_[3], _T("./res/big/400.gif"));
}

EnemyBase::~EnemyBase()
{
}

void EnemyBase::DrawTank(const HDC &center_hdc)
{
}

// 显示坦克
StarState EnemyBase::ShowStar(const HDC &center_hdc, int &remain_number)
{
	StarState result = star_.EnemyShowStar(center_hdc, tank_x_, tank_y_, box_mark_);
	switch (result)
	{
		// 当前正在计时, 未显示
	case StarState::kStarTiming:
		break;

		// 当前位置显示四角星失败
	case StarState::kStarFailed:
	{
		// 重新选个随机位置
		int tempx[3] = { kBoxSize, 13 * kBoxSize, 25 * kBoxSize };
		tank_x_ = tempx[rand() % 3];
	}
	break;

	// 四角星开始出现
	case StarState::kStarOut:
		SignBox_4(tank_x_, tank_y_, kSignStar);	// 标记为 kSignStar = 2000, 2000 属于坦克不能穿行的标志
		break;

		// 四角星正在出现
	case StarState::kStarShowing:
		break;

		// 四角星停止
	case StarState::kStarStop:
		// 四角星消失. 敌机出现, 剩余坦克数-1;
		enemy_id_ = kTotalEnemyNumber - remain_number;
		remain_number -= 1;

		SignBox_4(tank_x_, tank_y_, kSignEnemy + 1000 * enemy_tank_level_ + 100 * enemy_tank_kind_ + enemy_id_);		// 坦克出现, 将四角星标记改为坦克标记
		break;

		// 坦克已经出现
	case StarState::kTankOut:
		break;
	}
	return result;
}

void EnemyBase::TankMoving(const HDC &center_hdc)
{
	if (!star_.IsStop() || is_died_ || tank_timer_.IsTimeOut() == false)
	{
		return;
	}

	// 移动前取消标记
	SignBox_4(tank_x_, tank_y_, kElementEmpty);

	// 内部计时, 一定时差后回头射击
	ShootBack();

	// 重定向
	if (step_numbers_-- < 0)
	{
		RejustDirPosition();
	}

	// 可移动
	if (CheckMoveable())
	{
		tank_x_ += dev_xy_[tank_dir_][0] * speed_[enemy_tank_level_];
		tank_y_ += dev_xy_[tank_dir_][1] * speed_[enemy_tank_level_];
	}
	else // 不可移动,重定向
	{
		RejustDirPosition();
	}

	// 在新位置重新标记
	SignBox_4(tank_x_, tank_y_, kSignEnemy + enemy_tank_level_ * 1000 + enemy_tank_kind_ * 100 + enemy_id_);
}

// 
void EnemyBase::DrawBullet(const HDC &center_hdc)
{
	// 如果子弹没有移动或者敌机死亡
	if (bullet_.x == kShootTableX /*|| is_died_*/)
	{
		return;
	}
	int dir = bullet_.direction;

	TransparentBlt(center_hdc, bullet_.x, bullet_.y, Bullet::bullet_size[dir][0],
		Bullet::bullet_size[dir][1], GetImageHDC(&Bullet::bullet_image[dir]),
		0, 0, Bullet::bullet_size[dir][0], Bullet::bullet_size[dir][1], 0x000000);
}

//
bool EnemyBase::ShootBullet()
{
	if (is_pause_ || bullet_.x != kShootTableX || shoot_timer_.IsTimeOut() == false || is_died_ || star_.IsStop() == false)
	{
		return false;
	}

	// 子弹发射点坐标
	bullet_.x = tank_x_ + Bullet::devto_tank[tank_dir_][0];
	bullet_.y = tank_y_ + Bullet::devto_tank[tank_dir_][1];
	bullet_.direction = tank_dir_;

	SignBullet(bullet_.x, bullet_.y, bullet_.direction, kSignEnemyBullet);
	return true;
}

//
BulletShootKind EnemyBase::BulletMoving()
{
	// 如果子弹没有移动或者敌机死亡
	if (bullet_.x == kShootTableX/* || is_died_*/ || !bullet_timer_.IsTimeOut())
	{
		return BulletShootKind::kNone;
	}

	// 如果子弹在爆炸
	BulletShootKind result = CheckBomb();
	switch (result)
	{
	case BulletShootKind::kCamp:
	case BulletShootKind::kOther:
	case BulletShootKind::kPlayer1:
	case BulletShootKind::kPlayer2:
		return result;

	case BulletShootKind::kNone:
		break;
	default:
		break;
	}

	int dir = bullet_.direction;
	SignBullet(bullet_.x, bullet_.y, dir, kElementEmpty);

	bullet_.x += dev_xy_[dir][0] * bullet_.speed[enemy_tank_level_];
	bullet_.y += dev_xy_[dir][1] * bullet_.speed[enemy_tank_level_];

	SignBullet(bullet_.x, bullet_.y, dir, kSignEnemyBullet);
	return BulletShootKind::kNone;
}

//
void EnemyBase::Bombing(const HDC &center_hdc)
{
	int index[3] = { 0,1,2 };
	if (bombs_.is_show_bomb)
	{
		TransparentBlt(center_hdc, bombs_.bomb_x - kBoxSize, bombs_.bomb_y - kBoxSize, kBoxSize * 2, kBoxSize * 2,
			GetImageHDC(&Bomb::bomb_image[index[bombs_.counter % 3]]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
		if (bomb_timer_.IsTimeOut())
		{
			if (bombs_.counter++ >= 3)
			{
				bombs_.counter = 0;
				bombs_.is_show_bomb = false;
			}
		}
	}
}

// 有Gamecontrol内检测, 然后调用 BigestTank.class 需要覆盖这个方法,因为它需要射击四次才能杀死
bool EnemyBase::BeKilled(bool kill_anyway)
{
	// 如果敌机还没有出现
	if (star_.IsStop() == false || blast_.IsBlasting() || is_died_ == true)
	{
		return false;
	}

	MciSound::PlaySounds(kSoundEnemyBomb);
	is_died_ = true;
	SignBox_4(tank_x_, tank_y_, kElementEmpty);

	// 设置开始爆炸参数
	blast_.SetBlasting(tank_x_, tank_y_);

	return true;
}

// 显示坦克爆炸效果, GameControl 内循环检测
bool EnemyBase::Blasting(const HDC &center_hdc)
{
	switch (blast_.EnemyBlasting(center_hdc, &score_image_[enemy_tank_level_]))
	{
	case BlastState::kNotBlast:
		break;

	case BlastState::kBlasting:
		break;

	case BlastState::kBlastEnd:
		return true;

	default:
		break;
	}
	return false;
}

void EnemyBase::SetPause(bool val)
{
	is_pause_ = val;
}

int EnemyBase::GetId()
{
	return enemy_id_;
}

TankKind EnemyBase::GetKind()
{
	return enemy_tank_kind_;
}

byte EnemyBase::GetLevel()
{
	return enemy_tank_level_;
}

// x,y 是 16*16 中心点坐标
void EnemyBase::SignBox_8(int x, int y, int value)
{
	// 右坦克中心索引转到左上角那个的 格子索引
	int iy = y / kBoxSize - 1;
	int jx = x / kBoxSize - 1;
	for (int i = iy; i < iy + 2; i++)
	{
		for (int j = jx; j < jx + 2; j++)
		{
			box_mark_->box_8[i][j] = value;
		}
	}
}

// 标记或取消坦克所在的 4*4 = 16 个格子
void EnemyBase::SignBox_4(int x, int y, int value)
{
	// 左右调整 x,y 到占据百分比最多的 16 个 4*4 的格子中心
	if (tank_dir_ == kDirLeft || tank_dir_ == kDirRight)
	{
		if (x > (x / kSmallBoxSize) * kSmallBoxSize + kSmallBoxSize / 2)	// 如果是靠近右边节点, 
		{
			x = (x / kSmallBoxSize + 1) * kSmallBoxSize;
		}
		else 
		{
			x = (x / kSmallBoxSize) * kSmallBoxSize;					// 靠近格子线上的左边节点
		}
	}
	// 上下
	else
	{
		if (y > (y / kSmallBoxSize) * kSmallBoxSize + kSmallBoxSize / 2)	// 如果是靠近格子下边节点
		{
			y = (y / kSmallBoxSize + 1) * kSmallBoxSize;
		}
		else
		{
			y = (y / kSmallBoxSize) * kSmallBoxSize;					// 靠近格子线上的上边节点
		}
	}

	// 右坦克中心索引转到左上角那个的 格子索引
	int iy = y / kSmallBoxSize - 2;
	int jx = x / kSmallBoxSize - 2;
	for (int i = iy; i < iy + 4; i++)
	{
		for (int j = jx; j < jx + 4; j++)
		{
			box_mark_->box_4[i][j] = value;
		}
	}
}

void EnemyBase::SignBullet(int left_x, int top_y, byte dir, int val)
{
	// 转换弹头坐标
	int hx = left_x + Bullet::devto_head[dir][0];
	int hy = top_y + Bullet::devto_head[dir][1];

	// 转换成 4*4 格子下标索引
	int b4i = hy / kSmallBoxSize;
	int b4j = hx / kSmallBoxSize;
	if (b4i > 51 || b4j > 51 || b4i < 0 || b4j < 0)
	{
		//printf("茶水间%d, %d\n", b4i, b4j);
		return;
	}

	box_mark_->bullet_4[b4i][b4j] = val;
}

// 检测某个16*16位置可以放坦克吗, x,y 16*16的中心点
bool EnemyBase::CheckBox_8()
{
	// 获取坦克左上角的 4*4 下标 
	int iy = tank_y_ / kSmallBoxSize - 2;
	int jx = tank_x_ / kSmallBoxSize - 2;
	for (int i = iy; i < iy + 4; i++)
	{
		for (int j = jx; j < jx + 4; j++)
		{
			// 检测四角星, 玩家,敌机,
			if (box_mark_->box_4[i][j] != kSignStar && box_mark_->box_4[i][j] > kElementForest)
			{
				return false;
			}
		}
	}
	return true;
}

//
bool EnemyBase::CheckMoveable()
{
	// 坦克中心坐标
	int tempx = tank_x_ + dev_xy_[tank_dir_][0] * speed_[enemy_tank_level_];
	int tempy = tank_y_ + dev_xy_[tank_dir_][1] * speed_[enemy_tank_level_];

	if (tempx < kBoxSize || tempy < kBoxSize || tempy > kCenterWidth - kBoxSize || tempx > kCenterHeight - kBoxSize)
	{
		// 如果遇到障碍物,将坦克坐标调整到格子线上. 不然坦克和障碍物会有几个像素点间隔
		switch (tank_dir_)
		{
		case kDirLeft:	
			tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;	
			break;	// tank_x_ 与 tempx 之间跨越了格子, 将坦克放到tank_x_所在的格子线上
		case kDirUp:	
			tank_y_ = (tank_y_ / kBoxSize) * kBoxSize;	
			break;
		case kDirRight: 
			tank_x_ = (tempx / kBoxSize) * kBoxSize;	
			break;
		case kDirDown:	
			tank_y_ = (tempy / kBoxSize) * kBoxSize;	
			break;
		default:
			break;
		}
		return false;
	}

	// 转换像素点所在的 xy[26][26] 下标
	int index_i = (int)tempy / kBoxSize;
	int index_j = (int)tempx / kBoxSize;

	// 检测 8*8 障碍物
	int dev[4][2][2] = { { { -1,-1 },{ 0,-1 } },{ { -1,-1 },{ -1,0 } },{ { -1,1 },{ 0,1 } },{ { 1,-1 },{ 1,0 } } };

	// 检测坦克 4*4 格子
	// 四个方向需要检测的两个 4*4 的格子与坦克中心所在 4*4 格子的下标偏移量
	int  dev_4[4][4][2] = { { { -2,-2 },{ 1,-2 },{ -1,-2 },{ 0,-2 } },{ { -2,-2 },{ -2,1 },{ -2,-1 },{ -2,0 } },
							{ { -2, 2 },{ 1, 2 },{ -1, 2 },{ 0, 2 } },{ { 2, -2 },{ 2, 1 },{ 2, -1 },{ 2, 0 } } };

	// 转换成 [52][52] 下标
	int index_4i = tempy / kSmallBoxSize;
	int index_4j = tempx / kSmallBoxSize;

	// -1, 0, 1, 2 都可以移动
	bool tank1 = box_mark_->box_4[index_4i + dev_4[tank_dir_][0][0]][index_4j + dev_4[tank_dir_][0][1]] <= kElementIce;
	bool tank2 = box_mark_->box_4[index_4i + dev_4[tank_dir_][1][0]][index_4j + dev_4[tank_dir_][1][1]] <= kElementIce;
	bool tank3 = box_mark_->box_4[index_4i + dev_4[tank_dir_][2][0]][index_4j + dev_4[tank_dir_][2][1]] <= kElementIce;
	bool tank4 = box_mark_->box_4[index_4i + dev_4[tank_dir_][3][0]][index_4j + dev_4[tank_dir_][3][1]] <= kElementIce;

	// 遇到障碍物
	if (box_mark_->box_8[index_i + dev[tank_dir_][0][0]][index_j + dev[tank_dir_][0][1]] > 2 ||
		box_mark_->box_8[index_i + dev[tank_dir_][1][0]][index_j + dev[tank_dir_][1][1]] > 2)
	{
		// 如果遇到障碍物,将坦克坐标调整到格子线上. 不然坦克和障碍物会有几个像素点间隔
		switch (tank_dir_)
		{
		case kDirLeft:	
			tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;	
			break;	// tank_x_ 与 tempx 之间跨越了格子, 将坦克放到tank_x_所在的格子线上
		case kDirUp:	
			tank_y_ = (tank_y_ / kBoxSize) * kBoxSize;	
			break;
		case kDirRight: 
			tank_x_ = (tempx / kBoxSize) * kBoxSize;	
			break;
		case kDirDown:	
			tank_y_ = (tempy / kBoxSize) * kBoxSize;	
			break;
		default:													
			break;
		}
		return false;
	}
	// 遇到坦克不用调节
	else if (!tank1 || !tank2 || !tank3 || !tank4)
	{
		return false;
	}
	return true;
}

//
void EnemyBase::RejustDirPosition()
{
	step_numbers_ = rand() % 250;

	// 原左右变上下方向
	if (tank_dir_ == kDirLeft || tank_dir_ == kDirRight)
	{
		if (tank_x_ > (tank_x_ / kBoxSize) * kBoxSize + kBoxSize / 2 - 1)	// 如果是靠近格子线上的右边节点, -1是修正
		{
			tank_x_ = (tank_x_ / kBoxSize + 1) * kBoxSize;
		}
		else
		{
			tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;					// 靠近格子线上的左边节点
		}
	}
	// 上下变左右
	else
	{
		if (tank_y_ > (tank_y_ / kBoxSize) * kBoxSize + kBoxSize / 2 - 1)	// 如果是靠近格子线上的下边节点, -1是修正
		{
			tank_y_ = (tank_y_ / kBoxSize + 1) * kBoxSize;
		}
		else
		{
			tank_y_ = (tank_y_ / kBoxSize) * kBoxSize;					// 靠近格子线上的上边节点
		}
	}

	/* 重定向, 必须调正位置后才能设置方向
	* 设置坦克向下移动的几率大些*/
	if (tank_dir_ == kDirLeft || tank_dir_ == kDirRight)
	{
		bool val = rand() % 100 < 70;
		if (val)
		{
			tank_dir_ = kDirDown;
		}
		else
		{
			tank_dir_ = rand() % 4;
		}
	}
	else
	{
		tank_dir_ = rand() % 4;
	}
}

//
BulletShootKind EnemyBase::CheckBomb()
{
	int dir = bullet_.direction;

	// 子弹头接触到障碍物的那个点, 左右方向点在上, 上下方向点在右
	int bombx = bullet_.x + Bullet::devto_head[dir][0];
	int bomby = bullet_.y + Bullet::devto_head[dir][1];

	bool flag = false;
	int adjust_x = 0, adjust_y = 0;		// 修正爆照图片显示的坐标

	// 不能用 bombx 代替 bullet_[i].x,否则会覆盖障碍物的检测
	if (bullet_.x < 0 && dir == kDirLeft)
	{
		flag = true;
		adjust_x = 5;					// 将爆炸图片向右移一点
	}
	else if (bullet_.y < 0 && dir == kDirUp)
	{
		flag = true;
		adjust_y = 5;
	}

	// 必须减去子弹的宽 4, 不然子弹越界, 后面检测导致 box_8 下标越界
	else if (bullet_.x >= kCenterWidth - 4 && dir == kDirRight)
	{
		flag = true;
		adjust_x = -4;
	}
	else if (bullet_.y >= kCenterHeight - 4 && dir == kDirDown)
	{
		flag = true;
		adjust_y = -4;
	}
	if (flag)
	{
		// 设定爆炸参数, 修正爆炸中心所在的格子,左右或上下偏移一个格子之类的..
		bullet_.x = kShootTableX;
		bombs_.is_show_bomb = true;
		bombs_.bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[dir][0]) * kSmallBoxSize;
		bombs_.bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[dir][1]) * kSmallBoxSize;
		bombs_.counter = 0;
		return BulletShootKind::kOther;
	}

	int tempi, tempj;

	// 坐标所在 8*8 格子的索引
	int b8i = bomby / kBoxSize;
	int b8j = bombx / kBoxSize;

	// 4*4 格子索引
	int bi = bomby / kSmallBoxSize;
	int bj = bombx / kSmallBoxSize;

	// 如果击中玩家子弹
	if (box_mark_->bullet_4[bi][bj] == kSignPlayerBullet + 0 * 10 + 0 ||
		box_mark_->bullet_4[bi][bj] == kSignPlayerBullet + 0 * 10 + 1 ||
		box_mark_->bullet_4[bi][bj] == kSignPlayerBullet + 1 * 10 + 0 ||
		box_mark_->bullet_4[bi][bj] == kSignPlayerBullet + 1 * 10 + 1)
	{
		bullet_.x = kShootTableX;
		box_mark_->bullet_4[bi][bj] = kSignWaitUnsign;		// 先标记中间值, 等待被击中的子弹检测到该值后,再擦除该标记
		return BulletShootKind::kOther;
	}
	else if (box_mark_->bullet_4[bi][bj] == kSignWaitUnsign)
	{
		bullet_.x = kShootTableX;
		box_mark_->bullet_4[bi][bj] = kElementEmpty;
		return BulletShootKind::kOther;
	}

	switch (dir)
	{
		// 左右检测子弹头所在的4*4格子和它上面相邻的那个
	case kDirLeft:
	case kDirRight:
	{
		int temp[2][2] = { { 0, 0 },{ -1, 0 } };
		for (int n = 0; n < 2; n++)
		{
			// 8*8 格子, 判断是否击鸟巢
			tempi = b8i + temp[n][0];
			tempj = b8j + temp[n][1];
			if (box_mark_->box_8[tempi][tempj] == kSignCamp)
			{
				bombs_.counter = 0;
				bullet_.x = kShootTableX;
				SignBox_8(13 * kBoxSize, 25 * kBoxSize, kElementEmpty);

				return BulletShootKind::kCamp;
			}

			// 4*4 检测
			tempi = bi + temp[n][0];
			tempj = bj + temp[n][1];
			if (box_mark_->box_4[tempi][tempj] == kElementWall || box_mark_->box_4[tempi][tempj] == kElementStone)
			{
				// 设定爆炸参数, 修正爆炸中心所在的格子,
				bullet_.x = kShootTableX;
				bombs_.is_show_bomb = true;				// 指示 i bomb 爆炸
				bombs_.bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][0]) * kSmallBoxSize;
				bombs_.bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][1]) * kSmallBoxSize;
				bombs_.counter = 0;
				ShootWhat(bombx, bomby);
				return BulletShootKind::kOther;
			}
			// 4*4 玩家格子
			else if (box_mark_->box_4[tempi][tempj] == kSignPlayer || box_mark_->box_4[tempi][tempj] == kSignPlayer + 1)
			{
				bullet_.x = kShootTableX;
				bombs_.is_show_bomb = true;				// 指示 i bomb 爆炸
				bombs_.bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][0]) * kSmallBoxSize;
				bombs_.bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][1]) * kSmallBoxSize;
				bombs_.counter = 0;
				return (BulletShootKind)box_mark_->box_4[tempi][tempj];
			}
		}
	}
	break;

	// 上下只检测左右相邻的两个格子
	case kDirUp:
	case kDirDown:
	{
		int temp[2][2] = { { 0, 0 },{ 0, -1 } };
		for (int n = 0; n < 2; n++)
		{
			// 8*8 格子, 判断是否击中鸟巢
			tempi = b8i + temp[n][0];
			tempj = b8j + temp[n][1];
			if (box_mark_->box_8[tempi][tempj] == kSignCamp)
			{
				bombs_.counter = 0;
				bullet_.x = kShootTableX;
				SignBox_8(13 * kBoxSize, 25 * kBoxSize, kElementEmpty);
				return BulletShootKind::kCamp;
			}

			// 4*4 检测
			tempi = bi + temp[n][0];
			tempj = bj + temp[n][1];
			if (box_mark_->box_4[tempi][tempj] == kElementWall || box_mark_->box_4[tempi][tempj] == kElementStone)
			{
				// 设定爆炸参数, 修正爆炸中心所在的格子
				bullet_.x = kShootTableX;
				bombs_.is_show_bomb = true;				// 指示 i bomb 爆炸
				bombs_.bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][0]) * kSmallBoxSize;
				bombs_.bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][1]) * kSmallBoxSize;
				bombs_.counter = 0;
				ShootWhat(bombx, bomby);
				return BulletShootKind::kOther;
			}
			// 4*4 玩家小格子
			else if (box_mark_->box_4[tempi][tempj] == kSignPlayer || box_mark_->box_4[tempi][tempj] == kSignPlayer + 1)
			{
				bullet_.x = kShootTableX;
				bombs_.is_show_bomb = true;				// 指示 i bomb 爆炸
				bombs_.bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][0]) * kSmallBoxSize;
				bombs_.bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][1]) * kSmallBoxSize;
				bombs_.counter = 0;
				return (BulletShootKind)box_mark_->box_4[tempi][tempj];
			}
		}
	}
	break;
	default:
		break;
	}
	return BulletShootKind::kNone;
}

void EnemyBase::ShootWhat(int bulletx, int bullety)
{
	int boxi = bullety / kSmallBoxSize;
	int boxj = bulletx / kSmallBoxSize;
	int tempx, tempy;
	switch (bullet_.direction)
	{
	case kDirLeft:
	case kDirRight:
	{
		// 相邻的四个 4*4 格子, 顺序不能变, 后面用到下标判断
		int temp[4][2] = { { -2, 0 },{ -1, 0 },{ 0, 0 },{ 1, 0 } };
		for (int i = 0; i < 4; i++)
		{
			tempx = boxi + temp[i][0];
			tempy = boxj + temp[i][1];
			if (box_mark_->box_4[tempx][tempy] == kElementWall)
			{
				box_mark_->box_4[tempx][tempy] = kElementClear;
			}

			// 转到 tempx,tempy所在的 8*8 格子索引
			int n = tempx / 2;
			int m = tempy / 2;

			// 检测 8*8 格子内的4个 4*4 的小格子是否全部被清除,
			bool isClear = true;
			for (int a = 2 * n; a < 2 * n + 2; a++)
			{
				for (int b = 2 * m; b < 2 * m + 2; b++)
				{
					if (box_mark_->box_4[a][b] != kElementClear)
					{
						isClear = false;
					}
				}
			}
			if (isClear)
			{
				box_mark_->box_8[n][m] = kElementEmpty;
			}
		}
	}
	break;

	case kDirUp:
	case kDirDown:
	{
		// 相邻的四个 4*4 格子, 顺序不能变, 后面用到下标判断
		int temp[4][2] = { { 0, -2 },{ 0, -1 },{ 0, 0 },{ 0, 1 } };
		for (int i = 0; i < 4; i++)
		{
			tempx = boxi + temp[i][0];
			tempy = boxj + temp[i][1];
			if (box_mark_->box_4[tempx][tempy] == kElementWall)
			{
				box_mark_->box_4[tempx][tempy] = kElementClear;
			}

			// 转到 tempx,tempy所在的 8*8 格子索引
			int n = tempx / 2;
			int m = tempy / 2;

			// 检测 8*8 格子内的4个 4*4 的小格子是否全部被清除,
			bool isClear = true;
			for (int a = 2 * n; a < 2 * n + 2; a++)
			{
				for (int b = 2 * m; b < 2 * m + 2; b++)
				{
					if (box_mark_->box_4[a][b] != kElementClear)
					{
						isClear = false;
					}
				}
			}

			// 4 个 4*4 组成的 8*8 格子被清除完
			if (isClear)
			{
				box_mark_->box_8[n][m] = kElementEmpty;
			}
		}
	}
	break;

	default:
		break;
	}
}

/*
* 设置敌机回头运动较小的一段距离
*/
void EnemyBase::ShootBack()
{
	if (!shoot_back_timer_.IsTimeOut())
	{
		return;
	}

	int back_dir[4] = { kDirRight, kDirDown, kDirLeft, kDirUp };

	// 原左右变上下方向
	if (tank_dir_ == kDirLeft || tank_dir_ == kDirRight)
	{
		if (tank_x_ > (tank_x_ / kBoxSize) * kBoxSize + kBoxSize / 2 - 1)	// 如果是靠近格子线上的右边节点, -1是修正
		{
			tank_x_ = (tank_x_ / kBoxSize + 1) * kBoxSize;
		}
		else
		{
			tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;					// 靠近格子线上的左边节点
		}
	}
	// 上下变左右
	else
	{
		if (tank_y_ > (tank_y_ / kBoxSize) * kBoxSize + kBoxSize / 2 - 1)	// 如果是靠近格子线上的下边节点, -1是修正
		{
			tank_y_ = (tank_y_ / kBoxSize + 1) * kBoxSize;
		}
		else
		{
			tank_y_ = (tank_y_ / kBoxSize) * kBoxSize;					// 靠近格子线上的上边节点
		}
	}

	step_numbers_ = rand() % 30 + 30;
	tank_dir_ = back_dir[tank_dir_];
}

////////////////////////////////////////////////////////

CommonTank::CommonTank(byte level, BoxMark *box_mark) :
	EnemyBase(TankKind::kCommon, level, box_mark)
{
	tank_info_ = new TankInfo(kGrayTank, level, true);
}

CommonTank::~CommonTank()
{
	delete tank_info_;
	printf("~CommonTank::CommonTank() .. \n");
}

// 
void CommonTank::DrawTank(const HDC &center_hdc)
{
	if (!star_.is_outed_ || is_died_)
	{
		return;
	}
	IMAGE temp = tank_info_->GetTankImage(tank_dir_, tank_image_index_++ / 3 % 2);
	TransparentBlt(center_hdc, (int)tank_x_ - kBoxSize, (int)tank_y_ - kBoxSize, kBoxSize * 2, kBoxSize * 2, 
		GetImageHDC(&temp), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
}

//////////////////////////////////////////////////////////////////////////////


PropTank::PropTank(byte level, BoxMark *box_mark) :
	EnemyBase(TankKind::kProp, level, box_mark)
{
	tank_info_[0] = new TankInfo(kGrayTank, level, true);
	tank_info_[1] = new TankInfo(kRedTank, level, true);
}

PropTank::~PropTank()
{
	// 不能直接 delete[] mTank??
	for (int i = 0; i < 2; i++)
	{
		delete tank_info_[i];
	}
	printf("~PropTank::PropTank()..\n");
}

// 
void PropTank::DrawTank(const HDC &center_hdc)
{
	if (!star_.is_outed_ || is_died_)
	{
		return; 
	}
	IMAGE temp = tank_info_[index_counter_++ / 6 % 2]->GetTankImage(tank_dir_, tank_image_index_++ / 3 % 2);
	TransparentBlt(center_hdc, (int)tank_x_ - kBoxSize, (int)tank_y_ - kBoxSize, kBoxSize * 2, kBoxSize * 2,
		GetImageHDC(&temp), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
}

//////////////////////////////////////////////////////////////

BigestTank::BigestTank(TankKind kind, BoxMark *box_mark) :
	EnemyBase(kind, 3, box_mark)
{
	tank_info_[kGrayTank] = new TankInfo(kGrayTank, 3, true);
	tank_info_[kYellowTank] = new TankInfo(kYellowTank, 3, true);
	tank_info_[kRedTank] = new TankInfo(kRedTank, 3, true);
	tank_info_[kGreenTank] = new TankInfo(kGreenTank, 3, true);

	hp_ = 4;
}

BigestTank::~BigestTank()
{
	for (int i = 0; i < 4; i++)
	{
		delete tank_info_[i];
	}
	printf("~BigestTank::BigestTank()\n");
}

void BigestTank::DrawTank(const HDC &center_hdc)
{
	if (!star_.is_outed_ || is_died_)
	{
		return;
	}

	// 道具坦克和普通坦克变色区别
	TankInfo *temp[2] = { tank_info_[kGrayTank], tank_info_[kGrayTank] };

	switch (enemy_tank_kind_)
	{
	case TankKind::kProp:
		switch (hp_)
		{
		case 4:
			temp[0] = tank_info_[kRedTank];
			temp[1] = tank_info_[kGrayTank];
			break;
		case 3:
			temp[0] = tank_info_[kRedTank];
			temp[1] = tank_info_[kYellowTank];
			break;
		case 2:
			temp[0] = tank_info_[kYellowTank];
			temp[1] = tank_info_[kGrayTank];
			break;
		default:
			break;
		}
		break;
	case TankKind::kCommon:
		switch (hp_)
		{
		case 4:
			temp[0] = tank_info_[kGreenTank];
			temp[1] = tank_info_[kGrayTank];
			break;
		case 3:
			temp[0] = tank_info_[kGrayTank];
			temp[1] = tank_info_[kYellowTank];
			break;
		case 2:
			temp[0] = tank_info_[kYellowTank];
			temp[1] = tank_info_[kGreenTank];
			break;
		default:
			break;
		}
		break;
	}

	if (temp == NULL)
	{
		printf("错误!. EnemyBase.cpp");
		return;
	}
	IMAGE image = temp[index_counter_++ / 7 % 2]->GetTankImage(tank_dir_, tank_image_index_++ / 3 % 2);
	TransparentBlt(center_hdc, (int)tank_x_ - kBoxSize, (int)tank_y_ - kBoxSize, kBoxSize * 2, kBoxSize * 2,
		GetImageHDC(&image), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
}

bool BigestTank::BeKilled(bool kill_anyway)
{
	if (star_.is_outed_ == false || blast_.IsBlasting() || is_died_ == true)
	{
		return false;
	}

	MciSound::PlaySounds(kSoundBin);
	if (--hp_ <= 0 || kill_anyway)
	{
		hp_ = 0;
		return this->EnemyBase::BeKilled(kill_anyway);
	}
	return false;
}

} // namespace battlecity