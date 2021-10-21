#ifndef BATTLECITY_PLAYER_BASE_H_
#define BATTLECITY_PLAYER_BASE_H_

#include "tank.h"
#include "struct.h"
#include "time_clock.h"

namespace battlecity
{

// ��ҿ���
// һ�����ʵ����һ�����󣻸��Թ����Լ�������
class PlayerBase
{
public:
	PlayerBase(byte player, BoxMark *box_mark);								// player [0-1]
	~PlayerBase();

	void Init();
	void DrawPlayerTankIco(const HDC &right_panel_hdc);						// GameControl ��ѭ������
	bool ShowStar(const HDC &center_hdc);									// ���̹����˸����
	void DrawPlayerTank(const HDC &canvas_hdc);								// ������̹�˲���; GameControl ��ѭ������
	void DrawBullet(const HDC &center_hdc);									// �����ӵ�,��Ҫ���ƶ��ֿ�, ��Ȼ��ѭ���ڻ�ʧ֡, ��Ϊ��һ��ÿ�λ�ͼ�����ƶ�
	bool PlayerControl();													// ��ⰴ��; GameControl ��ѭ������

	BulletShootKind BulletMoving(const HDC &center_hdc);					// bullet_[i].x = kShootTableX �ӵ����˶�; GameControl ��ѭ������
	void Bombing(const HDC &center_hdc);									// �������Ƿ���ʾ��ըЧ���� ����ӵ����� �ĺ�������; GameControl ��ѭ������
	void GetKillEnemy(int &bullet1, int &bullet2);							// GameControl �ڵ���, ͨ��������ȡ�����еĵл� id; bullet1 : 1 ���ӵ����ел��� id, bullet2 : 2 ���ӵ����ел��� id; 

	void BeKilled();														// BeKilled() ���Ʊ�ըͼ��ʾ; GameControl ��ѭ������ EnemyBase->GetId() ��ȡ��������ҵ� id,Ȼ����ö�Ӧ��ҵ� BeKilled ����; 

	bool Blasting(const HDC &center_hdc);									// GameControl ��ѭ������; BeKill �����ú�����һ�� flag = true,�ú��������� flag Ȼ����ʾ��ըͼ,���� true ��������ù�

	const int &GetID();
	static bool IsGetTimeProp();											// GameControl �ڵ���, �������Ƿ���ʱ�����
	static bool IsGetShvelProp();											// ���ӵ���ѭ��
	bool IsGetBombProp();

	static void ShowProp(const HDC &center_hdc);							// �� gameControl ��ѭ������ ��Ⲣ��ʾ��˸����
	bool ShowScorePanel(const HDC &image_hdc);								// GameCnotrol ��ѭ������, �����ڵ��ó�ԱScorePanel ������ʾ���
	void SetPause();														// ���������ͣ�ƶ�; GameControl �ڼ���ӵ����н���жϵ���
	static void SetShowProp();												// ����ɹ�������ߵл�, ����ʾ����; GameControl �� CheckKillEnemy() �ڵ���
	void AddKillEnemyNum(byte enemy_level);									// ��¼�������ͬ�������͵ĵл�������; GameControl ��CheckKillEnemy() �ڵ���
	void ResetScorePanelData(const int &player_num, const int &stage);		// GameControl ��ÿһ�ؽ�����ʾ��������ʱ�����
	bool IsLifeEnd();														// ʤ��������һ��ʱ���, ��������Ҳ����³�ʼ��, ��һ�ز������
	void CheckShowGameOver(const HDC &center_hdc);							// GameCOntrol ��ѭ������, �ж��������ֵ, ��ʾGAMEOVER ����
	void SetShowGameover();													// GameControl ����� blasting ���������

private:
	void SignBullet(int left_x, int top_y, byte dir, int val);				// ����ӵ�ͷ���ڵ�һ�� 4*4 ����, �������ӵ�ͼƬ���Ͻ�����	
	void Reborn();															// ������������³���
	void DispatchProp(int prop_kind);										// ��ȡ������, ̹���ƶ��ڼ�����	
	void Move(int new_dir);													// �жϲ����ķ�������ƶ�,ͬʱ�������굽����������; PlayerControl() �ڵ���
	bool CheckMoveable();													// ��⵱ǰ�����ƶ��Ŀ�����; Move() �ڵ���
	bool ShootBullet(int bullet_id);										// ���� id ���ӵ�[0,1],���� J �� 1 ������; PlayerControl() �ڵ���
	BulletShootKind CheckBomb(int i);										// ����ӵ��ƶ������Ƿ���ж���; BulletMoving() �ڵ���
	void ClearWallOrStone(int id, int x, int y);							// id:�ӵ�id; x,y:�ӵ�͹���������, ���ݲ�ͬ����λ�ò�һ��; CheckBomb() �ڻ����ϰ���ʱ����
	void SignBox_8(int x, int y, int val);									// ��� 4 �� 8*8 �ĸ���; xx,y : 16*16 ���ӵ����ĵ�����;val : ��Ҫ��ǵ�ֵ	
	void SignBox_4(int cx, int cy, int val);								// ����̹����������, ���16�� 4*4 ����
	static bool CheckBox_8(int iy, int jx);									// ������ [26][26] ���±�����
	static void ProtectCamp(int val);

public:
	static std::list<PlayerBase *> *player_list_;							// ��GameControl�� ��ֵ

private:
	bool is_died_;															// �����Ƿ�����,����
	byte player_id_ : 1;													    // [0-1] ���
	PlayerTank *player_tank_;												// ̹����
	static BoxMark *box_mark_;

	IMAGE p12_image_;														// 1P\2P ͼ��
	int p12_image_x_;														// ͼ������
	int p12_image_y_;

	IMAGE player_tank_ico_image_;											// ���̹��ͼ��
	int player_tank_ico_image_x_;											// ͼ������
	int player_tank_ico_image_y_;

	int player_life_;														// �������, Ĭ��3
	int player_life_x_; 													// ͼ������
	int	player_life_y_;

	IMAGE black_number_image_;												// 0123456789 ��ɫ����
	int tank_x_;															// ̹�����ĵ�����
	int tank_y_;
	byte player_tank_level_ : 2;											// [0-3] ̹�˼���,��õ��ߺ�����̹��
	byte tank_dir_ : 2;														// ��ǰ̹�˷���0-��,1-��,2��,3-��
	static int dev_xy_[4][2];												// �ĸ������ƫ����
	int speed_[4];															// speed_ * dev_xy_ �õ��˶��ٶ�, �±��Ӧ player_tank_level_, ��ͬ�����ٶȲ�һ��

	Bullet bullet_[2];														// �����ӵ�
	int bullet1_counter_;													// �ӵ� 1 �ļ���, �ӵ� 1 �����ú���ܷ����ӵ� 2
	bool is_moving_;														// ָʾ̹���Ƿ��ƶ�, ���ݵ� GetTankImage() ��ȡ�ƶ���̹��

	Bomb bombs_[2];															// ��ը�ṹ��
	Blast blast_;															// ̹�˱�ը�ṹ, 

	Star star_;																// ̹�˳���ǰ���Ľ�����˸
	Ring ring_;																// ����Ȧ��

	static Prop prop_;														// ������

	// GameControl ��ѭ������ֵ, Ȼ�������� EnemyBase ֹͣ�ƶ�
	static bool get_time_prop_;												// ��¼�Ƿ��� ʱ�ӵ���
	bool get_bomb_prop_;													// ���׵���, �߼��� get_time_prop_ ��ͬ
	static bool get_shovel_prop_;											// ����Ƿ�ӵ�в��ӵ���
	static int shovel_prop_counter_;

	TimeClock tank_timer_;													// ̹���ƶ���ʱ��
	TimeClock bullet_timer_;												// �ӵ��ƶ��ٶ�
	TimeClock bomb_timer_;													// �ӵ���ը�ٶ�

	ScorePanel *score_panel_;												// �ؿ���������ʾ�������

	int kill_enemy_number_[4];												// ���ֵл�����,ɱ������

	bool is_pause_;															// ������һ����һ���, ��ͣһ��ʱ��
	int pause_counter_;														// Move() �ڼ���ֹͣ�ƶ�, DrawTank() ��ȡģ����,ʵ��̹����˸

	bool has_send_killednum_;												// has_send_kill_num_to_score_panel_; ��ֹ ʤ�����д�Ӫ������.��η������ݵ��������

	// CheckMoveable() �ڼ��; 
	// ̹���Ƿ��� _ICE ��,���� _ICE ��ʱ��������Ϊ true, ���ݸ� flag ���� mTankTimer ʱ���, �ӿ�̹���ƶ�, ���������� false, ����ԭ��ʱ���
	bool on_ice_;
	bool automove_;
	int automove_counter_;
	int rand_counter_;														// ÿ���Զ��ƶ��������

	PlayerGameover player_gameover_;										// ��ұ�������ʧ gameover ����

	static int move_speed_dev_[4];											// �ĸ�����̹���ƶ�ʱ����
	static int bullet_speed_dev_[4];										// ��ͬ�����ӵ�ʱ�����ٶ�
};
} // namespace battlecity


#endif