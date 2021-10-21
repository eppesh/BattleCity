#include "stdafx.h"
#include "player_base.h"
#include "mci_sound.h"

namespace battlecity
{
// PlayerBase �ྲ̬����

int PlayerBase::dev_xy_[4][2] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} };	// ������������
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

	player_life_ = 2;		// ��� HP
	player_tank_level_ = 0;

	Init();

	// ��ͬ������ݲ�ͬ
	if (player_id_ == 0)
	{
		loadimage(&p12_image_, _T("./res/big/1P.gif"));		// 1P\2Pͼ��
	}
	else
	{
		loadimage(&p12_image_, _T("./res/big/2P.gif"));
	}

	// ��ͬ������
	loadimage(&player_tank_ico_image_, _T("./res/big/playertank-ico.gif"));	// ���̹��ͼ��
	loadimage(&black_number_image_, _T("./res/big/black-number.gif"));	// ��ɫ����

	// Bullet ���ݳ�ʼ��
	// �����ӵ�ͼƬ
	TCHAR buf[100];
	for (i = 0; i < 4; i++)
	{
		_stprintf_s(buf, _T("./res/big/bullet-%d.gif"), i);
		loadimage(&Bullet::bullet_image[i], buf);
	}

	// ��ըͼƬ
	for (i = 0; i < 3; i++)
	{
		_stprintf_s(buf, _T("./res/big/bumb%d.gif"), i);
		loadimage(&Bomb::bomb_image[i], buf);
	}

	bomb_timer_.SetDrtTime(20);	// ��������̫С..

	// ��ʾ�������
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

	// ��ͬ������ݲ�ͬ
	if (player_id_ == 0)
	{
		p12_image_x_ = 233;									// 1P\2P ����
		p12_image_y_ = 129;
		player_tank_ico_image_x_ = 232;						// ���̹��ͼ������
		player_tank_ico_image_y_ = 137;
		player_life_x_ = 240;								// �������ֵ����
		player_life_y_ = 137;
		tank_x_ = 4 * 16 + kBoxSize;						// ̹���״γ���ʱ�����������
		tank_y_ = 12 * 16 + kBoxSize;

		tank_timer_.SetDrtTime(move_speed_dev_[player_tank_level_]);		// ̹���ƶ��ٶ�, ��ͬ����ͬ��� ��һ��
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
	tank_dir_ = kDirUp;		// ̹�˷���

	// �ӵ��ṹ����
	int temp_speed[4] = { 2,3,3,4 };			// ���ܳ��� 4 !! ����Ծ�����ж�.����̹�˼�������ӵ��ٶ�ϵ��
	for (i = 0; i < 2; i++)
	{
		bullet_[i].x = kShootTableX;			// x ���������ж��Ƿ���Է���
		bullet_[i].y = -1000;
		bullet_[i].direction = kDirUp;
		bullet_[i].killed_id = 0;				// ��¼���еĵл� id

												// ����̹�˼�������ӵ��ٶ�ϵ��
		for (int j = 0; j < 4; j++)
		{
			bullet_[i].speed[j] = temp_speed[j];
		}
	}

	bullet1_counter_ = 9;						// �ӵ� 1 �˶� N ��ѭ����ſ��Է����ӵ� 2 
	is_moving_ = false;

	// ��ըͼƬ
	for (i = 0; i < 2; i++)
	{
		bombs_[i].bomb_x = -100;
		bombs_[i].bomb_y = -100;
		bombs_[i].is_show_bomb = false;
		bombs_[i].counter = 0;
	}

	// SendKillNumToScorePanel() ��ʹ��
	has_send_killednum_ = false;

	// ɱ����
	for (i = 0; i < 4; i++)
	{
		kill_enemy_number_[i] = 0;
	}

	is_pause_ = false;
	pause_counter_ = 0;

	// ̹���Ƿ��ڱ����ƶ�
	on_ice_ = false;
	automove_ = false;
	automove_counter_ = 0;
	rand_counter_ = rand() % 6 + 3;
}

// ������ҵ�һЩ����: 1P\2P ̹��ͼ�� ����
void PlayerBase::DrawPlayerTankIco(const HDC &right_panel_hdc)
{
	// ����1P/2P
	TransparentBlt(right_panel_hdc, p12_image_x_, p12_image_y_, kPlayer2IcoSizeX, kPlayer2IcoSizeY,
		GetImageHDC(&p12_image_), 0, 0, kPlayer2IcoSizeX, kPlayer2IcoSizeY, 0xffffff);
	// ����̹��ͼ��
	TransparentBlt(right_panel_hdc, player_tank_ico_image_x_, player_tank_ico_image_y_, kPlayerTankIcoSizeX, kPlayerTankIcoSizeY,
		GetImageHDC(&player_tank_ico_image_), 0, 0, kPlayerTankIcoSizeX, kPlayerTankIcoSizeY, 0x000000);
	// ���������������
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
		SignBox_8(tank_x_, tank_y_, kElementEmpty);		// ��ֹ��һ��Ƶ�ͼ��̹�˳��ֵ�λ���ڵ�ס
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

	// 0-5����ʾ̹��. 6-11 ��ʾ.. ��������
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

	// ��ʾ������
	if (ring_.can_show_)
	{
		if (ring_.ShowRing(canvas_hdc, tank_x_, tank_y_) == false)
		{
			// ��ֹ��ұ�ը��������ʱ��,����λ�ñ��л�ռ��,Ȼ����ұ�Ǳ�����
			SignBox_4(tank_x_, tank_y_, kSignPlayer + player_id_);
		}
	}
}

void PlayerBase::DrawBullet(const HDC &center_hdc)
{
	for (int i = 0; i < 2; i++)
	{
		// �ӵ����ƶ�
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
	case 0:										// ���һ
		if (GetAsyncKeyState('A') & 0x8000)
		{
			// ͬ�����ƶ��ſ����Զ��ƶ�
			if (on_ice_ && tank_dir_ == kDirLeft)	// ���̹�˳����� ��
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
			// ͬ�����ƶ��ſ����Զ��ƶ�
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
			// ͬ�����ƶ��ſ����Զ��ƶ�
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
			// ͬ�����ƶ��ſ����Զ��ƶ�
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
		else if (is_moving_)		// �ɿ�������ֹͣ�����ƶ�������������²������ɿ����������ź�����������ͣ����������������
		{
			is_moving_ = false;
			MciSound::PlayMovingSound(false);
		}

		// ���ܼ� else if, ��Ȼ�ƶ�ʱ���޷������ӵ�
		if (GetAsyncKeyState('J') & 0x8000)	// �����ӵ�
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

	case 1:										// ��Ҷ�
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			// ͬ�����ƶ��ſ����Զ��ƶ�
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
			// ͬ�����ƶ��ſ����Զ��ƶ�
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
			// ͬ�����ƶ��ſ����Զ��ƶ�
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
			// ͬ�����ƶ��ſ����Զ��ƶ�
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

		// ���ּ� 1 �����ӵ�
		if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000)
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
		// �ӵ����ƶ�
		if (bullet_[i].x != kShootTableX)
		{
			int dir = bullet_[i].direction;

			// �������ϰ������
			BulletShootKind kind = CheckBomb(i);
			if (kind == BulletShootKind::kCamp || kind == BulletShootKind::kPlayer1 || kind == BulletShootKind::kPlayer2)
			{
				return kind;
			}
			else if (kind == BulletShootKind::kOther)
			{
				continue;
			}

			// �ȼ����ȡ�����
			SignBullet(bullet_[i].x, bullet_[i].y, dir, kElementEmpty);

			bullet_[i].x += dev_xy_[dir][0] * bullet_[i].speed[player_tank_level_];
			bullet_[i].y += dev_xy_[dir][1] * bullet_[i].speed[player_tank_level_];

			// ��¼�ӵ� 1 �Ĳ���, �����ɷ����ӵ� 2
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

// GameControl �ڵ���, ͨ�������� bullet_.killed_id ���ݽ�ȥ
void PlayerBase::GetKillEnemy(int &bullet1, int &bullet2)
{
	bullet1 = bullet_[0].killed_id;
	bullet2 = bullet_[1].killed_id;

	// ���ñ�־
	bullet_[0].killed_id = 0;
	bullet_[1].killed_id = 0;
}


void PlayerBase::BeKilled()
{
	// �����ʾ�ű����������ܵ�����
	if (ring_.can_show_)
	{
		return;
	}

	MciSound::PlaySounds(kSoundPlayerBomb);
	SignBox_4(tank_x_, tank_y_, kElementEmpty);
	
	blast_.SetBlasting(tank_x_, tank_y_);
}

// ��ұ����б�ը
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
		// ����Ƿ��������
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

/*GameControl ��ѭ������*/
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
		// �ջ�ò��ӵ���
		if (shovel_prop_counter_++ == 0)
		{
			ProtectCamp(kElementStone);
		}
		else if (shovel_prop_counter_ > 31000 && shovel_prop_counter_ < 35400)		// ��ʾһ��ʱ�����˸
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

/*GameControl ��ѭ������*/
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
	return score_panel_->Show(image_hdc);// ���Ż���������ʾ image ��������
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
	// ת����ͷ����
	int hx = left_x + Bullet::devto_head[dir][0];
	int hy = top_y + Bullet::devto_head[dir][1];

	// ת���� 4*4 �����±�����
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
	tank_x_ = (4 + 4 * player_id_) * 16 + kBoxSize;				// ̹���״γ���ʱ�����������
	tank_y_ = 12 * 16 + kBoxSize;
	SignBox_4(tank_x_, tank_y_, kSignPlayer + player_id_);		// ̹�˳���, ���Ľ��Ǳ�Ǹ�Ϊ̹�˱��

	player_tank_level_ = 0;										// ̹�˼��� [0-3]
	tank_timer_.SetDrtTime(move_speed_dev_[player_tank_level_]);
	bullet_timer_.SetDrtTime(bullet_speed_dev_[player_tank_level_]);

	tank_dir_ = kDirUp;											// ̹�˷���

	bullet1_counter_ = 6;										// �ӵ� 1 �˶� N ��ѭ����ſ��Է����ӵ� 2 
	is_moving_ = false;
	ring_.SetShowable(3222);									// ��ʾ������
}

void PlayerBase::DispatchProp(int prop_kind)
{
	MciSound::PlaySounds(kSoundGetProp);
	prop_.StopShowProp(true);

	switch (prop_kind)
	{
	case kPropAdd:			// �ӻ�
		MciSound::PlaySounds(kSoundAddLife);
		player_life_ = player_life_ + 1 > 5 ? 5 : player_life_ + 1;
		break;
	case kPropStar:			// �����
		player_tank_level_ = player_tank_level_ + 1 > 3 ? 3 : player_tank_level_ + 1;
		tank_timer_.SetDrtTime(move_speed_dev_[player_tank_level_]);
		bullet_timer_.SetDrtTime(bullet_speed_dev_[player_tank_level_]);
		break;
	case kPropTime:			// ʱ��
		get_time_prop_ = true;
		break;
	case  kPropBomb:		// ����
		get_bomb_prop_ = true;
		break;
	case kPropShovel:		// ����
		get_shovel_prop_ = true;
		break;
	case  kPropCap:			// ñ��
		ring_.SetShowable(12000);
		break;
	default:
		break;
	}
}

/*
* �����ͬʱ����̹�����ڸ���. ���뱣֤̹�������ڸ�������
 * ����ƶ���ʱ�� tank_timer_ δ��ʱ	�����ƶ�
 * �������ֵ�ù��					�����ƶ�
 * ���̹�����ڱ�ը					�����ƶ�
 * ��ұ���һ����һ��к�				�����ƶ�

 * �ƶ�ǰ���̹�� tank_x_,tank_y_ ���ڵ� box_4 �ĸ����� = �գ���ʾ�ô�û�ж�������
 * ����Ǳ�����ô���� tank_x_,tank_y_ ����ȷ�ĸ���λ����
 * ����Ƿ�����ƶ�
 * ��������ƶ��������µ� tank_x_, tank_y_ ����
 * �ƶ����� box_4 �ĸ����� = kSignPlayer + player_id_����ʾ�����̹�˴�����λ��

 * �µ����� tank_x_, tank_y_ �Ѿ�������ϣ��������أ���ѭ�����������������»���̹�ˣ��Ӷ�ʵ���ƶ�Ч����
 */
void PlayerBase::Move(int new_dir)
{
	if (!tank_timer_.IsTimeOut() || is_died_ || blast_.IsBlasting())
	{
		return;
	}

	// �����ұ���һ����һ�����ͣ
	if (is_pause_)
	{
		return;
	}

	SignBox_4(tank_x_, tank_y_, kElementEmpty);

	if (tank_dir_ != new_dir)
	{
		// ԭ���ұ����·���
		if (tank_dir_ == kDirLeft || tank_dir_ == kDirRight)
		{
			if (tank_x_ > (tank_x_ / kBoxSize) * kBoxSize + kBoxSize / 2 - 1)	// ����ǿ����������ϵ��ұ߽ڵ�, -1������
			{
				tank_x_ = (tank_x_ / kBoxSize + 1) * kBoxSize;
			}
			else
			{
				tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;					// �����������ϵ���߽ڵ�
			}
		}
		// ���±�����
		else
		{
			if (tank_y_ > (tank_y_ / kBoxSize) * kBoxSize + kBoxSize / 2 - 1)	// ����ǿ����������ϵ��±߽ڵ�, -1������
			{
				tank_y_ = (tank_y_ / kBoxSize + 1) * kBoxSize;
			}
			else
			{
				tank_y_ = (tank_y_ / kBoxSize) * kBoxSize;					// �����������ϵ��ϱ߽ڵ�
			}
		}

		// ���ķ���, �����ȵ���λ�ú�������÷���!!
		tank_dir_ = new_dir;
	}
	else								// �ƶ�
	{
		if (CheckMoveable())
		{
			tank_x_ += dev_xy_[tank_dir_][0];// *mSpeed[player_tank_level_];
			tank_y_ += dev_xy_[tank_dir_][1];// *mSpeed[player_tank_level_];
		}
	}
	SignBox_4(tank_x_, tank_y_, kSignPlayer + player_id_);
}

/* �жϵ�ǰ����ɷ��ƶ�
	box_8[i][j]
	i = y / kBoxSize		// y �����Ӧ���� i ֵ
	j = x / kBoxSize		// x �����Ӧ���� j ֵ
	---------
	| 1 | 2	|
	----a----
	| 3 | 4	|
	---------
* ��� (x,y) �� a ��, ת����� i,j ���ڸ��� 4
* ��� x ֵ�� a �����, ��ת����� j ���� 1��3; �ұ������� 2��4
* ��� y ֵ�� a ������, ��ת����� i ���� 1��2; ���������� 3��4
** ��� tempx,tempy ��Խ�˸����������ϰ�, ��ô�ͽ� tank_x_ �� tank_y_ ��������������,
*/
bool PlayerBase::CheckMoveable()
{
	// ̹����������
	int tempx = tank_x_ + dev_xy_[tank_dir_][0];// *mSpeed[player_tank_level_];
	int tempy = tank_y_ + dev_xy_[tank_dir_][1];// *mSpeed[player_tank_level_];

	// ��Ϸ��������һ�� 208*208 �Ļ����ϵģ�����ʵ����Ϸ�����С�� 208*208
	// ��������� tempx < 8 ��̹���������� < 8 ��˵����ʱ�Ѿ�Խ�磨��Ϊ�����ĵ㣩������˵̹���Ѿ��ƶ����߽���
	if (tempx < kBoxSize || tempy < kBoxSize || tempy > kCenterWidth - kBoxSize || tempx > kCenterHeight - kBoxSize)
	{
		// ��������ϰ���,��̹�������������������. ��Ȼ̹�˺��ϰ�����м������ص���
		switch (tank_dir_)
		{
		case kDirLeft:	tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;	break;	// tank_x_ �� tempx ֮���Խ�˸���, ��̹�˷ŵ�tank_x_���ڵĸ�������
		case kDirUp:	tank_y_ = (tank_y_ / kBoxSize) * kBoxSize;	break;
		case kDirRight: tank_x_ = (tempx / kBoxSize) * kBoxSize;	break;
		case kDirDown:	tank_y_ = (tempy / kBoxSize) * kBoxSize;	break;
		default:													break;
		}
		return false;
	}
	// ת�����ص����ڵ� xy[26][26] �±�
	int index_i = tempy / kBoxSize;
	int index_j = tempx / kBoxSize;

	// �ĸ�����̹�����ĵ������Ҫ�������� 8*8 ���ӵ��±�ƫ����
	int dev[4][2][2] = { {{-1,-1},{0,-1}},  {{-1,-1},{-1,0}},  {{-1,1},{0,1}}, { {1,-1},{1,0}} };

	// 8*8 �ϰ�����Ӽ��
	int temp1 = box_mark_->box_8[index_i + dev[tank_dir_][0][0]][index_j + dev[tank_dir_][0][1]];
	int temp2 = box_mark_->box_8[index_i + dev[tank_dir_][1][0]][index_j + dev[tank_dir_][1][1]];

	// prop_8���߸��Ӽ��
	int curi = tank_y_ / kBoxSize;	// ��ǰ̹�����ڵ�����, ������һ��������, �����жϵ���
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

	// ���̹�� 4*4 ����
	// �ĸ�������Ҫ�������� 4*4 �ĸ�����̹���������� 4*4 ���ӵ��±�ƫ����
	int  dev_4[4][4][2] = { {{-2,-2},{1,-2},{-1,-2},{0,-2}}, {{-2,-2},{-2,1},{-2,-1},{-2,0}},
							{{-2,2},{1,2},{-1,2},{0,2}}, {{2,-2},{2,1},{2,-1},{2,0}} };
	// ת���� [52][52] �±�
	int index_4i = tempy / kSmallBoxSize;
	int index_4j = tempx / kSmallBoxSize;

	// -1, 0, 1, 2 �������ƶ�
	bool tank1 = box_mark_->box_4[index_4i + dev_4[tank_dir_][0][0]][index_4j + dev_4[tank_dir_][0][1]] <= kElementIce;
	bool tank2 = box_mark_->box_4[index_4i + dev_4[tank_dir_][1][0]][index_4j + dev_4[tank_dir_][1][1]] <= kElementIce;
	bool tank3 = box_mark_->box_4[index_4i + dev_4[tank_dir_][2][0]][index_4j + dev_4[tank_dir_][2][1]] <= kElementIce;
	bool tank4 = box_mark_->box_4[index_4i + dev_4[tank_dir_][3][0]][index_4j + dev_4[tank_dir_][3][1]] <= kElementIce;

	// �����ϰ����������
	if (temp1 > 2 || temp2 > 2)
	{
		// ��������ϰ���,��̹�������������������. ��Ȼ̹�˺��ϰ�����м������ص���
		switch (tank_dir_)
		{
		case kDirLeft:	tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;	break;	// tank_x_ �� tempx ֮���Խ�˸���, ��̹�˷ŵ�tank_x_���ڵĸ�������
		case kDirUp:	tank_y_ = (tank_y_ / kBoxSize) * kBoxSize;	break;
		case kDirRight: tank_x_ = (tempx / kBoxSize) * kBoxSize;		break;
		case kDirDown:	tank_y_ = (tempy / kBoxSize) * kBoxSize;		break;
		default:													break;
		}
		return false;
	}
	// ������Ҳ��õ���
	else if (!tank1 || !tank2 || !tank3 || !tank4)
	{
		return false;
	}

	//. ����ǵ�һ�ν��� kElementIce ����
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

// �����ӵ�
bool PlayerBase::ShootBullet(int bullet_id)
{
	switch (bullet_id)
	{
	case 0:
		// 1���ӵ�����ʧ��, ��ըδ���ǰ���ܷ���
		if (bullet_[0].x != kShootTableX || bombs_[0].is_show_bomb == true)
		{
			return false;
		}

		// �ӵ����������
		bullet_[0].x = (int)(tank_x_ + Bullet::devto_tank[tank_dir_][0]);
		bullet_[0].y = (int)(tank_y_ + Bullet::devto_tank[tank_dir_][1]);
		bullet_[0].direction = tank_dir_;
		bullet1_counter_ = 6;

		SignBullet(bullet_[0].x, bullet_[0].y, bullet_[0].direction, kSignPlayerBullet + player_id_ * 10 + bullet_id);
		return true;

	case 1:
		// 2 ���ӵ�����ʧ��
		if (player_tank_level_ < 2 || bullet_[1].x != kShootTableX || bullet1_counter_ > 0 || bombs_[1].is_show_bomb == true)
		{
			return false;
		}

		// �ӵ����������
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

	// �ӵ�ͷ�Ӵ����ϰ�����Ǹ���, ���ҷ��������, ���·��������
	int bombx = bullet_[i].x + Bullet::devto_head[dir][0];
	int bomby = bullet_[i].y + Bullet::devto_head[dir][1];

	bool flag = false;
	int adjust_x = 0, adjust_y = 0;		// ��������ͼƬ��ʾ������

	// ������ bombx ���� bullet_[i].x,����Ḳ���ϰ���ļ��
	if (bullet_[i].x < 0 && bullet_[i].direction == kDirLeft)
	{
		flag = true;
		adjust_x = 5;					// ����ըͼƬ������һ��
	}
	else if (bullet_[i].y < 0 && bullet_[i].direction == kDirUp)
	{
		flag = true;
		adjust_y = 5;
	}

	// �����ȥ�ӵ��Ŀ� 4, ��Ȼ�ӵ�Խ��, �����⵼�� box_8 �±�Խ��
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
		// �趨��ը����, ������ը�������ڵĸ���,���һ�����ƫ��һ������֮���..
		bullet_[i].x = kShootTableX;
		bombs_[i].is_show_bomb = true;
		bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
		bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
		bombs_[i].counter = 0;

		MciSound::PlaySounds(kSoundBin);

		return BulletShootKind::kOther;
	}

	int tempi, tempj;

	// �������� 8*8 ���ӵ�����
	int b8i = bomby / kBoxSize;
	int b8j = bombx / kBoxSize;

	// ������ת���� 4*4 ��������
	int b4i = bomby / kSmallBoxSize;
	int b4j = bombx / kSmallBoxSize;

	// �����������һ������ӵ�
	if (box_mark_->bullet_4[b4i][b4j] == kSignPlayerBullet + (1 - player_id_) * 10 + 0 ||
		box_mark_->bullet_4[b4i][b4j] == kSignPlayerBullet + (1 - player_id_) * 10 + 1 ||
		box_mark_->bullet_4[b4i][b4j] == kSignEnemyBullet)
	{
		bullet_[i].x = kShootTableX;
		box_mark_->bullet_4[b4i][b4j] = kSignWaitUnsign;		// �ȱ���м�ֵ, �ȴ������е��ӵ������ñ��
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
		// ������Ӻ���һ��
		int temp[2][2] = { {0, 0}, {-1, 0} };
		for (int n = 0; n < 2; n++)
		{
			// 8*8 ����, �ж��Ƿ���ел�
			tempi = b8i + temp[n][0];
			tempj = b8j + temp[n][1];
			if (box_mark_->box_8[tempi][tempj] == kSignCamp)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].counter = 0;
				SignBox_8(13 * kBoxSize, 25 * kBoxSize, kElementEmpty);

				return BulletShootKind::kCamp;
			}

			// ���Ҽ���ӵ�ͷ���ڵ�4*4���Ӻ����������ڵ��Ǹ�
			// ��� 4*4 ����, �ɴ��ж��ϰ���
			tempi = b4i + temp[n][0];
			tempj = b4j + temp[n][1];
			if (box_mark_->box_4[tempi][tempj] == kElementWall || box_mark_->box_4[tempi][tempj] == kElementStone)
			{
				// �趨��ը����, ������ը�������ڵĸ���,���һ�����ƫ��һ������֮���..
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// ָʾ i bomb ��ը
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;
				ClearWallOrStone(i, bombx, bomby);
				return BulletShootKind::kOther;
			}
			else if (box_mark_->box_4[tempi][tempj] >= kSignEnemy /*&& box_mark_->box_4[tempi][tempj] < kSignEnemy + TOTAL_ENEMY_NUMBER*/)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// ָʾ i bomb ��ը
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;

				// ��ǻ����˵л��� id
				bullet_[i].killed_id = box_mark_->box_4[tempi][tempj];
				return BulletShootKind::kOther;
			}
			else if (box_mark_->box_4[tempi][tempj] == kSignPlayer && player_id_ != 0 || box_mark_->box_4[tempi][tempj] == kSignPlayer + 1 && player_id_ != 1)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// ָʾ i bomb ��ը
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;
				return (BulletShootKind)box_mark_->box_4[tempi][tempj];
			}
		}
	}
	break;

	// ����ֻ����������ڵ���������
	case kDirUp:
	case kDirDown:
	{
		// ������Ӻ������һ������
		int temp[2][2] = { { 0, 0 },{ 0, -1 } };
		for (int n = 0; n < 2; n++)
		{
			// 8*8 ����, �ж��Ƿ���ел�
			tempi = b8i + temp[n][0];
			tempj = b8j + temp[n][1];

			if (box_mark_->box_8[tempi][tempj] == kSignCamp)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].counter = 0;
				SignBox_8(13 * kBoxSize, 25 * kBoxSize, kElementEmpty);
				return BulletShootKind::kCamp;
			}

			// ��� 4*4 �Ƿ�����ϰ�
			tempi = b4i + temp[n][0];
			tempj = b4j + temp[n][1];
			if (box_mark_->box_4[tempi][tempj] == kElementWall || box_mark_->box_4[tempi][tempj] == kElementStone)
			{
				// �趨��ը����, ������ը�������ڵĸ���,���һ�����ƫ��һ������֮���..
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// ָʾ i bomb ��ը
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;
				ClearWallOrStone(i, bombx, bomby);
				return BulletShootKind::kOther;
			}
			else if (box_mark_->box_4[tempi][tempj] >= kSignEnemy/* && box_mark_->box_4[tempi][tempj] < kSignEnemy + TOTAL_ENEMY_NUMBER*/)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// ָʾ i bomb ��ը
				bombs_[i].bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][0]) * kSmallBoxSize;
				bombs_[i].bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_[i].direction][1]) * kSmallBoxSize;
				bombs_[i].counter = 0;

				// ��ǻ����˵л��� id
				bullet_[i].killed_id = box_mark_->box_4[tempi][tempj];
				return BulletShootKind::kOther;
			}
			else if (box_mark_->box_4[tempi][tempj] == kSignPlayer && player_id_ != 0 || box_mark_->box_4[tempi][tempj] == kSignPlayer + 1 && player_id_ != 1)
			{
				bullet_[i].x = kShootTableX;
				bombs_[i].is_show_bomb = true;				// ָʾ i bomb ��ը
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

// �ӵ������ϰ��ﱬը���øú���, ���б߽粻�ɵ���, �±��Խ��[52][52]
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
		bool bin_once = false;		// ���ѭ����ֻ����һ������

		// ��ͬһֱ�����ڵ��ĸ� 4*4 ����, ˳���ܱ�, �����õ��±��ж�
		int temp[4][2] = { { -2, 0 },{ -1, 0 },{ 0, 0 },{ 1, 0 } };
		for (int i = 0; i < 4; i++)
		{
			tempx = boxi + temp[i][0];
			tempy = boxj + temp[i][1];

			// ת�� tempx,tempy���ڵ� 8*8 ��������
			int n = tempx / 2;
			int m = tempy / 2;

			if (box_mark_->box_4[tempx][tempy] == kElementWall)
			{
				box_mark_->box_4[tempx][tempy] = kElementClear;

				// ��� 8*8 �����ڵ�4�� 4*4 ��С�����Ƿ�ȫ�������,
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
			else if (player_tank_level_ == 3 && box_mark_->box_4[tempx][tempy] == kElementStone && i % 2 == 0)	// %2==0 ��ֹ����ѭ���ж�
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
		bool bin_once = false;		// ���ѭ����ֻ����һ������

		// ���ڵ��ĸ� 4*4 ����, ˳���ܱ�, �����õ��±��ж�
		int temp[4][2] = { {0, -2}, {0, -1}, {0, 0}, {0, 1} };
		for (int i = 0; i < 4; i++)
		{
			tempx = boxi + temp[i][0];
			tempy = boxj + temp[i][1];

			// ת�� tempx,tempy���ڵ� 8*8 ��������
			int n = tempx / 2;
			int m = tempy / 2;

			if (box_mark_->box_4[tempx][tempy] == kElementWall)
			{
				box_mark_->box_4[tempx][tempy] = kElementClear;

				// ��� 8*8 �����ڵ�4�� 4*4 ��С�����Ƿ�ȫ�������,
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
			else if (player_tank_level_ == 3 && box_mark_->box_4[tempx][tempy] == kElementStone && i % 2 == 0)	// %2==0 ��ֹ����ѭ���ж�
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

// ������ 16 * 16 ���ĵ���������, ��̹������������ͬ
void PlayerBase::SignBox_8(int x, int y, int val)
{
	// ��̹����������ת�����Ͻ��Ǹ��� ��������
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

// ����̹����������, ���16�� 4*4 ����
void PlayerBase::SignBox_4(int cx, int cy, int val)
{
	// ���ҵ��� cs,cy ��ռ�ݰٷֱ����� 16 �� 4*4 �ĸ�������
	if (tank_dir_ == kDirLeft || tank_dir_ == kDirRight)
	{
		if (cx > (cx / kSmallBoxSize) * kSmallBoxSize + kSmallBoxSize / 2)	// ����ǿ����ұ߽ڵ�, 
		{
			cx = (cx / kSmallBoxSize + 1) * kSmallBoxSize;
		}
		else
		{
			cx = (cx / kSmallBoxSize) * kSmallBoxSize;					// �����������ϵ���߽ڵ�
		}
	}
	// ����
	else
	{
		if (cy > (cy / kSmallBoxSize) * kSmallBoxSize + kSmallBoxSize / 2)	// ����ǿ��������±߽ڵ�,
		{
			cy = (cy / kSmallBoxSize + 1) * kSmallBoxSize;
		}
		else
		{
			cy = (cy / kSmallBoxSize) * kSmallBoxSize;					// �����������ϵ��ϱ߽ڵ�
		}
	}

	// ��̹����������ת�����Ͻ��Ǹ��� ��������
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

// ���4��8*8 ����, ���������ϽǸ�������
bool PlayerBase::CheckBox_8(int iy, int jx)
{
	for (int i = iy; i < iy + 2; i++)
	{
		for (int j = jx; j < jx + 2; j++)
		{
			if (box_mark_->box_8[i][j] < kElementRiver)		// �����һ�� 8*8 ̹�˿��Խ���
			{
				return true;
			}
		}
	}
	return false;
}

/*��ò��ӵ��߱���camp �ڲ�ʹ��*/
void PlayerBase::ProtectCamp(int val)
{
	for (int i = 23; i < 26; i++)
	{
		for (int j = 11; j < 15; j++)
		{
			if (i < 24 || j < 12 || j > 13)
			{
				box_mark_->box_8[i][j] = val;			// ����Χ�� kElementWall
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