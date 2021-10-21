#ifndef BATTLECITY_ENEMY_BASE_H_
#define BATTLECITY_ENEMY_BASE_H_

#include "tank.h"
#include "struct.h"
#include "time_clock.h"

namespace battlecity
{

// �л�̹��
// һ���л�ʵ����һ������; �ṩ�л����(�Ƿ��ǵ���̹��), �л�����[0-3];
// GameControl �ڵл�������, ���Ὣ��� EnemyList ���Ƴ�, ��Ϊ�л��ӵ����ܻ������˶�;��Ϸ�������������ʾ����ͷŵл�������Դ
class EnemyBase
{
public:
	EnemyBase(TankKind kind, byte level, BoxMark *box_mark);	// kind[0-1]; level [0-4]
	virtual ~EnemyBase();

	StarState ShowStar(const HDC &center_hdc, int &remain_number);// ��ʾ��˸�Ľ���, true-������ʾ, false-��ʾ���
	void TankMoving(const HDC &center_hdc);						// �л��ƶ�
	virtual void DrawTank(const HDC &center_hdc);				// ������̹��, ���า����
	void DrawBullet(const HDC &center_hdc);						// �����ӵ�, ��Ҫ���ӵ��ƶ��ֿ�,
	bool ShootBullet();											// �����ӵ�
	BulletShootKind BulletMoving();								// �ӵ��ƶ�
	void Bombing(const HDC &center_hdc);
	virtual bool BeKilled(bool kill_anyway);					// �л�������, ��� SignBox ���, ������ʾ��һ�õ���ֱ��kill��BigestTank
	bool Blasting(const HDC &center_hdc);						// ��ʾ̹�˱�ըͼ, true ��ʶ��ը��,GameControl ��ȡ����ֵȻ�󽫸õл�ɾ��

	
	static void SetPause(bool);									// �� GameControl ������, ���� mPause, Ȼ�� ShootBullet() ���ֹͣ�����ӵ�

	int GetId();												// ���صл� id
	TankKind GetKind();											// ���صл�����, �Ƿ��ǵ���̹��
	byte GetLevel();

private:
	void SignBox_8(int x, int y, int value);
	void SignBox_4(int x, int y, int value);					// ��ǻ�ȡ�� 4*4 ��С�ĸ���Ϊ̹��;
		
	void SignBullet(int left_x, int top_y, byte dir, int val);	// ����ӵ�ͷ���ڵ�һ�� 4 * 4 ����; �������ӵ�ͼƬ���Ͻ�����

	bool CheckBox_8();											// ���ĳ�� box_8 �Ƿ���Է���̹��, ������ 16*16 ���ӵ����ĵ�, ��̹�����������ͬ
	bool CheckMoveable();
	void RejustDirPosition();									// ���¶�λ̹�˷���, ����̹��λ��, �����ڸ�����
	BulletShootKind CheckBomb();								// �ƶ��ӵ����ӵ���ײ��⣩
	void ShootWhat(int, int);									// ������к���

	void ShootBack();											// ��ͷ���; ��TankMoving()�ڵ���

protected:
	int enemy_id_;												// ����л���л�
	TankKind enemy_tank_kind_;									// �л����, ����̹�˺���̹ͨ������, [0-1]
	byte enemy_tank_level_ : 2;									// �л�̹��4������ [0-3]
	bool is_died_;												// �Ƿ񱻱�����, �����к�����Ϊ true, �л�����ֵ�����ƶ�
	BoxMark *box_mark_;											// ָ����ӱ�ǽṹ, �� GameControl ���ݽ���

	int tank_x_; 												// ̹������, ̹�˵����ĵ�
	int tank_y_;
	byte tank_dir_ : 2;											// ̹�˷���
	byte tank_image_index_ : 6;									// ̹���ƶ��л�ͼƬ
	int step_numbers_;											// ��ǰ�����ƶ��Ĳ���, һ����������������ϰ���任�������¼���;
	static int dev_xy_[4][2];									// �ĸ������ƫ����
	int speed_[4];												// speed_ * dev_xy_ �õ��˶��ٶ�, �±��Ӧ player_tank_level_, ��ͬ�����ٶȲ�һ��

	static bool is_pause_;										// �� GameControl ����, Ȼ���ڷ����ʱ�������ֵ, ����л�����ͣ��ֹͣ�����ӵ�

	Bullet bullet_;
	int shoot_counter_;											// ����һ�������, �������������ӵ�

	Bomb bombs_;												// ��ը�ṹ��
	Blast blast_;												// ̹�˱�ը�ṹ
	Star star_;													// �Ľ�����˸��

	TimeClock tank_timer_;										// �л��ƶ��ٶ�
	TimeClock bullet_timer_;									// �ӵ��ƶ��ٶ�
	TimeClock shoot_timer_;										// �����ӵ�Ƶ��
	TimeClock bomb_timer_;										// �ӵ���ը�ٶ�

	TimeClock shoot_back_timer_;								// ̹�˻�ͷ���,Ȼ��̾����ٴα���

	IMAGE score_image_[4];										// 100\200.. ��, �л���ը�����ʾ��
};

// ǰ������̹ͨ��
class CommonTank : public EnemyBase
{
public:
	CommonTank(byte level, BoxMark *box_mark);
	~CommonTank();
	void DrawTank(const HDC &center_hdc);						// ������̹��

public:
	TankInfo *tank_info_;										// ��ɫ̹��
};

// ǰ���ֵ���̹��
class PropTank : public EnemyBase
{
public:
	PropTank(byte level, BoxMark *box_mark);
	~PropTank();
	void DrawTank(const HDC &center_hdc);						// ������̹��

public:
	TankInfo *tank_info_[2];									// �洢��ɫ�ͺ�ɫ��̹��
	byte index_counter_ : 6;									// �±�������ɫ
};

// ���������̹�� (�����Ǻ�ƻ�, ��ͨ���̻ƻ�)
class BigestTank : public EnemyBase
{
public:
	BigestTank(TankKind kind, BoxMark *box_mark);
	~BigestTank();
	void DrawTank(const HDC &center_hdc);						// ������̹��
	bool BeKilled(bool kill_anyway);

public:
	TankInfo *tank_info_[4];									// ��,���,��
	byte index_counter_ : 6;									// �±�������ɫ
	int hp_;													// Ѫ��������̹����Ҫ�����ĴβŻᱬը
};
} // namespace battlecity


#endif