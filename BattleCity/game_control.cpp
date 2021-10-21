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

	loadimage(&black_background_image_, _T("./res/big/bg_black.gif"));		// ��ɫ����
	loadimage(&gray_background_image_, _T("./res/big/bg_gray.gif"));		// ��ɫ����
	loadimage(&stone_image_, _T("./res/big/stone.gif"));					// 12*12��ʯͷ
	loadimage(&forest_image_, _T("./res/big/forest.gif"));					// ����
	loadimage(&ice_image_, _T("./res/big/ice.gif"));						// ����
	loadimage(&river_image_[0], _T("./res/big/river-0.gif"));				// ����
	loadimage(&river_image_[1], _T("./res/big/river-1.gif"));		
	loadimage(&wall_image_, _T("./res/big/wall.gif"));						// ��ǽ
	loadimage(&camp_image_[0], _T("./res/big/camp0.gif"));					// ��Ӫ
	loadimage(&camp_image_[1], _T("./res/big/camp1.gif"));		 
	loadimage(&enemy_tank_ico_image_, _T("./res/big/enemytank-ico.gif"));	// �л�ͼ��
	loadimage(&flag_image_, _T("./res/big/flag.gif"));						// ����
	loadimage(&current_stage_image_, _T("./res/big/stage.gif"));
	loadimage(&black_number_image_, _T("./res/big/black-number.gif"));		// 0123456789 ��ɫ����
	loadimage(&gameover_image_, _T("./res/big/gameover.gif"));

	// �Զ�����Ƶ�ͼ
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
	remain_enemy_tank_number_ = 20;	 // ʣ��δ���ֵĵл�����
	cur_moving_tank_number_ = 0;
	killed_enemy_number_ = 0;
	is_camp_die_ = false;													// ��־��Ӫ�Ƿ񱻻���

	is_enemy_pause_ = false;			// �л���ͣ���
	enemy_pause_timer_.SetDrtTime(10000);	// �л���ͣ���

	main_timer_.SetDrtTime(14);
	camp_timer_.SetDrtTime(33);

	cut_stage_counter_ = 0;		// STAGE ��������

	// GameOver ͼƬ
	gameover_counter_ = 0;
	gameover_x_ = -100;
	gameover_y_ = -100;
	gameover_flag_ = false;
	gameover_timer_.SetDrtTime(30);

	// ���Ƶ�ͼ̹���α������
	cmt_image_x_ = kBoxSize;
	cmt_image_y_ = kBoxSize;

	// ÿ�ν����ͼ����֮ǰ�����֮ǰ�Ƿ���������ͼ
	has_custom_map_ = false;

	// �ؿ�������ʾ�������
	show_score_panel_ = false;

	// ʤ��
	is_win_ = false;
	win_counter_ = 0;

	msgoas_counter_ = 0;
	msgoas_y_ = kCenterHeight;
	show_gameover_after_score_panel_ = false;
}

// �洢��ҽ�����
void GameControl::AddPlayer(int player_num)
{
	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		delete *itor;
	}

	// ���ԭ������
	player_list_.clear();

	for (int i = 0; i < player_num; i++)
	{
		player_list_.push_back(new PlayerBase(i, box_mark_));
	}

	PlayerBase::player_list_ = &player_list_;
}


// ��ȡdata���ݻ��Ƶ�ͼ,��ʾ�л�����\�������\�ؿ�\����Ϣ
void GameControl::LoadMap()
{
	// ��ȡ��ͼ�ļ�����
	FILE *fp = NULL;
	if (0 != fopen_s(&fp, "./res/data/map.dat", "rb"))
	{
		throw _T("��ȡ��ͼ�����ļ��쳣.");
	}
	fseek(fp, sizeof(Map) * (current_stage_ - 1), SEEK_SET);
	fread(&map_, sizeof(Map), 1, fp);
	fclose(fp);

	InitSignBox();
}

// ����Լ�������ͼ
bool GameControl::CreateMap(bool *isCreate)
{
	int i, j, x = 0, y = 0;
	int tempx, tempy;
	bool flag = true;
	int keys[4] = { VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN };		// �±������ DIR_LEFT �ȶ�Ӧ
	int dev[4][2] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} };	// �α�̹���ƶ�����
	int twinkle_counter = 0;	// ̹���α���˸����
	int lastx = cmt_image_x_, lasty = cmt_image_y_;			// ��¼̹���ϴε�����, ���̹���ƶ�����任 sign_order ͼ��
	bool M_down = false;		// ��� M �Ƿ���, һֱ���� M �����л���ͼ, ֻ���µ���һ���л�

	// 14 �����
	int sign_order[14][4] = { {kElementIce, kElementIce, kElementIce, kElementIce},		// �ĸ����Ӷ��Ǳ�, ���������������� ..
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

	int cur_index = 13;		// ��Ӧ��������

	// ��������ϴλ��Ƶĵ�ͼ
	if (!has_custom_map_)
	{
		ClearSignBox();
	}

	// �����ٶ�
	TimeClock click;
	click.SetDrtTime(80);

	// ��ɫ����
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

			// ���� 16*16 �ĵ�ͼ
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

			// ���� 16*16 �ĵ�ͼ
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

			// ��� 8*8 �����ڲ��� 4*4 ����
			for (i = 0; i < 26; i++)
			{
				for (j = 0; j < 26; j++)
				{
					// ���� 8*8 ��� 4*4 ����, ��Ӫֵ��� box_8
					if (box_mark_->box_8[i][j] != kElementEmpty && box_mark_->box_8[i][j] != kSignCamp)
					{
						SignBox_4(i, j, box_mark_->box_8[i][j]);
					}

					// ��յл����ֵ�����λ��
					if (i <= 1 && j <= 1 || j >= 12 && j <= 13 && i <= 1 || j >= 24 && i <= 1)
					{
						box_mark_->box_8[i][j] = kElementEmpty;
						SignBox_4(i, j, kElementEmpty);
					}

					// ��λ�ò��ܻ���
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

		// ��ɫ����
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

		// ��Ӫ
		TransparentBlt(center_hdc_, kBoxSize * 12, kBoxSize * 24, kBoxSize * 2, kBoxSize * 2,
			GetImageHDC(&camp_image_[0]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);

		// ̹���α�
		if (twinkle_counter++ / 28 % 2 == 0)
		{
			TransparentBlt(center_hdc_, cmt_image_x_ - kBoxSize, cmt_image_y_ - kBoxSize, kBoxSize * 2, kBoxSize * 2,
				GetImageHDC(&create_map_tank_image_), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
		}
		// �����Ļ���ӡ�������� mImage_hdc ��
		BitBlt(image_hdc_, kCenterX, kCenterY, kCenterWidth, kCenterHeight, center_hdc_, 0, 0, SRCCOPY);
		// ���Ż���������ʾ image ��������
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

// ����ѭ����; ����ˢ�£���ҡ��ӵ����л� ���еĸ��¶��ڴ�; �������
GameResult GameControl::StartGame()
{
	// ����ͼ����ʱ��
	if (main_timer_.IsTimeOut())
	{
		// ʤ������ʧ�� ��ʾ�������
		if (show_score_panel_)
		{
			BitBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, GetImageHDC(&ScorePanel::background_), 0, 0, SRCCOPY);
			for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
			{
				// ������������ʾ��
				if (!(*itor)->ShowScorePanel(image_hdc_))
				{
					show_score_panel_ = false;

					// ʤ����ʧ�ܶ��ͷŵл���Դ
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

						// ��̬���ݻᱣ��,��Ҫ�ֶ�����
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

			// ���Ż���������ʾ image ��������
			StretchBlt(des_hdc_, 0, 0, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
			FlushBatchDraw();
			return GameResult::kVictory;
		}

		// ������ GAMEOVER ����
		if (show_gameover_after_score_panel_)
		{
			StretchBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, GetImageHDC(&black_background_image_),
				0, 0, kCenterWidth, kCenterHeight, SRCCOPY);
			BitBlt(image_hdc_, 66, msgoas_y_, 124, 80, GetImageHDC(&msgoas_image_), 0, 0, SRCCOPY);

			// ���Ż���������ʾ image ��������
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

		// �����ұ���������, ���ж�, ��Ϊ����Ҫ�������� mImage_hdc
		RefreshRightPanel();

		// ����������Ϸ����: center_hdc
		RefreshCenterPanel();

		// �����Ļ���ӡ�������� mImage_hdc ��
		BitBlt(image_hdc_, kCenterX, kCenterY, kCenterWidth, kCenterHeight, center_hdc_, 0, 0, SRCCOPY);
		// ���Ż���������ʾ image ��������
		StretchBlt(des_hdc_, 0, 0, kWindowWidth, kWindowHeight, image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, SRCCOPY);
		FlushBatchDraw();
	}

	// ���ݱ仯, �����漰��ͼ����
	RefreshData();

	return GameResult::kVictory;
}

// ˽�к���,����ʹ��

void GameControl::CutStage()
{
	// ��ɫ����
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
	// ��ɫ����
	StretchBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight,
		GetImageHDC(&gray_background_image_), 0, 0, 66, 66, SRCCOPY);


	TransparentBlt(image_hdc_, 97, 103, 39, 7, GetImageHDC(&current_stage_image_), 0, 0, 39, 7, 0xffffff);

	// [1-9] �ؿ�����������
	if (current_stage_ < 10)
	{
		TransparentBlt(image_hdc_, 157, 103, kBlackNumberSize, kBlackNumberSize,
			GetImageHDC(&black_number_image_), kBlackNumberSize * current_stage_, 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);
	}
	else	// 10,11,12 .. ˫λ���ؿ�
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
	// ��ʼ����Ǹ��ָ���
	int x = 0, y = 0;
	for (int i = 0; i < 26; i++)
	{
		for (int j = 0; j < 26; j++)
		{
			box_mark_->prop_8[i][j] = kElementEmpty;
			box_mark_->box_8[i][j] = kElementEmpty;	// 26*26
			SignBox_4(i, j, kElementEmpty);		// ��� 26*26 �� 52*52 ����
		}
	}

	// ��Ǵ�Ӫ
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
				box_mark_->box_8[i][j] = kElementWall;			// ����Χ�� kElementWall
			}
		}
	}
}

//
void GameControl::InitSignBox()
{
	// ��ʼ����Ǹ��ָ���
	int x = 0, y = 0;
	for (int i = 0; i < 26; i++)
	{
		for (int j = 0; j < 26; j++)
		{
			box_mark_->prop_8[i][j] = kElementEmpty;
			box_mark_->box_8[i][j] = map_.buf[i][j] - '0';	// 26*26
			SignBox_4(i, j, map_.buf[i][j] - '0');		// ��� 26*26 �� 52*52 ����
		}
	}

	// ��Ǵ�Ӫ
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
				box_mark_->box_8[i][j] = kElementWall;			// ����Χ�� kElementWall
			}
		}
	}
}

// ���޸�, ��ӵĵл�������Ҫ�޸�
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

	// ÿ��5�ܳ�һ�ܵ���̹��
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

// �ṩ8*8 �����Ͻ�����, �������4�� 4*4 �ĸ���
void GameControl::SignBox_4(int i, int j, int sign_val)
{
	int temp_i[4] = { 2 * i, 2 * i + 1, 2 * i, 2 * i + 1 };
	int temp_j[4] = { 2 * j, 2 * j, 2 * j + 1, 2 * j + 1 };

	for (int i = 0; i < 4; i++)
		box_mark_->box_4[temp_i[i]][temp_j[i]] = sign_val;
}

//  ���ݸ���, ���漰��ͼ����!!�������ж��������꣬��һ��ѭ�����н�������߻�ͼ��ʵ���˶�
bool GameControl::RefreshData()
{
	if (GetAsyncKeyState(27) & 0x8000)
	{
		return false;
	}

	// �������Ƿ��� 'ʱ��' ��ֹ����
	if (PlayerBase::IsGetTimeProp())
	{
		is_enemy_pause_ = true;
		enemy_pause_timer_.Init();		// ���� t1 = t2
		EnemyBase::SetPause(true);
	}

	// ѭ�����ӵ����߼�
	PlayerBase::IsGetShvelProp();

	// ��һ�õ��׵���
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

					// ��Ҽ�¼����ĵл�����
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

	// ���, ���ܰ�����ͼ����! �ں���ʱ��
	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		// �񳲱��������ֹͣ�ƶ�
		if (!gameover_flag_)
		{
			(*itor)->PlayerControl();
		}

		// ����ӵ����н��
		BulletShootKind kind = (*itor)->BulletMoving(center_hdc_);
		switch (kind)
		{
		case BulletShootKind::kCamp:
			gameover_x_ = kCenterWidth / 2 - kGameoverWidth / 2;
			gameover_y_ = kCenterHeight;
			gameover_flag_ = true;

			// ��Ӫ��ը
			is_camp_die_ = true;
			blast_.SetBlasting(11, 23);

			MciSound::PlaySounds(kSoundCampBomb);
			MciSound::PauseBGM(true);
			MciSound::PlayMovingSound(false);
			break;

			// ���������е���� Ȼ����ͣ��
		case BulletShootKind::kPlayer1:
			for (std::list<PlayerBase *>::iterator i = player_list_.begin(); i != player_list_.end(); i++)
			{
				if ((*i)->GetID() == 0)
				{
					(*i)->SetPause();
				}
			}
			break;

			// ���������е���� Ȼ����ͣ��
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

	// �л�, �˴����ܰ����ƻ�ͼ����, �ں���ʱ��, ��Ȼ�ǻᵼ�¼�ʱ��������ʱ����һ��,����ʧ֡
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

			// ���֮ǰû�����ù��� flag (��ұ�������hi���ø� flag, ��ʱ�����ٴ�����)
			if (gameover_flag_ == false)
			{
				gameover_x_ = kCenterWidth / 2 - kGameoverWidth / 2;
				gameover_y_ = kCenterHeight;
				gameover_flag_ = true;
			}

			// ��Ӫ��ը
			is_camp_die_ = true;
			blast_.SetBlasting(11, 23);

			MciSound::PlaySounds(kSoundCampBomb);
			MciSound::PauseBGM(true);
			MciSound::PlayMovingSound(false);
			break;

		default:
			break;
		}

		// ����л���ͣ
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
	// ��ɫ����
	StretchBlt(image_hdc_, 0, 0, kCanvasWidth, kCanvasHeight, GetImageHDC(&gray_background_image_), 0, 0, 66, 66, SRCCOPY);

	// ��ʾ�л�����ͼ��
	int x[2] = { 233, 241 };
	int n, index;
	for (int i = 0; i < remain_enemy_tank_number_; i++)
	{
		n = i / 2;
		index = i % 2;

		TransparentBlt(image_hdc_, x[index], 19 + n * 8, kEnemyTankIcoSize, kEnemyTankIcoSize,
			GetImageHDC(&enemy_tank_ico_image_), 0, 0, kEnemyTankIcoSize, kEnemyTankIcoSize, 0xffffff);	// ע�����ͼ���к�ɫ����
	}

	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		(*itor)->DrawPlayerTankIco(image_hdc_);
	}

	// ����
	TransparentBlt(image_hdc_, 232, 177, kFlagIcoSizeX, kFlagIcoSizeY,
		GetImageHDC(&flag_image_), 0, 0, kFlagIcoSizeX, kFlagIcoSizeY, 0xffffff);	// ע��ͼ�����к�ɫ����

	// �ؿ�
	if (current_stage_ < 10)
	{
		TransparentBlt(image_hdc_, 238, 193, 7, 7, GetImageHDC(&black_number_image_),
			kBlackNumberSize * current_stage_, 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);
	}
	else	// 10,11,12 .. ˫λ���ؿ�
	{
		TransparentBlt(image_hdc_, 233, 193, 7, 7, GetImageHDC(&black_number_image_),
			kBlackNumberSize * (current_stage_ / 10), 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);
		TransparentBlt(image_hdc_, 241, 193, 7, 7, GetImageHDC(&black_number_image_),
			kBlackNumberSize * (current_stage_ % 10), 0, kBlackNumberSize, kBlackNumberSize, 0xffffff);
	}
}

// �����м���Ϸ����
void GameControl::RefreshCenterPanel()
{
	BitBlt(center_hdc_, 0, 0, kCenterWidth, kCenterHeight, GetImageHDC(&black_background_image_), 0, 0, SRCCOPY);// ���ĺ�ɫ������Ϸ��

   // �Ľ�����˸����
	for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
	{
		// �����ǰ�л���û�г���, ���������һ�ܵл�
		StarState result = (*EnemyItor)->ShowStar(center_hdc_, remain_enemy_tank_number_);
		if (result != StarState::kTankOut)
		{
			break;
		}
	}

	// ��ʼ���������ļ����Ƶ�ͼ; ����Ϊ kBoxSize x kBoxSize �ĸ���
	// x���꣺ j*kBoxSize; y���꣺ i*kBoxSize
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

	// ��ⱻ���ٵ��ϰ���, ���ƺ�ɫͼƬ����
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
		(*itor)->DrawPlayerTank(center_hdc_);		// ̹��
		(*itor)->DrawBullet(center_hdc_);
		CheckKillEnemy(*itor);
		(*itor)->CheckShowGameOver(center_hdc_);
	}

	// �л�
	for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
	{
		(*EnemyItor)->DrawTank(center_hdc_);
		(*EnemyItor)->DrawBullet(center_hdc_);
	}

	// ɭ��
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

	// �л��ӵ�\̹�˱�ըͼ, �����غ�
	for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
	{
		(*EnemyItor)->Bombing(center_hdc_);

		// ��ը���, �Ƴ��л�
		if ((*EnemyItor)->Blasting(center_hdc_))
		{
		}
	}

	bool player_all_die = true;
	for (std::list<PlayerBase *>::iterator itor = player_list_.begin(); itor != player_list_.end(); itor++)
	{
		(*itor)->Bombing(center_hdc_);
		if ((*itor)->Blasting(center_hdc_) == true)		// ��������ù�
		{
			// �����˫�����, ������һ����һ�û�б�����, ��ô��ǰ������������Ҿ���ʾ gameover����
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

	// ��ұ�������
	if (player_all_die && gameover_flag_ == false)
	{
		gameover_x_ = kCenterWidth / 2 - kGameoverWidth / 2;
		gameover_y_ = kCenterHeight;
		gameover_flag_ = true;

		MciSound::PauseBGM(true);
		MciSound::PlayMovingSound(false);
	}

	// ������˸, �ڲ��Զ���ʱ��
	PlayerBase::ShowProp(center_hdc_);

	// ��Ӫ
	if (!is_camp_die_)		// ���û��ը
	{
		TransparentBlt(center_hdc_, kBoxSize * 12, kBoxSize * 24, kBoxSize * 2, kBoxSize * 2,
			GetImageHDC(&camp_image_[0]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
	}
	else	// ��ʾ���ݻٵ�camp
	{
		TransparentBlt(center_hdc_, kBoxSize * 12, kBoxSize * 24, kBoxSize * 2, kBoxSize * 2,
			GetImageHDC(&camp_image_[1]), 0, 0, kBoxSize * 2, kBoxSize * 2, 0x000000);
	}

	blast_.CampBlasting(center_hdc_);

	IsWinOver();
	IsGameOver();
}

// ��ȡPlayerBase �ڵ�����, ����л�
void GameControl::CheckKillEnemy(PlayerBase *player_base)
{
	int bullet[2] = { 0, 0 };
	player_base->GetKillEnemy(bullet[0], bullet[1]);		// ��ȡ��һ��еĵл�id, �洢�� bullet[2] ��

	for (int i = 0; i < 2; i++)
	{
		if (bullet[i] >= kSignEnemy)
		{
			for (std::list<EnemyBase *>::iterator EnemyItor = enemy_list_.begin(); EnemyItor != enemy_list_.end(); EnemyItor++)
			{
				if ((*EnemyItor)->GetId() == bullet[i] % 100)		// 100xx ����λ�� id
				{
					// �������л�
					if ((*EnemyItor)->BeKilled(false))
					{
						killed_enemy_number_++;
						cur_moving_tank_number_--;
						if ((int)TankKind::kProp == bullet[i] % 1000 / 100)		// ��ȡ�ٷ�λ�ĵл�����
						{
							PlayerBase::SetShowProp();
						}

						// ��Ҽ�¼����ĵл�����
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

// ����л���������, �� mWinCounter ����ת���������
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