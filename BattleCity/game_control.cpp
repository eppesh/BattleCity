#include "game_control.h"
#include "stdafx.h"
#include "mci_sound.h"

namespace battlecity
{
int GameControl::current_stage_ = 1;	// [1-35]
GameControl::GameControl(HDC des_hdc, HDC image_hdc)
{
	des_hdc_ = des_hdc;
	image_hdc_ = image_hdc;
	center_image_.Resize(kCenterWidth, kCenterHeight);
	center_hdc_ = GetImageHDC(&center_image_);
	box_mark_ = new BoxMark();

	loadimage(&black_background_image_, _T("./res/big/bg_black.gif"));		// 黑色背景
	loadimage(&gray_background_image_, _T("./res/big/bg_gray.gif"));		// 灰色背景
	loadimage(&stone_image_, _T("./res/big/stone.gif"));					// 12*12的石头
	loadimage(&forest_image_, _T("./res/big/forest.gif"));					// 树林
	loadimage(&ice_image_, _T("./res/big/ice.gif"));						// 冰块
	loadimage(&river_image_[0], _T("./res/big/river-0.gif"));				// 河流
	loadimage(&river_image_[1], _T("./res/big/river-1.gif"));		
	loadimage(&wall_image_, _T("./res/big/wall.gif"));						// 泥墙
	loadimage(&camp_image_[0], _T("./res/big/camp0.gif"));					// 大本营
	loadimage(&camp_image_[1], _T("./res/big/camp1.gif"));		 
	loadimage(&enemy_tank_ico_image_, _T("./res/big/enemytank-ico.gif"));	// 敌机图标
	loadimage(&flag_image_, _T("./res/big/flag.gif"));						// 旗子
	loadimage(&current_stage_image_, _T("./res/big/stage.gif"));
	loadimage(&black_number_image_, _T("./res/big/black-number.gif"));		// 0123456789 黑色数字
	loadimage(&gameover_image_, _T("./res/big/gameover.gif"));

	// 自定义绘制地图
	loadimage(&create_map_tank_image_, _T("./res/big/0Player/m0-1-2.gif"));

	loadimage(&msgoas_image_, _T("./res/big/big-gameover.gif"));

	Init();
}

GameControl::~GameControl()
{
	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		delete *itor;
	}

	delete box_mark_;
	printf("~GameControl::GameControl()\n");
}

void GameControl::Init()
{
	remain_enemy_tank_number_ = 20;	 // 剩余未出现的敌机数量
	cur_moving_tank_number_ = 0;
	killed_enemy_number_ = 0;
	is_camp_die_ = false;													// 标志大本营是否被击中

	is_enemy_pause_ = false;			// 敌机暂停与否
	enemy_pause_timer_.SetDrtTime(10000);	// 敌机暂停多久

	main_timer_.SetDrtTime(14);
	camp_timer_.SetDrtTime(33);

	cut_stage_counter_ = 0;		// STAGE 字样计数

	// GameOver 图片
	gameover_counter_ = 0;
	gameover_x_ = -100;
	gameover_y_ = -100;
	gameover_flag_ = false;
	gameover_timer_.SetDrtTime(30);

	// 绘制地图坦克游标的坐标
	cmt_image_x_ = kBoxSize;
	cmt_image_y_ = kBoxSize;

	// 每次进入地图制作之前都检测之前是否有制作地图
	has_custom_map_ = false;

	// 关卡结束显示分数面板
	show_score_panel_ = false;

	// 胜利
	is_win_ = false;
	win_counter_ = 0;

	msgoas_counter_ = 0;
	msgoas_y_ = kCenterHeight;
	show_gameover_after_score_panel_ = false;
}

// 存储玩家进链表
void GameControl::AddPlayer(int player_num)
{
	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		delete *itor;
	}

	// 清空原来数据
	player_list_.clear();

	for (int i = 0; i < player_num; i++)
	{
		player_list_.push_back(new PlayerBase(i, box_mark_));
	}

	PlayerBase::player_list_ = &player_list_;
}


// 读取data数据绘制地图,显示敌机数量\玩家生命\关卡\等信息
void GameControl::LoadMap()
{
	// 读取地图文件数据
	FILE *fp = NULL;
	if (0 != fopen_s(&fp, "./res/data/map.dat", "rb"))
	{
		throw _T("读取地图数据文件异常.");
	}
	fseek(fp, sizeof(Map) * (current_stage_ - 1), SEEK_SET);
	fread(&map_, sizeof(Map), 1, fp);
	fclose(fp);

	InitSignBox();
}

// 玩家自己创建地图
bool GameControl::CreateMap(bool *isCreate)
{
	int i, j, x = 0, y = 0;
	int tempx, tempy;
	bool flag = true;
	int keys[4] = { VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN };		// 下标必须与 DIR_LEFT 等对应
	int dev[4][2] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} };	// 游标坦克移动分量
	int twinkle_counter = 0;	// 坦克游标闪烁计数
	int lastx = cmt_image_x_, lasty = cmt_image_y_;			// 记录坦克上次的坐标, 如果坦克移动不会变换 sign_order 图形
	bool M_down = false;		// 检测 M 是否按下, 一直按下 M 不会切换地图, 只按下的那一次切换

	// 14 中情况
	int sign_order[14][4] = { {kElementIce, kElementIce, kElementIce, kElementIce},		// 四个格子都是冰, 依次左上右上左下 ..
		{ kElementForest,	kElementForest,		kElementForest,		kElementForest},
		{ kElementRiver,	kElementRiver,		kElementRiver,		kElementRiver },
		{ kElementStone,	kElementStone ,		kElementStone ,		kElementStone },
		{ kElementStone ,	kElementStone ,		kElementEmpty,		kElementEmpty },
		{ kElementStone ,	kElementEmpty,		kElementStone ,		kElementEmpty },
		{ kElementEmpty,	kElementEmpty,		kElementStone ,		kElementStone },
		{ kElementEmpty,	kElementStone ,		kElementEmpty,		kElementStone },
		{ kElementWall,		kElementWall ,		kElementWall ,		kElementWall  },
		{ kElementWall ,	kElementWall ,		kElementEmpty,		kElementEmpty },
		{ kElementWall ,	kElementEmpty,		kElementWall,		kElementEmpty },
		{ kElementEmpty,	kElementEmpty,		kElementWall ,		kElementWall  },
		{ kElementEmpty,	kElementWall ,		kElementEmpty,		kElementWall  },
		{ kElementEmpty,	kElementEmpty ,		kElementEmpty ,		kElementEmpty } };

	int cur_index = 13;		// 对应上面数组

	// 清除或保留上次绘制的地图
	if (!has_custom_map_)
	{
		ClearSignBox();
	}

	// 按键速度
	TimeClock click;
	click.SetDrtTime(80);

	// 灰色背景
	StretchBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, GetImageHDC(&gray_background_image_), 0, 0, 66, 66, SRCCOPY);
	int counter = 0;
	while (flag)
	{
		counter++;
		Sleep(34);

		if (GetAsyncKeyState(VK_LEFT) & 0x8000 && counter > 3)
		{
			counter = 0;

			lastx = cmt_image_x_;
			lasty = cmt_image_y_;
			tempx = cmt_image_x_ + dev[0][0] * kBoxSize * 2;
			tempy = cmt_image_y_ + dev[0][1] * kBoxSize * 2;
			if (tempx >= kBoxSize && tempx <= kBoxSize * 25 && tempy >= kBoxSize && tempy <= kBoxSize * 25)
			{
				cmt_image_x_ += dev[0][0] * kBoxSize * 2;
				cmt_image_y_ += dev[0][1] * kBoxSize * 2;
			}
		}
		else if (GetAsyncKeyState(VK_UP) & 0x8000 && counter > 3)
		{
			counter = 0;

			lastx = cmt_image_x_;
			lasty = cmt_image_y_;
			tempx = cmt_image_x_ + dev[1][0] * kBoxSize * 2;
			tempy = cmt_image_y_ + dev[1][1] * kBoxSize * 2;
			if (tempx >= kBoxSize && tempx <= kBoxSize * 25 && tempy >= kBoxSize && tempy <= kBoxSize * 25)
			{
				cmt_image_x_ += dev[1][0] * kBoxSize * 2;
				cmt_image_y_ += dev[1][1] * kBoxSize * 2;
			}
		}
		else if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && counter > 3)
		{
			counter = 0;

			lastx = cmt_image_x_;
			lasty = cmt_image_y_;
			tempx = cmt_image_x_ + dev[2][0] * kBoxSize * 2;
			tempy = cmt_image_y_ + dev[2][1] * kBoxSize * 2;
			if (tempx >= kBoxSize && tempx <= kBoxSize * 25 && tempy >= kBoxSize && tempy <= kBoxSize * 25)
			{
				cmt_image_x_ += dev[2][0] * kBoxSize * 2;
				cmt_image_y_ += dev[2][1] * kBoxSize * 2;
			}
		}
		else if (GetAsyncKeyState(VK_DOWN) & 0x8000 && counter > 3)
		{
			counter = 0;

			lastx = cmt_image_x_;
			lasty = cmt_image_y_;
			tempx = cmt_image_x_ + dev[3][0] * kBoxSize * 2;
			tempy = cmt_image_y_ + dev[3][1] * kBoxSize * 2;
			if (tempx >= kBoxSize && tempx <= kBoxSize * 25 && tempy >= kBoxSize && tempy <= kBoxSize * 25)
			{
				cmt_image_x_ += dev[3][0] * kBoxSize * 2;
				cmt_image_y_ += dev[3][1] * kBoxSize * 2;
			}
		}
		else if (GetAsyncKeyState('J') & 0x8000 && counter > 3)
		{
			counter = 0;

			if (cmt_image_x_ == lastx && cmt_image_y_ == lasty)
			{
				cur_index = cur_index + 1 > 13 ? 0 : cur_index + 1;
			}
			else
			{
				lastx = cmt_image_x_;
				lasty = cmt_image_y_;
			}

			// 更改 16*16 的地图
			i = cmt_image_y_ / kBoxSize - 1;
			j = cmt_image_x_ / kBoxSize - 1;
			box_mark_->box_8[i][j] = sign_order[cur_index][0];
			box_mark_->box_8[i][j + 1] = sign_order[cur_index][1];
			box_mark_->box_8[i + 1][j] = sign_order[cur_index][2];
			box_mark_->box_8[i + 1][j + 1] = sign_order[cur_index][3];
		}
		else if (GetAsyncKeyState('K') & 0x8000 && counter > 3)
		{
			counter = 0;

			if (cmt_image_x_ == lastx && cmt_image_y_ == lasty)
			{
				cur_index = cur_index - 1 < 0 ? 13 : cur_index - 1;
			}
			else
			{
				lastx = cmt_image_x_;
				lasty = cmt_image_y_;
			}

			// 更改 16*16 的地图
			i = cmt_image_y_ / kBoxSize - 1;
			j = cmt_image_x_ / kBoxSize - 1;
			box_mark_->box_8[i][j] = sign_order[cur_index][0];
			box_mark_->box_8[i][j + 1] = sign_order[cur_index][1];
			box_mark_->box_8[i + 1][j] = sign_order[cur_index][2];
			box_mark_->box_8[i + 1][j + 1] = sign_order[cur_index][3];
		}
		else if (GetAsyncKeyState(VK_RETURN) & 0x8000 && counter > 3)
		{
			counter = 0;

			// 标记 8*8 格子内部的 4*4 格子
			for (i = 0; i < 26; i++)
			{
				for (j = 0; j < 26; j++)
				{
					// 根据 8*8 标记 4*4 格子, 大本营值标记 box_8
					if (box_mark_->box_8[i][j] != kElementEmpty && box_mark_->box_8[i][j] != kSignCamp)
					{
						SignBox_4(i, j, box_mark_->box_8[i][j]);
					}

					// 清空敌机出现的三个位置
					if (i <= 1 && j <= 1 || j >= 12 && j <= 13 && i <= 1 || j >= 24 && i <= 1)
					{
						box_mark_->box_8[i][j] = kElementEmpty;
						SignBox_4(i, j, kElementEmpty);
					}

					// 鸟巢位置不能绘制
					if (i >= 24 && j >= 12 && j <= 13)
					{
						box_mark_->box_8[i][j] = kSignCamp;
					}
				}
			}

			break;
		}

		if (GetAsyncKeyState(27) & 0x8000)
		{
			break;
		}

		// 黑色背景
		StretchBlt(center_hdc_, 0, 0, kCenterWidth, kCenterHeight, GetImageHDC(&black_background_image_), 0, 0, kCenterWidth, kCenterHeight, SRCCOPY);

		for (int i = 0; i < 26; i++)
		{
			for (int j = 0; j < 26; j++)
			{
				x = j * kBoxSize;// +kCenterX;
				y = i * kBoxSize;// +kCenterY;
				switch (box_mark_->box_8[i][j])
				{
				case kElementWall:
					BitBlt(center_hdc_, x, y, kBoxSize, kBoxSize, GetImageHDC(&wall_image_), 0, 0, SRCCOPY);
					break;
				case kElementForest:
					BitBlt(center_hdc_, x, y, kBoxSize, kBoxSize, GetImageHDC(&forest_image_), 0, 0, SRCCOPY);
					break;
				case kElementIce:
					BitBlt(center_hdc_, x, y, kBoxSize, kBoxSize, GetImageHDC(&ice_image_), 0, 0, SRCCOPY);
					break;
				case kElementRiver:
					BitBlt(center_hdc_, x, y, kBoxSize, kBoxSize, GetImageHDC(&river_image_[0]), 0, 0, SRCCOPY);
					break;
				case kElementStone:
					BitBlt(center_hdc_, x, y, kBoxSize, kBoxSize, GetImageHDC(&stone_image_), 0, 0, SRCCOPY);
					break;
				default:
					break;
				}
			}
		}

		// 大本营
		TransparentBlt(center_hdc_, kBoxSize * 12, kBoxSize * 24, kBoxSize * 2, kBoxSize * 2,
			GetImageHDC(&camp_image_[0]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);

		// 坦克游标
		if (twinkle_counter++ / 28 % 2 == 0)
		{
			TransparentBlt(center_hdc_, cmt_image_x_ - kBoxSize, cmt_image_y_ - kBoxSize, kBoxSize * 2, kBoxSize * 2,
				GetImageHDC(&create_map_tank_image_), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
		}
		// 将中心画布印到主画布 mImage_hdc 上
		BitBlt(image_hdc_, kCenterX, kCenterY, kCenterWidth, kCenterHeight, center_hdc_, 0, 0, SRCCOPY);
		// 整张画布缩放显示 image 到主窗口
		StretchBlt(des_hdc_, 0, 0, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
		FlushBatchDraw();
	}
	*isCreate = true;
	has_custom_map_ = true;
	return true;
}

void GameControl::GameLoop()
{
	MciSound::PlaySounds(kSoundStart);
	CutStage();
	ShowStage();
	GameResult result = GameResult::kVictory;

	while (result != GameResult::kFail)
	{
		result = StartGame();
		Sleep(1);
	}
}

// 有序循环体; 画面刷新：玩家、子弹、敌机 所有的更新都在此; 按键检测
GameResult GameControl::StartGame()
{
	// 主绘图操作时间
	if (main_timer_.IsTimeOut())
	{
		// 胜利或者失败 显示分数面板
		if (show_score_panel_)
		{
			BitBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, GetImageHDC(&ScorePanel::background_), 0, 0, SRCCOPY);
			for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
			{
				// 如果分数面板显示完
				if (!(*itor)->ShowScorePanel(image_hdc_))
				{
					show_score_panel_ = false;

					// 胜利或失败都释放敌机资源
					for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
					{
						delete *EnemyItor;
					}
					enemy_list_.clear();

					if (is_win_)
					{
						Init();
						for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
						{
							if (!(*itor)->IsLifeEnd())
							{
								(*itor)->Init();
							}
						}

						// 静态数据会保留,需要手动重置
						EnemyBase::SetPause(false);

						current_stage_++;
						LoadMap();
						MciSound::PlaySounds(kSoundStart);
						CutStage();
						ShowStage();
					}
					else
					{
						current_stage_ = 1;
						show_gameover_after_score_panel_ = true;
						MciSound::PlaySounds(kSoundFail);
					}
					break;
				}
			}

			// 整张画布缩放显示 image 到主窗口
			StretchBlt(des_hdc_, 0, 0, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
			FlushBatchDraw();
			return GameResult::kVictory;
		}

		// 上升的 GAMEOVER 字样
		if (show_gameover_after_score_panel_)
		{
			StretchBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, GetImageHDC(&black_background_image_),
				0, 0, kCenterWidth, kCenterHeight, SRCCOPY);
			BitBlt(image_hdc_, 66, msgoas_y_, 124, 80, GetImageHDC(&msgoas_image_), 0, 0, SRCCOPY);

			// 整张画布缩放显示 image 到主窗口
			StretchBlt(des_hdc_, 0, 0, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
			FlushBatchDraw();

			msgoas_y_ = msgoas_y_ - 2 > 66 ? msgoas_y_ - 2 : 66;
			if (msgoas_y_ == 66)
			{
				msgoas_counter_++;
			}

			if (msgoas_counter_ > 122)
			{
				show_gameover_after_score_panel_ = false;
				msgoas_counter_ = 0;
				return GameResult::kFail;
			}
			return GameResult::kVictory;
		}

		AddEnemy();

		// 更新右边面板的数据, 待判断, 因为不需要经常更新 mImage_hdc
		RefreshRightPanel();

		// 更新中心游戏区域: center_hdc
		RefreshCenterPanel();

		// 将中心画布印到主画布 mImage_hdc 上
		BitBlt(image_hdc_, kCenterX, kCenterY, kCenterWidth, kCenterHeight, center_hdc_, 0, 0, SRCCOPY);
		// 整张画布缩放显示 image 到主窗口
		StretchBlt(des_hdc_, 0, 0, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
		FlushBatchDraw();
	}

	// 数据变化, 不能涉及绘图操作
	RefreshData();

	return GameResult::kVictory;
}

// 私有函数,本类使用

void GameControl::CutStage()
{
	// 灰色背景
	StretchBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight,
		GetImageHDC(&gray_background_image_), 0, 0, 66, 66, SRCCOPY);

	while (cut_stage_counter_ < 110)
	{
		Sleep(6);
		cut_stage_counter_ += 3;
		StretchBlt(image_hdc_, 0, 0, kCanvasWidth, cut_stage_counter_, GetImageHDC(&black_background_image_), 0, 0, kCenterWidth, kCenterHeight, SRCCOPY);
		StretchBlt(image_hdc_, 0, kCanvasHeight - cut_stage_counter_, kCanvasWidth, kCanvasHeight,
			GetImageHDC(&black_background_image_), 0, 0, kCenterWidth, kCenterHeight, SRCCOPY);

		StretchBlt(des_hdc_, 0, 0, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
		FlushBatchDraw();
	}
}

void GameControl::ShowStage()
{
	// 灰色背景
	StretchBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight,
		GetImageHDC(&gray_background_image_), 0, 0, 66, 66, SRCCOPY);


	TransparentBlt(image_hdc_, 97, 103, 39, 7, GetImageHDC(&current_stage_image_), 0, 0, 39, 7, 0xffffff);

	// [1-9] 关卡，单个数字
	if (current_stage_ < 10)
	{
		TransparentBlt(image_hdc_, 157, 103, kBlackNumberSize, kBlackNumberSize,
			GetImageHDC(&black_number_image_), kBlackNumberSize * current_stage_, 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);
	}
	else	// 10,11,12 .. 双位数关卡
	{
		TransparentBlt(image_hdc_, 157, 103, kBlackNumberSize, kBlackNumberSize,
			GetImageHDC(&black_number_image_), kBlackNumberSize * (current_stage_ / 10), 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);

		TransparentBlt(image_hdc_, 157, 103, kBlackNumberSize, kBlackNumberSize,
			GetImageHDC(&black_number_image_), kBlackNumberSize * (current_stage_ % 10), 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);
	}
	StretchBlt(des_hdc_, 0, 0, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
	FlushBatchDraw();

	Sleep(1300);

	MciSound::PlaySounds(kSoundBGM);
}

//
void GameControl::ClearSignBox()
{
	// 初始化标记各种格子
	int x = 0, y = 0;
	for (int i = 0; i < 26; i++)
	{
		for (int j = 0; j < 26; j++)
		{
			box_mark_->prop_8[i][j] = kElementEmpty;
			box_mark_->box_8[i][j] = kElementEmpty;	// 26*26
			SignBox_4(i, j, kElementEmpty);		// 标记 26*26 和 52*52 格子
		}
	}

	// 标记大本营
	for (int i = 23; i < 26; i++)
	{
		for (int j = 11; j < 15; j++)
		{
			if (i >= 24 && j >= 12 && j <= 13)
			{
				box_mark_->box_8[i][j] = kSignCamp;
			}
			else
			{
				box_mark_->box_8[i][j] = kElementWall;			// 鸟巢周围是 kElementWall
			}
		}
	}
}

//
void GameControl::InitSignBox()
{
	// 初始化标记各种格子
	int x = 0, y = 0;
	for (int i = 0; i < 26; i++)
	{
		for (int j = 0; j < 26; j++)
		{
			box_mark_->prop_8[i][j] = kElementEmpty;
			box_mark_->box_8[i][j] = map_.buf[i][j] - '0';	// 26*26
			SignBox_4(i, j, map_.buf[i][j] - '0');		// 标记 26*26 和 52*52 格子
		}
	}

	// 标记大本营
	for (int i = 23; i < 26; i++)
	{
		for (int j = 11; j < 15; j++)
		{
			if (i >= 24 && j >= 12 && j <= 13)
			{
				box_mark_->box_8[i][j] = kSignCamp;
			}
			else
			{
				box_mark_->box_8[i][j] = kElementWall;			// 鸟巢周围是 kElementWall
			}
		}
	}
}

// 待修改, 添加的敌机种类需要修改
void GameControl::AddEnemy()
{
	int size = enemy_list_.size();

	if (cur_moving_tank_number_ >= 6 || kTotalEnemyNumber - size <= 0)
	{
		return;
	}
	cur_moving_tank_number_++;

	int level;
	TankKind kind;
	if (size < 8)
	{
		level = 0;
	}
	else if (size < 13)
	{
		level = 1;
	}
	else if (size < 17)
	{
		level = 2;
	}
	else
	{
		level = 3;
	}

	// 每个5架除一架道具坦克
	if (size % 5 == 4)
	{
		kind = TankKind::kProp;
	}
	else
	{
		kind = TankKind::kCommon;
	}

	switch (level)
	{
	case 0:
	case 1:
	case 2:
		switch (kind)
		{
		case 0: 
			enemy_list_.push_back((new PropTank(level, box_mark_))); 
			break;
		case 1: 
			enemy_list_.push_back((new CommonTank(level, box_mark_))); 
			break;
		default: break;
		}
		break;

	case 3:
		enemy_list_.push_back(new BigestTank((TankKind)kind, box_mark_));
		break;
	default:
		break;
	}
}

// 提供8*8 的左上角索引, 标记里面4个 4*4 的格子
void GameControl::SignBox_4(int i, int j, int sign_val)
{
	int temp_i[4] = { 2 * i, 2 * i + 1, 2 * i, 2 * i + 1 };
	int temp_j[4] = { 2 * j, 2 * j, 2 * j + 1, 2 * j + 1 };

	for (int i = 0; i < 4; i++)
		box_mark_->box_4[temp_i[i]][temp_j[i]] = sign_val;
}

//  数据更新, 不涉及绘图操作!!更新所有东西的坐标，下一次循环体中将在新左边绘图，实现运动
bool GameControl::RefreshData()
{
	if (GetAsyncKeyState(27) & 0x8000)
	{
		return false;
	}

	// 检测玩家是否获得 '时钟' 静止道具
	if (PlayerBase::IsGetTimeProp())
	{
		is_enemy_pause_ = true;
		enemy_pause_timer_.Init();		// 重置 t1 = t2
		EnemyBase::SetPause(true);
	}

	// 循环铲子道具逻辑
	PlayerBase::IsGetShvelProp();

	// 玩家获得地雷道具
	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		if ((*itor)->IsGetBombProp())
		{
			for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
			{
				if ((*EnemyItor)->BeKilled(true))
				{
					killed_enemy_number_++;
					cur_moving_tank_number_--;

					// 玩家记录消灭的敌机数量
					(*itor)->AddKillEnemyNum((*EnemyItor)->GetLevel());

					if (killed_enemy_number_ == 20)
					{
						win_counter_ = 0;
						is_win_ = true;
					}
				}
			}
		}
	}

	// 玩家, 不能包含绘图操作! 内含计时器
	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		// 鸟巢被消灭玩家停止移动
		if (!gameover_flag_)
		{
			(*itor)->PlayerControl();
		}

		// 玩家子弹击中结果
		BulletShootKind kind = (*itor)->BulletMoving(center_hdc_);
		switch (kind)
		{
		case BulletShootKind::kCamp:
			gameover_x_ = kCenterWidth / 2 - kGameoverWidth / 2;
			gameover_y_ = kCenterHeight;
			gameover_flag_ = true;

			// 大本营爆炸
			is_camp_die_ = true;
			blast_.SetBlasting(11, 23);

			MciSound::PlaySounds(kSoundCampBomb);
			MciSound::PauseBGM(true);
			MciSound::PlayMovingSound(false);
			break;

			// 遍历被击中的玩家 然后暂停它
		case BulletShootKind::kPlayer1:
			for (std::list<PlayerBase *>::iterator i = player_list_.begin(); i != player_list_.end(); i++)
			{
				if ((*i)->GetID() == 0)
				{
					(*i)->SetPause();
				}
			}
			break;

			// 遍历被击中的玩家 然后暂停它
		case BulletShootKind::kPlayer2:
			for (std::list<PlayerBase *>::iterator i = player_list_.begin(); i != player_list_.end(); i++)
			{
				if ((*i)->GetID() == 1)
				{
					(*i)->SetPause();
				}
			}
			break;
		default:
			break;
		}
	}

	// 敌机, 此处不能包含计绘图操作, 内含计时器, 不然那会导致计时器与主计时器不一致,导致失帧
	for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
	{
		(*EnemyItor)->ShootBullet();

		int result = (*EnemyItor)->BulletMoving();
		switch (result)
		{
		case BulletShootKind::kPlayer1:
		case BulletShootKind::kPlayer2:
			for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
			{
				if ((*itor)->GetID() + kSignPlayer == result)
				{
					(*itor)->BeKilled();
					break;
				}
			}
			break;

		case BulletShootKind::kCamp:

			// 如果之前没有设置过该 flag (玩家被消灭完hi设置该 flag, 此时不用再次设置)
			if (gameover_flag_ == false)
			{
				gameover_x_ = kCenterWidth / 2 - kGameoverWidth / 2;
				gameover_y_ = kCenterHeight;
				gameover_flag_ = true;
			}

			// 大本营爆炸
			is_camp_die_ = true;
			blast_.SetBlasting(11, 23);

			MciSound::PlaySounds(kSoundCampBomb);
			MciSound::PauseBGM(true);
			MciSound::PlayMovingSound(false);
			break;

		default:
			break;
		}

		// 如果敌机暂停
		if (is_enemy_pause_ == false)
		{
			(*EnemyItor)->TankMoving(center_hdc_);
		}
		else if (enemy_pause_timer_.IsTimeOut())
		{
			is_enemy_pause_ = false;
			(*EnemyItor)->SetPause(false);
		}
	}

	return true;
}

void GameControl::RefreshRightPanel()
{
	// 灰色背景
	StretchBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, GetImageHDC(&gray_background_image_), 0, 0, 66, 66, SRCCOPY);

	// 显示敌机数量图标
	int x[2] = { 233, 241 };
	int n, index;
	for (int i = 0; i < remain_enemy_tank_number_; i++)
	{
		n = i / 2;
		index = i % 2;

		TransparentBlt(image_hdc_, x[index], 19 + n * 8, kEnemyTankIcoSize, kEnemyTankIcoSize,
			GetImageHDC(&enemy_tank_ico_image_), 0, 0, kEnemyTankIcoSize, kEnemyTankIcoSize, 0xffffff);	// 注意这个图标有黑色部分
	}

	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		(*itor)->DrawPlayerTankIco(image_hdc_);
	}

	// 旗子
	TransparentBlt(image_hdc_, 232, 177, kFlagIcoSizeX, kFlagIcoSizeY,
		GetImageHDC(&flag_image_), 0, 0, kFlagIcoSizeX, kFlagIcoSizeY, 0xffffff);	// 注意图标内有黑色部分

	// 关卡
	if (current_stage_ < 10)
	{
		TransparentBlt(image_hdc_, 238, 193, 7, 7, GetImageHDC(&black_number_image_),
			kBlackNumberSize * current_stage_, 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);
	}
	else	// 10,11,12 .. 双位数关卡
	{
		TransparentBlt(image_hdc_, 233, 193, 7, 7, GetImageHDC(&black_number_image_),
			kBlackNumberSize * (current_stage_ / 10), 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);
		TransparentBlt(image_hdc_, 241, 193, 7, 7, GetImageHDC(&black_number_image_),
			kBlackNumberSize * (current_stage_ % 10), 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);
	}
}

// 更新中间游戏区域
void GameControl::RefreshCenterPanel()
{
	BitBlt(center_hdc_, 0, 0, kCenterWidth, kCenterHeight, GetImageHDC(&black_background_image_), 0, 0, SRCCOPY);// 中心黑色背景游戏区

   // 四角星闪烁控制
	for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
	{
		// 如果当前敌机还没有出现, 不会遍历下一架敌机
		StarState result = (*EnemyItor)->ShowStar(center_hdc_, remain_enemy_tank_number_);
		if (result != StarState::kTankOut)
		{
			break;
		}
	}

	// 开始根据数据文件绘制地图; 划分为 kBoxSize x kBoxSize 的格子
	// x坐标： j*kBoxSize; y坐标： i*kBoxSize
	int x = 0, y = 0;
	for (int i = 0; i < 26; i++)
	{
		for (int j = 0; j < 26; j++)
		{
			x = j * kBoxSize;// +kCenterX;
			y = i * kBoxSize;// +kCenterY;
			switch (box_mark_->box_8[i][j])
			{
			case kElementWall:
				BitBlt(center_hdc_, x, y, kBoxSize, kBoxSize, GetImageHDC(&wall_image_), 0, 0, SRCCOPY);
				break;
			case kElementIce:
				BitBlt(center_hdc_, x, y, kBoxSize, kBoxSize, GetImageHDC(&ice_image_), 0, 0, SRCCOPY);
				break;
			case kElementRiver:
				BitBlt(center_hdc_, x, y, kBoxSize, kBoxSize, GetImageHDC(&river_image_[0]), 0, 0, SRCCOPY);
				break;
			case kElementStone:
				BitBlt(center_hdc_, x, y, kBoxSize, kBoxSize, GetImageHDC(&stone_image_), 0, 0, SRCCOPY);
				break;
			default:
				break;
			}
		}
	}

	// 检测被销毁的障碍物, 绘制黑色图片擦除
	for (int i = 0; i < 52; i++)
	{
		for (int j = 0; j < 52; j++)
		{
			if (box_mark_->box_4[i][j] == kElementClear)
			{
				BitBlt(center_hdc_, j * kSmallBoxSize, i * kSmallBoxSize, kSmallBoxSize, kSmallBoxSize,
					GetImageHDC(&black_background_image_), 0, 0, SRCCOPY);
			}
		}
	}

	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		(*itor)->ShowStar(center_hdc_);
		(*itor)->DrawPlayerTank(center_hdc_);		// 坦克
		(*itor)->DrawBullet(center_hdc_);
		CheckKillEnemy(*itor);
		(*itor)->CheckShowGameOver(center_hdc_);
	}

	// 敌机
	for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
	{
		(*EnemyItor)->DrawTank(center_hdc_);
		(*EnemyItor)->DrawBullet(center_hdc_);
	}

	// 森林
	for (int i = 0; i < 26; i++)
	{
		for (int j = 0; j < 26; j++)
		{
			x = j * kBoxSize;// +kCenterX;
			y = i * kBoxSize;// +kCenterY;
			if (box_mark_->box_8[i][j] == kElementForest)
			{
				TransparentBlt(center_hdc_, x, y, kBoxSize, kBoxSize, GetImageHDC(&forest_image_), 0, 0, kBoxSize, kBoxSize, 0x000000);
			}
		}
	}

	// 敌机子弹\坦克爆炸图, 不能重合
	for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
	{
		(*EnemyItor)->Bombing(center_hdc_);

		// 爆炸完毕, 移除敌机
		if ((*EnemyItor)->Blasting(center_hdc_))
		{
		}
	}

	bool player_all_die = true;
	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		(*itor)->Bombing(center_hdc_);
		if ((*itor)->Blasting(center_hdc_) == true)		// 玩家生命用光
		{
			// 如果是双人玩家, 并且有一个玩家还没有被消灭, 那么当前这个被消灭的玩家就显示 gameover字样
			if (player_list_.size() == 2 && (player_list_.front()->IsLifeEnd() == false || player_list_.back()->IsLifeEnd() == false))
			{
				(*itor)->SetShowGameover();
			}
		}
		if (!(*itor)->IsLifeEnd())
		{
			player_all_die = false;
		}
	}

	// 玩家被消灭完
	if (player_all_die && gameover_flag_ == false)
	{
		gameover_x_ = kCenterWidth / 2 - kGameoverWidth / 2;
		gameover_y_ = kCenterHeight;
		gameover_flag_ = true;

		MciSound::PauseBGM(true);
		MciSound::PlayMovingSound(false);
	}

	// 道具闪烁, 内部自定义时钟
	PlayerBase::ShowProp(center_hdc_);

	// 大本营
	if (!is_camp_die_)		// 如果没爆炸
	{
		TransparentBlt(center_hdc_, kBoxSize * 12, kBoxSize * 24, kBoxSize * 2, kBoxSize * 2,
			GetImageHDC(&camp_image_[0]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
	}
	else	// 显示被摧毁的camp
	{
		TransparentBlt(center_hdc_, kBoxSize * 12, kBoxSize * 24, kBoxSize * 2, kBoxSize * 2,
			GetImageHDC(&camp_image_[1]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
	}

	blast_.CampBlasting(center_hdc_);

	IsWinOver();
	IsGameOver();
}

// 读取PlayerBase 内的数据, 消灭敌机
void GameControl::CheckKillEnemy(PlayerBase *player_base)
{
	int bullet[2] = { 0, 0 };
	player_base->GetKillEnemy(bullet[0], bullet[1]);		// 获取玩家击中的敌机id, 存储进 bullet[2] 内

	for (int i = 0; i < 2; i++)
	{
		if (bullet[i] >= kSignEnemy)
		{
			for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
			{
				if ((*EnemyItor)->GetId() == bullet[i] % 100)		// 100xx 后两位是 id
				{
					// 如果消灭敌机
					if ((*EnemyItor)->BeKilled(false))
					{
						killed_enemy_number_++;
						cur_moving_tank_number_--;
						if ((int)TankKind::kProp == bullet[i] % 1000 / 100)		// 获取百分位的敌机种类
						{
							PlayerBase::SetShowProp();
						}

						// 玩家记录消灭的敌机数量
						player_base->AddKillEnemyNum((*EnemyItor)->GetLevel());
					}
					if (killed_enemy_number_ == 20)
					{
						win_counter_ = 0;
						is_win_ = true;
					}
					break;
				}
			}
		}
	}
}
void GameControl::IsGameOver()
{
	if (!gameover_flag_)
	{
		return;
	}

	TransparentBlt(center_hdc_, gameover_x_, gameover_y_, kGameoverWidth, kGameoverHeight,
		GetImageHDC(&gameover_image_), 0, 0, kGameoverWidth, kGameoverHeight, 0x000000);

	if (gameover_timer_.IsTimeOut() && gameover_y_ >= kCenterHeight * 0.45)
	{
		gameover_y_ -= 2;
	}
	else if (gameover_y_ < kCenterHeight * 0.45)
	{
		gameover_counter_++;
	}

	if (gameover_counter_ > 250 && show_score_panel_ == false)
	{
		show_score_panel_ = true;
		is_win_ = false;
		for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
		{
			(*itor)->ResetScorePanelData(player_list_.size(), current_stage_);
		}
	}
}

// 如果敌机都被消灭, 隔 mWinCounter 后跳转到分数面板
void GameControl::IsWinOver()
{
	if (is_win_ && win_counter_++ > 210 && !gameover_flag_ && show_score_panel_ == false)
	{
		MciSound::PauseBGM(true);
		show_score_panel_ = true;
		for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
		{
			(*itor)->ResetScorePanelData(player_list_.size(), current_stage_);
		}
	}
}
} // namespace battlecity