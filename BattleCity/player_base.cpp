#include "stdafx.h"
#include "player_base.h"
#include "mci_sound.h"

namespace battlecity
{
// PlayerBase 类静态数据

int PlayerBase::dev_xy_[4][2] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} };	// 依次左上右下
Prop PlayerBase::prop_;
bool PlayerBase::get_time_prop_ = false;
bool PlayerBase::get_shovel_prop_ = false;
int PlayerBase::shovel_prop_counter_ = 0;
std::list<PlayerBase *> *PlayerBase::player_list_ = NULL;
BoxMark *PlayerBase::box_mark_ = NULL;
int PlayerBase::move_speed_dev_[4] = { 21, 19, 17, 15 };
int PlayerBase::bullet_speed_dev_[4] = { 17, 16, 15, 14 };

PlayerBase::PlayerBase(byte player, BoxMark *box_mark)
{
	int i = 0;
	player_id_ = player;
	player_tank_ = new PlayerTank(player_id_);
	box_mark_ = box_mark;

	player_life_ = 2;		// 玩家 HP
	player_tank_level_ = 0;

	Init();

	// 不同玩家数据不同
	if (player_id_ == 0)
	{
		loadimage(&p12_image_, _T("./res/big/1P.gif"));		// 1P\2P图标
	}
	else
	{
		loadimage(&p12_image_, _T("./res/big/2P.gif"));
	}

	// 共同的数据
	loadimage(&player_tank_ico_image_, _T("./res/big/playertank-ico.gif"));	// 玩家坦克图标
	loadimage(&black_number_image_, _T("./res/big/black-number.gif"));	// 黑色数字

	// Bullet 数据初始化
	// 加载子弹图片
	TCHAR buf[100];
	for (i = 0; i < 4; i++)
	{
		_stprintf_s(buf, _T("./res/big/bullet-%d.gif"), i);
		loadimage(&Bullet::bullet_image[i], buf);
	}

	// 爆炸图片
	for (i = 0; i < 3; i++)
	{
		_stprintf_s(buf, _T("./res/big/bumb%d.gif"), i);
		loadimage(&Bomb::bomb_image[i], buf);
	}

	bomb_timer_.SetDrtTime(20);	// 不能设置太小..

	// 显示分数面板
	score_panel_ = new ScorePanel(player_id_);
}

PlayerBase::~PlayerBase()
{
	delete player_tank_;
	delete score_panel_;
	printf("PLayerBase:: ~PlayerBase()\n");
}

void PlayerBase::Init()
{
	prop_.Init(box_mark_);
	star_.Init();
	ring_.Init();
	blast_.Init();
	player_gameover_.Init(player_id_);

	get_time_prop_ = false;
	get_bomb_prop_ = false;
	get_shovel_prop_ = false;

	// 不同玩家数据不同
	if (player_id_ == 0)
	{
		p12_image_x_ = 233;									// 1P\2P 坐标
		p12_image_y_ = 129;
		player_tank_ico_image_x_ = 232;						// 玩家坦克图标坐标
		player_tank_ico_image_y_ = 137;
		player_life_x_ = 240;								// 玩家生命值坐标
		player_life_y_ = 137;
		tank_x_ = 4 * 16 + kBoxSize;						// 坦克首次出现时候的中心坐标
		tank_y_ = 12 * 16 + kBoxSize;

		tank_timer_.SetDrtTime(move_speed_dev_[player_tank_level_]);		// 坦克移动速度, 不同级别不同玩家 不一样
		bullet_timer_.SetDrtTime(bullet_speed_dev_[player_tank_level_]);
	}
	else
	{
		p12_image_x_ = 233;
		p12_image_y_ = 153;
		player_tank_ico_image_x_ = 232;
		player_tank_ico_image_y_ = 161;
		player_life_x_ = 240;
		player_life_y_ = 161;
		tank_x_ = 8 * 16 + kBoxSize;
		tank_y_ = 12 * 16 + kBoxSize;

		tank_timer_.SetDrtTime(move_speed_dev_[player_tank_level_]);
		bullet_timer_.SetDrtTime(bullet_speed_dev_[player_tank_level_]);
	}

	int i = 0;
	is_died_ = false;
	tank_dir_ = kDirUp;		// 坦克方向

	// 子弹结构数据
	int temp_speed[4] = { 2,3,3,4 };			// 不能超过 4 !! 会跳跃格子判断.根据坦克级别分配子弹速度系数
	for (i = 0; i < 2; i++)
	{
		bullet_[i].x = kShootTableX;			// x 坐标用于判断是否可以发射
		bullet_[i].y = -1000;
		bullet_[i].direction = kDirUp;
		bullet_[i].killed_id = 0;				// 记录击中的敌机 id

												// 根据坦克级别分配子弹速度系数
		for (int j = 0; j < 4; j++)
		{
			bullet_[i].speed[j] = temp_speed[j];
		}
	}

	bullet1_counter_ = 9;						// 子弹 1 运动 N 个循环后才可以发射子弹 2 
	is_moving_ = false;

	// 爆炸图片
	for (i = 0; i < 2; i++)
	{
		bombs_[i].bomb_x = -100;
		bombs_[i].bomb_y = -100;
		bombs_[i].is_show_bomb = false;
		bombs_[i].counter = 0;
	}

	// SendKillNumToScorePanel() 内使用
	has_send_killednum_ = false;

	// 杀敌数
	for (i = 0; i < 4; i++)
	{
		kill_enemy_number_[i] = 0;
	}

	is_pause_ = false;
	pause_counter_ = 0;

	// 坦克是否在冰上移动
	on_ice_ = false;
	automove_ = false;
	automove_counter_ = 0;
	rand_counter_ = rand() % 6 + 3;
}

// 绘制玩家的一些数据: 1P\2P 坦克图标 生命
void PlayerBase::DrawPlayerTankIco(const HDC &right_panel_hdc)
{
	// 绘制1P/2P
	TransparentBlt(right_panel_hdc, p12_image_x_, p12_image_y_, kPlayer2IcoSizeX, kPlayer2IcoSizeY,
		GetImageHDC(&p12_image_), 0, 0, kPlayer2IcoSizeX, kPlayer2IcoSizeY, 0xffffff);
	// 绘制坦克图标
	TransparentBlt(right_panel_hdc, player_tank_ico_image_x_, player_tank_ico_image_y_, kPlayerTankIcoSizeX, kPlayerTankIcoSizeY,
		GetImageHDC(&player_tank_ico_image_), 0, 0, kPlayerTankIcoSizeX, kPlayerTankIcoSizeY, 0x000000);
	// 绘制玩家生命数字
	TransparentBlt(right_panel_hdc, player_life_x_, player_life_y_, kBlackNumberSize, kBlackNumberSize,
		GetImageHDC(&black_number_image_), kBlackNumberSize * player_life_, 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);
}

bool PlayerBase::ShowStar(const HDC &center_hdc)
{
	switch (star_.ShowStar(center_hdc, tank_x_, tank_y_))
	{
	case StarState::kStarShowing:
		break;

	case StarState::kStarStop:
		SignBox_8(tank_x_, tank_y_, kElementEmpty);		// 防止玩家绘制地图把坦克出现的位置遮挡住
		SignBox_4(tank_x_, tank_y_, kSignPlayer + player_id_);
		ring_.SetShowable(3222);
		return kStopShowStar;

	case StarState::kTankOut:
		return kStopShowStar;
	}

	return kShowingStar;
}

//
void PlayerBase::DrawPlayerTank(const HDC &canvas_hdc)
{
	if (!star_.IsStop() || is_died_ || blast_.IsBlasting())
	{
		return;
	}

	// 0-5不显示坦克. 6-11 显示.. 依次类推
	if (is_pause_ && pause_counter_++ / 10 % 2 != 0)
	{
		if (pause_counter_ > 266)
		{
			is_pause_ = false;
		}
		return;
	}

	IMAGE tank = player_tank_->GetTankImage(player_tank_level_, tank_dir_, is_moving_);
	TransparentBlt(canvas_hdc, (int)(tank_x_ - kBoxSize), (int)(tank_y_ - kBoxSize), kBoxSize * 2, kBoxSize * 2, GetImageHDC(&tank), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);

	// 显示保护环
	if (ring_.can_show_)
	{
		if (ring_.ShowRing(canvas_hdc, tank_x_, tank_y_) == false)
		{
			// 防止玩家爆炸到结束的时候,重生位置被敌机占用,然后玩家标记被擦除
			SignBox_4(tank_x_, tank_y_, kSignPlayer + player_id_);
		}
	}
}

void PlayerBase::DrawBullet(const HDC &center_hdc)
{
	for (int i = 0; i < 2; i++)
	{
		// 子弹在移动
		if (bullet_[i].x != kShootTableX)
		{
			int dir = bullet_[i].direction;

			TransparentBlt(center_hdc, bullet_[i].x, bullet_[i].y, Bullet::bullet_size[dir][0],
				Bullet::bullet_size[dir][1], GetImageHDC(&Bullet::bullet_image[dir]),
				0, 0, Bullet::bullet_size[dir][0], Bullet::bullet_size[dir][1], 0x000000);
		}
	}
}

//
bool PlayerBase::PlayerControl()
{
	if (is_died_ || blast_.IsBlasting() || !star_.IsStop())
	{
		return true;
	}

	// 
	if (automove_)
	{
		if (automove_counter_++ < rand_counter_)
		{
			Move(tank_dir_);
		}
		else
		{
			automove_ = false;
			automove_counter_ = 0;
		}
	}

	switch (player_id_)
	{
	case 0:										// 玩家一
		if (GetAsyncKeyState('A') & 0x8000)
		{
			// 同方向移动才开启自动移动
			if (on_ice_ && tank_dir_ == kDirLeft)	// 如果坦克朝向是 左
			{
				automove_ = true;
				automove_counter_ = 0;
				rand_counter_ = rand() % 8 + 7;
			}
			if (is_moving_ == false) 
			{
				MciSound::PlayMovingSound(true);
			}
			is_moving_ = true;
			Move(kDirLeft);
		}
		else if (GetAsyncKeyState('W') & 0x8000)
		{
			// 同方向移动才开启自动移动
			if (on_ice_ && tank_dir_ == kDirUp)
			{
				automove_ = true;
				automove_counter_ = 0;
				rand_counter_ = rand() % 8 + 7;
			}

			if (is_moving_ == false)
			{
				MciSound::PlayMovingSound(true);
			}
			is_moving_ = true;
			Move(kDirUp);
		}
		else if (GetAsyncKeyState('D') & 0x8000)
		{
			// 同方向移动才开启自动移动
			if (on_ice_ && tank_dir_ == kDirRight)
			{
				automove_ = true;
				automove_counter_ = 0;
				rand_counter_ = rand() % 8 + 7;
			}

			if (is_moving_ == false)
			{
				MciSound::PlayMovingSound(true);
			}
			is_moving_ = true;
			Move(kDirRight);
		}
		else if (GetAsyncKeyState('S') & 0x8000)
		{
			// 同方向移动才开启自动移动
			if (on_ice_ && tank_dir_ == kDirDown)
			{
				automove_ = true;
				automove_counter_ = 0;
				rand_counter_ = rand() % 8 + 7;
			}
			if (is_moving_ == false)
			{
				MciSound::PlayMovingSound(true);
			}
			is_moving_ = true;
			Move(kDirDown);
		}
		else if (is_moving_)		// 松开按键，停止播放移动声音；如果按下并快速松开，声音播放后又立即被暂停，所以听不到声音
		{
			is_moving_ = false;
			MciSound::PlayMovingSound(false);
		}

		// 不能加 else if, 不然移动时候无法发射子弹
		if (GetAsyncKeyState('J') & 0x8000)	// 发射子弹
		{
			if (!ShootBullet(0))
			{
				if (ShootBullet(1))
				{
					MciSound::PlaySounds(kSoundShoot1);
				}
			}
			else
			{
				MciSound::PlaySounds(kSoundShoot0);
			}
		}
		break;

	case 1:										// 玩家二
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			// 同方向移动才开启自动移动
			if (on_ice_ && tank_dir_ == kDirLeft)
			{
				automove_ = true;
				automove_counter_ = 0;
				rand_counter_ = rand() % 8 + 7;
			}

			if (is_moving_ == false)
			{
				MciSound::PlayMovingSound(true);
			}
			is_moving_ = true;
			Move(kDirLeft);
		}
		else if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			// 同方向移动才开启自动移动
			if (on_ice_ && tank_dir_ == kDirUp)
			{
				automove_ = true;
				automove_counter_ = 0;
				rand_counter_ = rand() % 8 + 7;
			}

			if (is_moving_ == false)
			{
				MciSound::PlayMovingSound(true);
			}
			is_moving_ = true;
			Move(kDirUp);
		}
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			// 同方向移动才开启自动移动
			if (on_ice_ && tank_dir_ == kDirRight)
			{
				automove_ = true;
				automove_counter_ = 0;
				rand_counter_ = rand() % 8 + 7;
			}

			if (is_moving_ == false)
			{
				MciSound::PlayMovingSound(true);
			}
			is_moving_ = true;
			Move(kDirRight);
		}
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			// 同方向移动才开启自动移动
			if (on_ice_ && tank_dir_ == kDirDown)
			{
				automove_ = true;
				automove_counter_ = 0;
				rand_counter_ = rand() % 8 + 7;
			}

			if (is_moving_ == false)
			{
				MciSound::PlayMovingSound(true);
			}
			is_moving_ = true;
			Move(kDirDown);
		}
		else if (is_moving_)
		{
			MciSound::PlayMovingSound(false);
			is_moving_ = false;
		}

		// 数字键 1 发射子弹; 0x31对应数字1键（非数字键的1，如笔记本上没有数字键盘时，需要用Q键左上方那个1键）
		if ((GetAsyncKeyState(VK_NUMPAD1) & 0x8000) || (GetAsyncKeyState(0x31) & 0x8000))
		{
			if (!ShootBullet(0))
			{
				if (ShootBullet(1))
				{
					MciSound::PlaySounds(kSoundShoot3);
				}
			}
			else
			{
				MciSound::PlaySounds(kSoundShoot2);
			}
		}
		break;
	default:
		break;
	}
	return true;
}

//
BulletShootKind PlayerBase::BulletMoving(const HDC &center_hdc)
{
	if (/* is_died_*/ bullet_timer_.IsTimeOut() == false)
	{
		return BulletShootKind::kNone;
	}


	for (int i = 0; i < 2; i++)
	{
		// 子弹在移动
		if (bullet_[i].x != kShootTableX)
		{
			int dir = bullet_[i].direction;

			// 检测打中障碍物与否
			BulletShootKind kind = CheckBomb(i);
			if (kind == BulletShootKind::kCamp || kind == BulletShootKind::kPlayer1 || kind == BulletShootKind::kPlayer2)
			{
				return kind;
			}
			else if (kind == BulletShootKind::kOther)
			{
				continue;
			}

			// 先检测再取消标记
			SignBullet(bullet_[i].x, bullet_[i].y, dir, kElementEmpty);

			bullet_[i].x += dev_xy_[dir][0] * bullet_[i].speed[player_tank_level_];
			bullet_[i].y += dev_xy_[dir][1] * bullet_[i].speed[player_tank_level_];

			// 记录子弹 1 的步数, 决定可否发射子弹 2
			if (i == 0)
			{
				bullet1_counter_--;
			}

			SignBullet(bullet_[i].x, bullet_[i].y, dir, kSignPlayerBullet + player_id_ * 10 + i);
		}
	}

	return BulletShootKind::kNone;
}


void PlayerBase::Bombing(const HDC &center_hdc)
{
	int index[6] = { 0,1,1,2,2,1 };
	for (int i = 0; i < 2; i++)
	{
		if (bombs_[i].is_show_bomb)
		{
			TransparentBlt(center_hdc, bombs_[i].bomb_x - kBoxSize, bombs_[i].bomb_y - kBoxSize, kBoxSize * 2, kBoxSize * 2,
				GetImageHDC(&Bomb::bomb_image[index[bombs_[i].counter % 6]]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
			// bug?	if (bomb_timer_.IsTimeOut())
			{
				if (bombs_[i].counter++ >= 6)
				{
					bombs_[i].counter = 0;
					bombs_[i].is_show_bomb = false;
				}
			}
		}
	}
}

// GameControl 内调用, 通过参数将 bullet_.killed_id 传递进去
void PlayerBase::GetKillEnemy(int &bullet1, int &bullet2)
{
	bullet1 = bullet_[0].killed_id;
	bullet2 = bullet_[1].killed_id;

	// 重置标志
	bullet_[0].killed_id = 0;
	bullet_[1].killed_id = 0;
}


void PlayerBase::BeKilled()
{
	// 如果显示着保护环不会受到攻击
	if (ring_.can_show_)
	{
		return;
	}

	MciSound::PlaySounds(kSoundPlayerBomb);
	SignBox_4(tank_x_, tank_y_, kElementEmpty);
	
	blast_.SetBlasting(tank_x_, tank_y_);
}

// 玩家被击中爆炸
bool PlayerBase::Blasting(const HDC &center_hdc)
{
	BlastState result = blast_.Blasting(center_hdc);
	switch (result)
	{
	case BlastState::kNotBlast:
		break;

	case BlastState::kBlasting:
		break;

	case BlastState::kBlastEnd:
		// 检测是否可以重生
		if (player_life_-- <= 0)
		{
			is_died_ = true;
			player_life_ = 0;
			return true;
		}
		else
		{
			Reborn();
		}
		break;

	default:
		break;
	}
	return false;
}

//
const int &PlayerBase::GetID()
{
	return player_id_;
}

/*GameControl 内循环调用*/
bool PlayerBase::IsGetTimeProp()
{
	bool temp = get_time_prop_;
	get_time_prop_ = false;
	return temp;
}

bool PlayerBase::IsGetShvelProp()
{
	if (get_shovel_prop_)
	{
		// 刚获得铲子道具
		if (shovel_prop_counter_++ == 0)
		{
			ProtectCamp(kElementStone);
		}
		else if (shovel_prop_counter_ > 31000 && shovel_prop_counter_ < 35400)		// 显示一段时间后闪烁
		{
			int val = kElementStone;
			if (shovel_prop_counter_ % 12 < 6)
			{
				val = kElementStone;
			}
			else
			{
				val = kElementWall;
			}

			ProtectCamp(val);
		}
		else if (shovel_prop_counter_ > 35400)
		{
			get_shovel_prop_ = false;
			shovel_prop_counter_ = 0;
			ProtectCamp(kElementWall);
		}
	}
	return false;
}

/*GameControl 内循环调用*/
bool PlayerBase::IsGetBombProp()
{
	bool temp = get_bomb_prop_;
	get_bomb_prop_ = false;
	return temp;
}

//
void PlayerBase::ShowProp(const HDC &center_hdc)
{
	prop_.ShowProp(center_hdc);
}

//
bool PlayerBase::ShowScorePanel(const HDC &image_hdc)
{
	return score_panel_->Show(image_hdc);// 整张画布缩放显示 image 到主窗口
}

//
void PlayerBase::SetPause()
{
	is_pause_ = true;
	pause_counter_ = 0;
}

//
void PlayerBase::SetShowProp()
{
	int n = 0;
	int m = 0;
	for (int i = 0; i < 50; i++)
	{
		n = rand() % 25;
		m = rand() % 25;
		if (CheckBox_8(n, m))
		{
			break;
		}
	}
	MciSound::PlaySounds(kSoundPropOut);
	prop_.StartShowProp(n, m);
}
void PlayerBase::AddKillEnemyNum(byte enemy_level)
{
	kill_enemy_number_[enemy_level]++;
}
void PlayerBase::ResetScorePanelData(const int &player_num, const int &stage)
{
	score_panel_->ResetData(kill_enemy_number_, player_num, stage);
}
bool PlayerBase::IsLifeEnd()
{
	return is_died_;// player_life_ <= 0;
}
void PlayerBase::CheckShowGameOver(const HDC &center_hdc)
{
	player_gameover_.Show(center_hdc);
}
void PlayerBase::SetShowGameover()
{
	player_gameover_.SetShow();
}
/////////////////////////////////////////////////////////////


void PlayerBase::SignBullet(int left_x, int top_y, byte dir, int val)
{
	// 转换弹头坐标
	int hx = left_x + Bullet::devto_head[dir][0];
	int hy = top_y + Bullet::devto_head[dir][1];

	// 转换成 4*4 格子下标索引
	int b4i = hy / kSmallBoxSize;
	int b4j = hx / kSmallBoxSize;
	if (b4i > 51 || b4j > 51 || b4i < 0 || b4j < 0)
	{
		return;
	}

	box_mark_->bullet_4[b4i][b4j] = val;
}

// private function
void PlayerBase::Reborn()
{
	tank_x_ = (4 + 4 * player_id_) * 16 + kBoxSize;				// 坦克首次出现时候的中心坐标
	tank_y_ = 12 * 16 + kBoxSize;
	SignBox_4(tank_x_, tank_y_, kSignPlayer + player_id_);		// 坦克出现, 将四角星标记改为坦克标记

	player_tank_level_ = 0;										// 坦克级别 [0-3]
	tank_timer_.SetDrtTime(move_speed_dev_[player_tank_level_]);
	bullet_timer_.SetDrtTime(bullet_speed_dev_[player_tank_level_]);

	tank_dir_ = kDirUp;											// 坦克方向

	bullet1_counter_ = 6;										// 子弹 1 运动 N 个循环后才可以发射子弹 2 
	is_moving_ = false;
	ring_.SetShowable(3222);									// 显示保护环
}

void PlayerBase::DispatchProp(int prop_kind)
{
	MciSound::PlaySounds(kSoundGetProp);
	prop_.StopShowProp(true);

	switch (prop_kind)
	{
	case kPropAdd:			// 加机
		MciSound::PlaySounds(kSoundAddLife);
		player_life_ = player_life_ + 1 > 5 ? 5 : player_life_ + 1;
		break;
	case kPropStar:			// 五角星
		player_tank_level_ = player_tank_level_ + 1 > 3 ? 3 : player_tank_level_ + 1;
		tank_timer_.SetDrtTime(move_speed_dev_[player_tank_level_]);
		bullet_timer_.SetDrtTime(bullet_speed_dev_[player_tank_level_]);
		break;
	case kPropTime:			// 时钟
		get_time_prop_ = true;
		break;
	case  kPropBomb:		// 地雷
		get_bomb_prop_ = true;
		break;
	case kPropShovel:		// 铲子
		get_shovel_prop_ = true;
		break;
	case  kPropCap:			// 帽子
		ring_.SetShowable(12000);
		break;
	default:
		break;
	}
}

/*
* 变向的同时调整坦克所在格子. 必须保证坦克中心在格子线上
 * 玩家移动计时器 tank_timer_ 未到时	不能移动
 * 玩家生命值用光后					不能移动
 * 玩家坦克正在爆炸					不能移动
 * 玩家被另一个玩家击中后				不能移动

 * 移动前清除坦克 tank_x_,tank_y_ 所在的 box_4 四个格子 = 空，表示该处没有东西存在
 * 如果是变向，那么调整 tank_x_,tank_y_ 到正确的格子位置上
 * 检测是否可以移动
 * 如果可以移动，计算新的 tank_x_, tank_y_ 坐标
 * 移动后标记 box_4 四个格子 = kSignPlayer + player_id_；表示该玩家坦克处于新位置

 * 新的坐标 tank_x_, tank_y_ 已经更新完毕，函数返回，待循环体根据这个坐标重新绘制坦克，从而实现移动效果。
 */
void PlayerBase::Move(int new_dir)
{
	if (!tank_timer_.IsTimeOut() || is_died_ || blast_.IsBlasting())
	{
		return;
	}

	// 如果玩家被另一个玩家击中暂停
	if (is_pause_)
	{
		return;
	}

	SignBox_4(tank_x_, tank_y_, kElementEmpty);

	if (tank_dir_ != new_dir)
	{
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

		// 更改方向, 必须先调正位置后才能设置方向!!
		tank_dir_ = new_dir;
	}
	else								// 移动
	{
		if (CheckMoveable())
		{
			tank_x_ += dev_xy_[tank_dir_][0];// *mSpeed[player_tank_level_];
			tank_y_ += dev_xy_[tank_dir_][1];// *mSpeed[player_tank_level_];
		}
	}
	SignBox_4(tank_x_, tank_y_, kSignPlayer + player_id_);
}

/* 判断当前方向可否移动
	box_8[i][j]
	i = y / kBoxSize		// y 坐标对应的是 i 值
	j = x / kBoxSize		// x 坐标对应的是 j 值
	---------
	| 1 | 2	|
	----a----
	| 3 | 4	|
	---------
* 如果 (x,y) 在 a 点, 转换后的 i,j 属于格子 4
* 如果 x 值在 a 点左边, 则转换后的 j 属于 1或3; 右边则属于 2或4
* 如果 y 值在 a 点以上, 则转换后的 i 属于 1或2; 以下则属于 3或4
** 如果 tempx,tempy 跨越了格子又遇到障碍, 那么就将 tank_x_ 或 tank_y_ 调整到格子线上,
*/
bool PlayerBase::CheckMoveable()
{
	// 坦克中心坐标
	int tempx = tank_x_ + dev_xy_[tank_dir_][0];// *mSpeed[player_tank_level_];
	int tempy = tank_y_ + dev_xy_[tank_dir_][1];// *mSpeed[player_tank_level_];

	// 游戏是运行在一个 208*208 的画布上的，所以实际游戏区域大小是 208*208
	// 如果新坐标 tempx < 8 即坦克中心坐标 < 8 ，说明此时已经越界（因为是中心点），就是说坦克已经移动到边界了
	if (tempx < kBoxSize || tempy < kBoxSize || tempy > kCenterWidth - kBoxSize || tempx > kCenterHeight - kBoxSize)
	{
		// 如果遇到障碍物,将坦克坐标调整到格子线上. 不然坦克和障碍物会有几个像素点间隔
		switch (tank_dir_)
		{
		case kDirLeft:	tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;	break;	// tank_x_ 与 tempx 之间跨越了格子, 将坦克放到tank_x_所在的格子线上
		case kDirUp:	tank_y_ = (tank_y_ / kBoxSize) * kBoxSize;	break;
		case kDirRight: tank_x_ = (tempx / kBoxSize) * kBoxSize;	break;
		case kDirDown:	tank_y_ = (tempy / kBoxSize) * kBoxSize;	break;
		default:													break;
		}
		return false;
	}
	// 转换像素点所在的 xy[26][26] 下标
	int index_i = tempy / kBoxSize;
	int index_j = tempx / kBoxSize;

	// 四个方向坦克中心点相对于要检测的两个 8*8 格子的下标偏移量
	int dev[4][2][2] = { {{-1,-1},{0,-1}},  {{-1,-1},{-1,0}},  {{-1,1},{0,1}}, { {1,-1},{1,0}} };

	// 8*8 障碍物格子检测
	int temp1 = box_mark_->box_8[index_i + dev[tank_dir_][0][0]][index_j + dev[tank_dir_][0][1]];
	int temp2 = box_mark_->box_8[index_i + dev[tank_dir_][1][0]][index_j + dev[tank_dir_][1][1]];

	// prop_8道具格子检测
	int curi = tank_y_ / kBoxSize;	// 当前坦克所在的坐标, 不是下一步的坐标, 用于判断道具
	int curj = tank_x_ / kBoxSize;
	int prop[4] = { box_mark_->prop_8[curi][curj],	 box_mark_->prop_8[curi - 1][curj],
					box_mark_->prop_8[curi][curj - 1], box_mark_->prop_8[curi - 1][curj - 1] };
	for (int i = 0; i < 4; i++)
	{
		if (prop[i] >= kSignProp && prop[i] < kSignProp + 6)
		{
			DispatchProp(prop[i] - kSignProp);
			break;
		}
	}

	// 检测坦克 4*4 格子
	// 四个方向需要检测的两个 4*4 的格子与坦克中心所在 4*4 格子的下标偏移量
	int  dev_4[4][4][2] = { {{-2,-2},{1,-2},{-1,-2},{0,-2}}, {{-2,-2},{-2,1},{-2,-1},{-2,0}},
							{{-2,2},{1,2},{-1,2},{0,2}}, {{2,-2},{2,1},{2,-1},{2,0}} };
	// 转换成 [52][52] 下标
	int index_4i = tempy / kSmallBoxSize;
	int index_4j = tempx / kSmallBoxSize;

	// -1, 0, 1, 2 都可以移动
	bool tank1 = box_mark_->box_4[index_4i + dev_4[tank_dir_][0][0]][index_4j + dev_4[tank_dir_][0][1]] <= kElementIce;
	bool tank2 = box_mark_->box_4[index_4i + dev_4[tank_dir_][1][0]][index_4j + dev_4[tank_dir_][1][1]] <= kElementIce;
	bool tank3 = box_mark_->box_4[index_4i + dev_4[tank_dir_][2][0]][index_4j + dev_4[tank_dir_][2][1]] <= kElementIce;
	bool tank4 = box_mark_->box_4[index_4i + dev_4[tank_dir_][3][0]][index_4j + dev_4[tank_dir_][3][1]] <= kElementIce;

	// 遇到障碍物调节坐标
	if (temp1 > 2 || temp2 > 2)
	{
		// 如果遇到障碍物,将坦克坐标调整到格子线上. 不然坦克和障碍物会有几个像素点间隔
		switch (tank_dir_)
		{
		case kDirLeft:	tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;	break;	// tank_x_ 与 tempx 之间跨越了格子, 将坦克放到tank_x_所在的格子线上
		case kDirUp:	tank_y_ = (tank_y_ / kBoxSize) * kBoxSize;	break;
		case kDirRight: tank_x_ = (tempx / kBoxSize) * kBoxSize;		break;
		case kDirDown:	tank_y_ = (tempy / kBoxSize) * kBoxSize;		break;
		default:													break;
		}
		return false;
	}
	// 遇到玩家不用调节
	else if (!tank1 || !tank2 || !tank3 || !tank4)
	{
		return false;
	}

	//. 如果是第一次进入 kElementIce 上面
	if (!on_ice_ && (temp1 == kElementIce || temp2 == kElementIce))
	{
		on_ice_ = true;
	}
	if (on_ice_ && temp1 != kElementIce && temp2 != kElementIce)
	{
		on_ice_ = false;
	}
	return true;
}

// 发射子弹
bool PlayerBase::ShootBullet(int bullet_id)
{
	switch (bullet_id)
	{
	case 0:
		// 1号子弹发射失败, 爆炸未完成前不能发射
		if (bullet_[0].x != kShootTableX || bombs_[0].is_show_bomb == true)
		{
			return false;
		}

		// 子弹发射点坐标
		bullet_[0].x = (int)(tank_x_ + Bullet::devto_tank[tank_dir_][0]);
		bullet_[0].y = (int)(tank_y_ + Bullet::devto_tank[tank_dir_][1]);
		bullet_[0].direction = tank_dir_;
		bullet1_counter_ = 6;

		SignBullet(bullet_[0].x, bullet_[0].y, bullet_[0].direction, kSignPlayerBullet + player_id_ * 10 + bullet_id);
		return true;

	case 1:
		// 2 号子弹发射失败
		if (player_tank_level_ < 2 || bullet_[1].x != kShootTableX || bullet1_counter_ > 0 || bombs_[1].is_show_bomb == true)
		{
			return false;
		}

		// 子弹发射点坐标
		bullet_[1].x = (int)(tank_x_ + Bullet::devto_tank[tank_dir_][0]);
		bullet_[1].y = (int)(tank_y_ + Bullet::devto_tank[tank_dir_][1]);
		bullet_[1].direction = tank_dir_;

		SignBullet(bullet_[1].x, bullet_[1].y, bullet_[1].direction, kSignPlayerBullet + player_id_ * 10 + bullet_id);
		return true;

	default:
		break;
	}
	return false;
}

//
BulletShootKind PlayerBase::CheckBomb(int i)
{
	int dir = bullet_[i].direction;

	// 子弹头接触到障碍物的那个点, 左右方向点在上, 上下方向点在右
	int bombx = bullet_[i].x + Bullet::devto_head[dir][0];
	int bomby = bullet_[i].y + Bullet::devto_head[dir][1];

	bool flag = false;
	int adjust_x = 0, adjust_y = 0;		// 修正爆照图片显示的坐标

	// 不能用 bombx 代替 bullet_[i].x,否则会覆盖障碍物的检测
	if (bullet_[i].x < 0 && bullet_[i].direction == kDirLeft)
	{
		flag = true;
		adjust_x = 5;					// 将爆炸图片向右移一点
	}
	else if (bullet_[i].y < 0 && bullet_[i].direction == kDirUp)
	{
		flag = true;
		adjust_y = 5;
	}

	// 必须减去子弹的宽 4, 不然子弹越界, 后面检测导致 box_8 下标越界
	else if (bullet_[i].x >= kCenterWidth - 4 && bullet_[i].direction == kDirRight)
	{
		flag = true;
		adjust_x = -4;
	}
	else if (bullet_[i].y >= kCenterHeight - 4 && bullet_[i].direction == kDirDown)
	{
		flag = true;
		adjust_y = -4;
	}
	if (flag)
	{
		// 设定爆炸参数, 修正爆炸中心所在的格子,左右或上下偏移一个格子之类的..
		bullet_[i].x = kShootTableX;
		bombs_[i].is_show_bomb = true;
		bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
		bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
		bombs_[i].counter = 0;

		MciSound::PlaySounds(kSoundBin);

		return BulletShootKind::kOther;
	}

	int tempi, tempj;

	// 坐标所在 8*8 格子的索引
	int b8i = bomby / kBoxSize;
	int b8j = bombx / kBoxSize;

	// 将坐标转换成 4*4 格子索引
	int b4i = bomby / kSmallBoxSize;
	int b4j = bombx / kSmallBoxSize;

	// 如果击中另外一个玩家子弹
	if (box_mark_->bullet_4[b4i][b4j] == kSignPlayerBullet + (1 - player_id_) * 10 + 0 ||
		box_mark_->bullet_4[b4i][b4j] == kSignPlayerBullet + (1 - player_id_) * 10 + 1 ||
		box_mark_->bullet_4[b4i][b4j] == kSignEnemyBullet)
	{
		bullet_[i].x = kShootTableX;
		box_mark_->bullet_4[b4i][b4j] = kSignWaitUnsign;		// 先标记中间值, 等待被击中的子弹擦除该标记
		return BulletShootKind::kOther;
	}
	else if (box_mark_->bullet_4[b4i][b4j] == kSignWaitUnsign)
	{
		bullet_[i].x = kShootTableX;
		box_mark_->bullet_4[b4i][b4j] = kElementEmpty;
		return BulletShootKind::kOther;
	}

	switch (bullet_[i].direction)
	{
	case kDirLeft:
	case kDirRight:
	{
		// 自身格子和上一个
		int temp[2][2] = { {0, 0}, {-1, 0} };
		for (int n = 0; n < 2; n++)
		{
			// 8*8 格子, 判断是否击中敌机
			tempi = b8i + temp[n][0];
			tempj = b8j + temp[n][1];
			if (box_mark_->box_8[tempi][tempj] == kSignCamp)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].counter = 0;
				SignBox_8(13 * kBoxSize, 25 * kBoxSize, kElementEmpty);

				return BulletShootKind::kCamp;
			}

			// 左右检测子弹头所在的4*4格子和它上面相邻的那个
			// 检测 4*4 格子, 由此判断障碍物
			tempi = b4i + temp[n][0];
			tempj = b4j + temp[n][1];
			if (box_mark_->box_4[tempi][tempj] == kElementWall || box_mark_->box_4[tempi][tempj] == kElementStone)
			{
				// 设定爆炸参数, 修正爆炸中心所在的格子,左右或上下偏移一个格子之类的..
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// 指示 i bomb 爆炸
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;
				ClearWallOrStone(i, bombx, bomby);
				return BulletShootKind::kOther;
			}
			else if (box_mark_->box_4[tempi][tempj] >= kSignEnemy /*&& box_mark_->box_4[tempi][tempj] < kSignEnemy + TOTAL_ENEMY_NUMBER*/)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// 指示 i bomb 爆炸
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;

				// 标记击中了敌机的 id
				bullet_[i].killed_id = box_mark_->box_4[tempi][tempj];
				return BulletShootKind::kOther;
			}
			else if (box_mark_->box_4[tempi][tempj] == kSignPlayer && player_id_ != 0 || box_mark_->box_4[tempi][tempj] == kSignPlayer + 1 && player_id_ != 1)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// 指示 i bomb 爆炸
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;
				return (BulletShootKind)box_mark_->box_4[tempi][tempj];
			}
		}
	}
	break;

	// 上下只检测左右相邻的两个格子
	case kDirUp:
	case kDirDown:
	{
		// 自身格子和左边那一个格子
		int temp[2][2] = { { 0, 0 },{ 0, -1 } };
		for (int n = 0; n < 2; n++)
		{
			// 8*8 格子, 判断是否击中敌机
			tempi = b8i + temp[n][0];
			tempj = b8j + temp[n][1];

			if (box_mark_->box_8[tempi][tempj] == kSignCamp)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].counter = 0;
				SignBox_8(13 * kBoxSize, 25 * kBoxSize, kElementEmpty);
				return BulletShootKind::kCamp;
			}

			// 检测 4*4 是否击中障碍
			tempi = b4i + temp[n][0];
			tempj = b4j + temp[n][1];
			if (box_mark_->box_4[tempi][tempj] == kElementWall || box_mark_->box_4[tempi][tempj] == kElementStone)
			{
				// 设定爆炸参数, 修正爆炸中心所在的格子,左右或上下偏移一个格子之类的..
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// 指示 i bomb 爆炸
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;
				ClearWallOrStone(i, bombx, bomby);
				return BulletShootKind::kOther;
			}
			else if (box_mark_->box_4[tempi][tempj] >= kSignEnemy/* && box_mark_->box_4[tempi][tempj] < kSignEnemy + TOTAL_ENEMY_NUMBER*/)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// 指示 i bomb 爆炸
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;

				// 标记击中了敌机的 id
				bullet_[i].killed_id = box_mark_->box_4[tempi][tempj];
				return BulletShootKind::kOther;
			}
			else if (box_mark_->box_4[tempi][tempj] == kSignPlayer && player_id_ != 0 || box_mark_->box_4[tempi][tempj] == kSignPlayer + 1 && player_id_ != 1)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// 指示 i bomb 爆炸
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;
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

// 子弹击中障碍物爆炸调用该函数, 击中边界不可调用, 下标会越界[52][52]
void PlayerBase::ClearWallOrStone(int bulletid, int bulletx, int bullety)
{
	int boxi = bullety / kSmallBoxSize;
	int boxj = bulletx / kSmallBoxSize;
	int tempx, tempy;
	switch (bullet_[bulletid].direction)
	{
	case kDirLeft:
	case kDirRight:
	{
		bool bin_once = false;		// 多次循环中只播放一次声音

		// 在同一直线相邻的四个 4*4 格子, 顺序不能变, 后面用到下标判断
		int temp[4][2] = { { -2, 0 },{ -1, 0 },{ 0, 0 },{ 1, 0 } };
		for (int i = 0; i < 4; i++)
		{
			tempx = boxi + temp[i][0];
			tempy = boxj + temp[i][1];

			// 转到 tempx,tempy所在的 8*8 格子索引
			int n = tempx / 2;
			int m = tempy / 2;

			if (box_mark_->box_4[tempx][tempy] == kElementWall)
			{
				box_mark_->box_4[tempx][tempy] = kElementClear;

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
			else if (player_tank_level_ < 3 && box_mark_->box_4[tempx][tempy] == kElementStone && bin_once == false)
			{
				MciSound::PlaySounds(kSoundBin);
				bin_once = true;
			}
			else if (player_tank_level_ == 3 && box_mark_->box_4[tempx][tempy] == kElementStone && i % 2 == 0)	// %2==0 防止多余循环判断
			{
				box_mark_->box_8[n][m] = kElementEmpty;
				for (int a = 2 * n; a < 2 * n + 2; a++)
				{
					for (int b = 2 * m; b < 2 * m + 2; b++)
					{
						box_mark_->box_4[a][b] = kElementClear;
					}
				}
			}
		}
	}
	break;

	case kDirUp:
	case kDirDown:
	{
		bool bin_once = false;		// 多次循环中只播放一次声音

		// 相邻的四个 4*4 格子, 顺序不能变, 后面用到下标判断
		int temp[4][2] = { {0, -2}, {0, -1}, {0, 0}, {0, 1} };
		for (int i = 0; i < 4; i++)
		{
			tempx = boxi + temp[i][0];
			tempy = boxj + temp[i][1];

			// 转到 tempx,tempy所在的 8*8 格子索引
			int n = tempx / 2;
			int m = tempy / 2;

			if (box_mark_->box_4[tempx][tempy] == kElementWall)
			{
				box_mark_->box_4[tempx][tempy] = kElementClear;

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
			else if (player_tank_level_ < 3 && box_mark_->box_4[tempx][tempy] == kElementStone && bin_once == false)
			{
				MciSound::PlaySounds(kSoundBin);
				bin_once = true;
			}
			else if (player_tank_level_ == 3 && box_mark_->box_4[tempx][tempy] == kElementStone && i % 2 == 0)	// %2==0 防止多余循环判断
			{
				box_mark_->box_8[n][m] = kElementEmpty;
				for (int a = 2 * n; a < 2 * n + 2; a++)
				{
					for (int b = 2 * m; b < 2 * m + 2; b++)
					{
						box_mark_->box_4[a][b] = kElementClear;
					}
				}
			}
		}
	}
	break;

	default:
		break;
	}
}

// 参数是 16 * 16 中心点像素坐标, 与坦克中心坐标相同
void PlayerBase::SignBox_8(int x, int y, int val)
{
	// 右坦克中心索引转到左上角那个的 格子索引
	int iy = y / kBoxSize - 1;
	int jx = x / kBoxSize - 1;
	for (int i = iy; i < iy + 2; i++)
	{
		for (int j = jx; j < jx + 2; j++)
		{
			box_mark_->box_8[i][j] = val;
		}
	}
}

// 根据坦克中心坐标, 标记16个 4*4 格子
void PlayerBase::SignBox_4(int cx, int cy, int val)
{
	// 左右调整 cs,cy 到占据百分比最多的 16 个 4*4 的格子中心
	if (tank_dir_ == kDirLeft || tank_dir_ == kDirRight)
	{
		if (cx > (cx / kSmallBoxSize) * kSmallBoxSize + kSmallBoxSize / 2)	// 如果是靠近右边节点, 
		{
			cx = (cx / kSmallBoxSize + 1) * kSmallBoxSize;
		}
		else
		{
			cx = (cx / kSmallBoxSize) * kSmallBoxSize;					// 靠近格子线上的左边节点
		}
	}
	// 上下
	else
	{
		if (cy > (cy / kSmallBoxSize) * kSmallBoxSize + kSmallBoxSize / 2)	// 如果是靠近格子下边节点,
		{
			cy = (cy / kSmallBoxSize + 1) * kSmallBoxSize;
		}
		else
		{
			cy = (cy / kSmallBoxSize) * kSmallBoxSize;					// 靠近格子线上的上边节点
		}
	}

	// 右坦克中心索引转到左上角那个的 格子索引
	int iy = cy / kSmallBoxSize - 2;
	int jx = cx / kSmallBoxSize - 2;
	for (int i = iy; i < iy + 4; i++)
	{
		for (int j = jx; j < jx + 4; j++)
		{
			box_mark_->box_4[i][j] = val;
		}
	}
}

// 检测4个8*8 格子, 参数是左上角格子索引
bool PlayerBase::CheckBox_8(int iy, int jx)
{
	for (int i = iy; i < iy + 2; i++)
	{
		for (int j = jx; j < jx + 2; j++)
		{
			if (box_mark_->box_8[i][j] < kElementRiver)		// 如果有一个 8*8 坦克可以进来
			{
				return true;
			}
		}
	}
	return false;
}

/*获得铲子道具保护camp 内部使用*/
void PlayerBase::ProtectCamp(int val)
{
	for (int i = 23; i < 26; i++)
	{
		for (int j = 11; j < 15; j++)
		{
			if (i < 24 || j < 12 || j > 13)
			{
				box_mark_->box_8[i][j] = val;			// 鸟巢周围是 kElementWall
				for (int m = 2 * i; m < 2 * i + 2; m++)
				{
					for (int n = 2 * j; n < 2 * j + 2; n++)
					{
						box_mark_->box_4[m][n] = val;
					}
				}
			}
		}
	}
}
}// namespace battlecity
