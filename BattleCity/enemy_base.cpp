#include "stdafx.h"
#include "enemy_base.h"
#include "mci_sound.h"

namespace battlecity
{
//////////////////////////////////////////////////////////////////////////

// �л�̹�˿���

bool EnemyBase::is_pause_ = false;
int EnemyBase::dev_xy_[4][2] = { { -1, 0 },{ 0, -1 },{ 1, 0 },{ 0, 1 } };	// ������������

// ����ĳ����𼶱�ĵл�
EnemyBase::EnemyBase(TankKind kind, byte level, BoxMark *box_mark)
{
	enemy_tank_kind_ = kind;
	enemy_tank_level_ = level;
	is_died_ = false;
	box_mark_ = box_mark;

	int temp_x[3] = { kBoxSize, 13 * kBoxSize, 25 * kBoxSize };	// ̹��������ֵ�����λ�� x ����
	tank_x_ = temp_x[rand() % 3];
	tank_y_ = kBoxSize;
	tank_dir_ = kDirDown;
	tank_image_index_ = 0;

	step_numbers_ = rand() % 200;					// ��ǰ��������ƶ��Ĳ���

	// ��ͬ����̹���ƶ��ٶ�ϵ��
	int temp[4] = { 1, 1, 1, 1 };
	for (int i = 0; i < 4; i++)
	{
		speed_[i] = temp[i];
	}

	// �ӵ���ʼ��; ���������� PlayerBase ������
	bullet_.x = kShootTableX;
	bullet_.y = -100;
	bullet_.direction = kDirDown;
	for (int i = 0; i < 4; i++)
	{
		bullet_.speed[i] = 3;		// ���ܳ��� 4
	}
	bullet_.killed_id = 0;			// ��¼�������̹�˵�id

	shoot_counter_ = rand() % 100 + 100;	// �����������ӵ�

	// ��ըͼƬ
	bombs_.bomb_x = -100;
	bombs_.bomb_y = -100;
	bombs_.is_show_bomb = false;
	bombs_.counter = 0;

	// ��ͬ����л��ƶ�ʱ����
	int move_level[4] = { 30, 15, 27, 25 };
	tank_timer_.SetDrtTime(move_level[enemy_tank_level_]);

	// �ӵ��ƶ�ʱ����
	bullet_timer_.SetDrtTime(30);

	// �����ӵ�Ƶ��
	shoot_timer_.SetDrtTime(rand() % 1000 + 700);

	// �ӵ���ը�ٶ�
	bomb_timer_.SetDrtTime(37);

	// ̹�˱�ը����
	//mBlastTimer.SetDrtTime(37);

	// ���û�ͷ���Ƶ��
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

// ��ʾ̹��
StarState EnemyBase::ShowStar(const HDC &center_hdc, int &remain_number)
{
	StarState result = star_.EnemyShowStar(center_hdc, tank_x_, tank_y_, box_mark_);
	switch (result)
	{
		// ��ǰ���ڼ�ʱ, δ��ʾ
	case StarState::kStarTiming:
		break;

		// ��ǰλ����ʾ�Ľ���ʧ��
	case StarState::kStarFailed:
	{
		// ����ѡ�����λ��
		int tempx[3] = { kBoxSize, 13 * kBoxSize, 25 * kBoxSize };
		tank_x_ = tempx[rand() % 3];
	}
	break;

	// �Ľ��ǿ�ʼ����
	case StarState::kStarOut:
		SignBox_4(tank_x_, tank_y_, kSignStar);	// ���Ϊ kSignStar = 2000, 2000 ����̹�˲��ܴ��еı�־
		break;

		// �Ľ������ڳ���
	case StarState::kStarShowing:
		break;

		// �Ľ���ֹͣ
	case StarState::kStarStop:
		// �Ľ�����ʧ. �л�����, ʣ��̹����-1;
		enemy_id_ = kTotalEnemyNumber - remain_number;
		remain_number -= 1;

		SignBox_4(tank_x_, tank_y_, kSignEnemy + 1000 * enemy_tank_level_ + 100 * enemy_tank_kind_ + enemy_id_);		// ̹�˳���, ���Ľ��Ǳ�Ǹ�Ϊ̹�˱��
		break;

		// ̹���Ѿ�����
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

	// �ƶ�ǰȡ�����
	SignBox_4(tank_x_, tank_y_, kElementEmpty);

	// �ڲ���ʱ, һ��ʱ����ͷ���
	ShootBack();

	// �ض���
	if (step_numbers_-- < 0)
	{
		RejustDirPosition();
	}

	// ���ƶ�
	if (CheckMoveable())
	{
		tank_x_ += dev_xy_[tank_dir_][0] * speed_[enemy_tank_level_];
		tank_y_ += dev_xy_[tank_dir_][1] * speed_[enemy_tank_level_];
	}
	else // �����ƶ�,�ض���
	{
		RejustDirPosition();
	}

	// ����λ�����±��
	SignBox_4(tank_x_, tank_y_, kSignEnemy + enemy_tank_level_ * 1000 + enemy_tank_kind_ * 100 + enemy_id_);
}

// 
void EnemyBase::DrawBullet(const HDC &center_hdc)
{
	// ����ӵ�û���ƶ����ߵл�����
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

	// �ӵ����������
	bullet_.x = tank_x_ + Bullet::devto_tank[tank_dir_][0];
	bullet_.y = tank_y_ + Bullet::devto_tank[tank_dir_][1];
	bullet_.direction = tank_dir_;

	SignBullet(bullet_.x, bullet_.y, bullet_.direction, kSignEnemyBullet);
	return true;
}

//
BulletShootKind EnemyBase::BulletMoving()
{
	// ����ӵ�û���ƶ����ߵл�����
	if (bullet_.x == kShootTableX/* || is_died_*/ || !bullet_timer_.IsTimeOut())
	{
		return BulletShootKind::kNone;
	}

	// ����ӵ��ڱ�ը
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

// ��Gamecontrol�ڼ��, Ȼ����� BigestTank.class ��Ҫ�����������,��Ϊ����Ҫ����Ĵβ���ɱ��
bool EnemyBase::BeKilled(bool kill_anyway)
{
	// ����л���û�г���
	if (star_.IsStop() == false || blast_.IsBlasting() || is_died_ == true)
	{
		return false;
	}

	MciSound::PlaySounds(kSoundEnemyBomb);
	is_died_ = true;
	SignBox_4(tank_x_, tank_y_, kElementEmpty);

	// ���ÿ�ʼ��ը����
	blast_.SetBlasting(tank_x_, tank_y_);

	return true;
}

// ��ʾ̹�˱�ըЧ��, GameControl ��ѭ�����
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

// x,y �� 16*16 ���ĵ�����
void EnemyBase::SignBox_8(int x, int y, int value)
{
	// ��̹����������ת�����Ͻ��Ǹ��� ��������
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

// ��ǻ�ȡ��̹�����ڵ� 4*4 = 16 ������
void EnemyBase::SignBox_4(int x, int y, int value)
{
	// ���ҵ��� x,y ��ռ�ݰٷֱ����� 16 �� 4*4 �ĸ�������
	if (tank_dir_ == kDirLeft || tank_dir_ == kDirRight)
	{
		if (x > (x / kSmallBoxSize) * kSmallBoxSize + kSmallBoxSize / 2)	// ����ǿ����ұ߽ڵ�, 
		{
			x = (x / kSmallBoxSize + 1) * kSmallBoxSize;
		}
		else 
		{
			x = (x / kSmallBoxSize) * kSmallBoxSize;					// �����������ϵ���߽ڵ�
		}
	}
	// ����
	else
	{
		if (y > (y / kSmallBoxSize) * kSmallBoxSize + kSmallBoxSize / 2)	// ����ǿ��������±߽ڵ�
		{
			y = (y / kSmallBoxSize + 1) * kSmallBoxSize;
		}
		else
		{
			y = (y / kSmallBoxSize) * kSmallBoxSize;					// �����������ϵ��ϱ߽ڵ�
		}
	}

	// ��̹����������ת�����Ͻ��Ǹ��� ��������
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
	// ת����ͷ����
	int hx = left_x + Bullet::devto_head[dir][0];
	int hy = top_y + Bullet::devto_head[dir][1];

	// ת���� 4*4 �����±�����
	int b4i = hy / kSmallBoxSize;
	int b4j = hx / kSmallBoxSize;
	if (b4i > 51 || b4j > 51 || b4i < 0 || b4j < 0)
	{
		//printf("��ˮ��%d, %d\n", b4i, b4j);
		return;
	}

	box_mark_->bullet_4[b4i][b4j] = val;
}

// ���ĳ��16*16λ�ÿ��Է�̹����, x,y 16*16�����ĵ�
bool EnemyBase::CheckBox_8()
{
	// ��ȡ̹�����Ͻǵ� 4*4 �±� 
	int iy = tank_y_ / kSmallBoxSize - 2;
	int jx = tank_x_ / kSmallBoxSize - 2;
	for (int i = iy; i < iy + 4; i++)
	{
		for (int j = jx; j < jx + 4; j++)
		{
			// ����Ľ���, ���,�л�,
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
	// ̹����������
	int tempx = tank_x_ + dev_xy_[tank_dir_][0] * speed_[enemy_tank_level_];
	int tempy = tank_y_ + dev_xy_[tank_dir_][1] * speed_[enemy_tank_level_];

	if (tempx < kBoxSize || tempy < kBoxSize || tempy > kCenterWidth - kBoxSize || tempx > kCenterHeight - kBoxSize)
	{
		// ��������ϰ���,��̹�������������������. ��Ȼ̹�˺��ϰ�����м������ص���
		switch (tank_dir_)
		{
		case kDirLeft:	
			tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;	
			break;	// tank_x_ �� tempx ֮���Խ�˸���, ��̹�˷ŵ�tank_x_���ڵĸ�������
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

	// ת�����ص����ڵ� xy[26][26] �±�
	int index_i = (int)tempy / kBoxSize;
	int index_j = (int)tempx / kBoxSize;

	// ��� 8*8 �ϰ���
	int dev[4][2][2] = { { { -1,-1 },{ 0,-1 } },{ { -1,-1 },{ -1,0 } },{ { -1,1 },{ 0,1 } },{ { 1,-1 },{ 1,0 } } };

	// ���̹�� 4*4 ����
	// �ĸ�������Ҫ�������� 4*4 �ĸ�����̹���������� 4*4 ���ӵ��±�ƫ����
	int  dev_4[4][4][2] = { { { -2,-2 },{ 1,-2 },{ -1,-2 },{ 0,-2 } },{ { -2,-2 },{ -2,1 },{ -2,-1 },{ -2,0 } },
							{ { -2, 2 },{ 1, 2 },{ -1, 2 },{ 0, 2 } },{ { 2, -2 },{ 2, 1 },{ 2, -1 },{ 2, 0 } } };

	// ת���� [52][52] �±�
	int index_4i = tempy / kSmallBoxSize;
	int index_4j = tempx / kSmallBoxSize;

	// -1, 0, 1, 2 �������ƶ�
	bool tank1 = box_mark_->box_4[index_4i + dev_4[tank_dir_][0][0]][index_4j + dev_4[tank_dir_][0][1]] <= kElementIce;
	bool tank2 = box_mark_->box_4[index_4i + dev_4[tank_dir_][1][0]][index_4j + dev_4[tank_dir_][1][1]] <= kElementIce;
	bool tank3 = box_mark_->box_4[index_4i + dev_4[tank_dir_][2][0]][index_4j + dev_4[tank_dir_][2][1]] <= kElementIce;
	bool tank4 = box_mark_->box_4[index_4i + dev_4[tank_dir_][3][0]][index_4j + dev_4[tank_dir_][3][1]] <= kElementIce;

	// �����ϰ���
	if (box_mark_->box_8[index_i + dev[tank_dir_][0][0]][index_j + dev[tank_dir_][0][1]] > 2 ||
		box_mark_->box_8[index_i + dev[tank_dir_][1][0]][index_j + dev[tank_dir_][1][1]] > 2)
	{
		// ��������ϰ���,��̹�������������������. ��Ȼ̹�˺��ϰ�����м������ص���
		switch (tank_dir_)
		{
		case kDirLeft:	
			tank_x_ = (tank_x_ / kBoxSize) * kBoxSize;	
			break;	// tank_x_ �� tempx ֮���Խ�˸���, ��̹�˷ŵ�tank_x_���ڵĸ�������
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
	// ����̹�˲��õ���
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

	/* �ض���, �������λ�ú�������÷���
	* ����̹�������ƶ��ļ��ʴ�Щ*/
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

	// �ӵ�ͷ�Ӵ����ϰ�����Ǹ���, ���ҷ��������, ���·��������
	int bombx = bullet_.x + Bullet::devto_head[dir][0];
	int bomby = bullet_.y + Bullet::devto_head[dir][1];

	bool flag = false;
	int adjust_x = 0, adjust_y = 0;		// ��������ͼƬ��ʾ������

	// ������ bombx ���� bullet_[i].x,����Ḳ���ϰ���ļ��
	if (bullet_.x < 0 && dir == kDirLeft)
	{
		flag = true;
		adjust_x = 5;					// ����ըͼƬ������һ��
	}
	else if (bullet_.y < 0 && dir == kDirUp)
	{
		flag = true;
		adjust_y = 5;
	}

	// �����ȥ�ӵ��Ŀ� 4, ��Ȼ�ӵ�Խ��, �����⵼�� box_8 �±�Խ��
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
		// �趨��ը����, ������ը�������ڵĸ���,���һ�����ƫ��һ������֮���..
		bullet_.x = kShootTableX;
		bombs_.is_show_bomb = true;
		bombs_.bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[dir][0]) * kSmallBoxSize;
		bombs_.bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[dir][1]) * kSmallBoxSize;
		bombs_.counter = 0;
		return BulletShootKind::kOther;
	}

	int tempi, tempj;

	// �������� 8*8 ���ӵ�����
	int b8i = bomby / kBoxSize;
	int b8j = bombx / kBoxSize;

	// 4*4 ��������
	int bi = bomby / kSmallBoxSize;
	int bj = bombx / kSmallBoxSize;

	// �����������ӵ�
	if (box_mark_->bullet_4[bi][bj] == kSignPlayerBullet + 0 * 10 + 0 ||
		box_mark_->bullet_4[bi][bj] == kSignPlayerBullet + 0 * 10 + 1 ||
		box_mark_->bullet_4[bi][bj] == kSignPlayerBullet + 1 * 10 + 0 ||
		box_mark_->bullet_4[bi][bj] == kSignPlayerBullet + 1 * 10 + 1)
	{
		bullet_.x = kShootTableX;
		box_mark_->bullet_4[bi][bj] = kSignWaitUnsign;		// �ȱ���м�ֵ, �ȴ������е��ӵ���⵽��ֵ��,�ٲ����ñ��
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
		// ���Ҽ���ӵ�ͷ���ڵ�4*4���Ӻ����������ڵ��Ǹ�
	case kDirLeft:
	case kDirRight:
	{
		int temp[2][2] = { { 0, 0 },{ -1, 0 } };
		for (int n = 0; n < 2; n++)
		{
			// 8*8 ����, �ж��Ƿ����
			tempi = b8i + temp[n][0];
			tempj = b8j + temp[n][1];
			if (box_mark_->box_8[tempi][tempj] == kSignCamp)
			{
				bombs_.counter = 0;
				bullet_.x = kShootTableX;
				SignBox_8(13 * kBoxSize, 25 * kBoxSize, kElementEmpty);

				return BulletShootKind::kCamp;
			}

			// 4*4 ���
			tempi = bi + temp[n][0];
			tempj = bj + temp[n][1];
			if (box_mark_->box_4[tempi][tempj] == kElementWall || box_mark_->box_4[tempi][tempj] == kElementStone)
			{
				// �趨��ը����, ������ը�������ڵĸ���,
				bullet_.x = kShootTableX;
				bombs_.is_show_bomb = true;				// ָʾ i bomb ��ը
				bombs_.bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][0]) * kSmallBoxSize;
				bombs_.bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][1]) * kSmallBoxSize;
				bombs_.counter = 0;
				ShootWhat(bombx, bomby);
				return BulletShootKind::kOther;
			}
			// 4*4 ��Ҹ���
			else if (box_mark_->box_4[tempi][tempj] == kSignPlayer || box_mark_->box_4[tempi][tempj] == kSignPlayer + 1)
			{
				bullet_.x = kShootTableX;
				bombs_.is_show_bomb = true;				// ָʾ i bomb ��ը
				bombs_.bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][0]) * kSmallBoxSize;
				bombs_.bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][1]) * kSmallBoxSize;
				bombs_.counter = 0;
				return (BulletShootKind)box_mark_->box_4[tempi][tempj];
			}
		}
	}
	break;

	// ����ֻ����������ڵ���������
	case kDirUp:
	case kDirDown:
	{
		int temp[2][2] = { { 0, 0 },{ 0, -1 } };
		for (int n = 0; n < 2; n++)
		{
			// 8*8 ����, �ж��Ƿ������
			tempi = b8i + temp[n][0];
			tempj = b8j + temp[n][1];
			if (box_mark_->box_8[tempi][tempj] == kSignCamp)
			{
				bombs_.counter = 0;
				bullet_.x = kShootTableX;
				SignBox_8(13 * kBoxSize, 25 * kBoxSize, kElementEmpty);
				return BulletShootKind::kCamp;
			}

			// 4*4 ���
			tempi = bi + temp[n][0];
			tempj = bj + temp[n][1];
			if (box_mark_->box_4[tempi][tempj] == kElementWall || box_mark_->box_4[tempi][tempj] == kElementStone)
			{
				// �趨��ը����, ������ը�������ڵĸ���
				bullet_.x = kShootTableX;
				bombs_.is_show_bomb = true;				// ָʾ i bomb ��ը
				bombs_.bomb_x = (bombx / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][0]) * kSmallBoxSize;
				bombs_.bomb_y = (bomby / kSmallBoxSize + Bullet::bomb_center_dev[bullet_.direction][1]) * kSmallBoxSize;
				bombs_.counter = 0;
				ShootWhat(bombx, bomby);
				return BulletShootKind::kOther;
			}
			// 4*4 ���С����
			else if (box_mark_->box_4[tempi][tempj] == kSignPlayer || box_mark_->box_4[tempi][tempj] == kSignPlayer + 1)
			{
				bullet_.x = kShootTableX;
				bombs_.is_show_bomb = true;				// ָʾ i bomb ��ը
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
		// ���ڵ��ĸ� 4*4 ����, ˳���ܱ�, �����õ��±��ж�
		int temp[4][2] = { { -2, 0 },{ -1, 0 },{ 0, 0 },{ 1, 0 } };
		for (int i = 0; i < 4; i++)
		{
			tempx = boxi + temp[i][0];
			tempy = boxj + temp[i][1];
			if (box_mark_->box_4[tempx][tempy] == kElementWall)
			{
				box_mark_->box_4[tempx][tempy] = kElementClear;
			}

			// ת�� tempx,tempy���ڵ� 8*8 ��������
			int n = tempx / 2;
			int m = tempy / 2;

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
	}
	break;

	case kDirUp:
	case kDirDown:
	{
		// ���ڵ��ĸ� 4*4 ����, ˳���ܱ�, �����õ��±��ж�
		int temp[4][2] = { { 0, -2 },{ 0, -1 },{ 0, 0 },{ 0, 1 } };
		for (int i = 0; i < 4; i++)
		{
			tempx = boxi + temp[i][0];
			tempy = boxj + temp[i][1];
			if (box_mark_->box_4[tempx][tempy] == kElementWall)
			{
				box_mark_->box_4[tempx][tempy] = kElementClear;
			}

			// ת�� tempx,tempy���ڵ� 8*8 ��������
			int n = tempx / 2;
			int m = tempy / 2;

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

			// 4 �� 4*4 ��ɵ� 8*8 ���ӱ������
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
* ���õл���ͷ�˶���С��һ�ξ���
*/
void EnemyBase::ShootBack()
{
	if (!shoot_back_timer_.IsTimeOut())
	{
		return;
	}

	int back_dir[4] = { kDirRight, kDirDown, kDirLeft, kDirUp };

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
	// ����ֱ�� delete[] mTank??
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

	// ����̹�˺���̹ͨ�˱�ɫ����
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
		printf("����!. EnemyBase.cpp");
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