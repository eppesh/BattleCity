#ifndef BATTLECITY_GAME_CONTROL_H_
#define BATTLECITY_GAME_CONTROL_H_

#include "player_base.h"
#include "enemy_base.h"
#include "time_clock.h"

namespace battlecity
{
struct Map
{
    char buf[26][27];
};

// �๦��:
// �� main ���������ڻ�ͼ�ӿ�; ����һ�� IMAGE ����, ���Ƶ�ͼ/�л��б���Ϣ/����/�ؿ�;
// �洢���ӱ��; �洢�������; ������Ϸѭ��; 

// ��Ա����:
// EnemyList: �洢������� and �����ƶ� and �ȴ����ֵĵл�


enum GameResult { kVictory, kFail };		        // ÿһ��ʤ��\ʧ��

class GameControl
{
public:
    GameControl(HDC des_hdc, HDC image_hdc);
    ~GameControl();
    void Init();
    void AddPlayer(int player_num);					// ������ҽ��������
    void LoadMap();									// ��ȡ .map ��ͼ�ļ�|�������Ƶ�ͼ
    bool CreateMap(bool *out);				        // ����Զ����ͼ
    void GameLoop();
    GameResult StartGame();							// ���»�����Ϸ���ֶ���, ���� false ������Ϸ

private:
    void CutStage();		                        // ����
    void ShowStage();				                // ��ʼÿһ��֮ǰ��ʾ STAGE 2 ֮�������
    void ClearSignBox();
    void InitSignBox();				                // ��ʼ�����ӱ��
    void AddEnemy();						        // ��Ϸ��ʼǰ ���20 �ܵл�
    void SignBox_4(int i, int j, int sign_val);	    // ��Ǹ���, LoadMap() �ڵ���
    bool RefreshData();
    void RefreshRightPanel();						// ˢ���ұ���Ϣ�������, �������ݱ仯�ŵ��ø���
    void RefreshCenterPanel();						// ˢ���м���Ϸ���� 208 * 208
    void CheckKillEnemy(PlayerBase *player_base);	// �������Ƿ���ел�, ������ɾ���õл�
    void IsGameOver();				                // ѭ������Ƿ�flag,  gameover
    void IsWinOver();				                // �������ел�ʤ������

public:
    static int current_stage_;						// [1-35] ��ǰ�ؿ�, SelectPanel ��ʹ��, ������ʹ��

private:
    // ���ڻ�ͼ�ӿ�
    IMAGE center_image_;				            // ���ڱ��� mCenter_hdc ������Ч��.
    HDC des_hdc_;					                // ������hdc, main ����
    HDC image_hdc_;					                // 256*224 �� hdc, main ����
    HDC center_hdc_;				                // 208*208 �� hdc, �������, ����Ϸ����,�ֿ�,��������,����Ҳ�����趨

    BoxMark *box_mark_;			                    // �洢���ӱ��
    std::list<PlayerBase *> player_list_;
    std::list<EnemyBase *> enemy_list_;				// �л��б�

    IMAGE gray_background_image_;		            // ��Ϸ��ɫ����ͼ
    IMAGE black_background_image_;	                // ��ɫ����ͼ

    Map map_;						                // �洢��ͼ���ݽṹ
    IMAGE stone_image_;				                // ʯͷ
    IMAGE forest_image_;				            // ����
    IMAGE ice_image_;				                // ����
    IMAGE river_image_[2];			                // ����
    IMAGE wall_image_;				                // ��ǽ
    IMAGE camp_image_[2];					        // ��Ӫ

    IMAGE enemy_tank_ico_image_;		            // �л�̹��ͼ��
    IMAGE flag_image_;				                // ����

    int cut_stage_counter_;
    IMAGE current_stage_image_;		                // STAGE ����
    IMAGE black_number_image_;		                // 0123456789 ��ǰ�ؿ���
    IMAGE gameover_image_;			

    // ��ʼ mActiveEnemyTankNumber + mRemainEnemyTankNumber = 20
    int remain_enemy_tank_number_;		            // ����δ��ʾ�ĵл�����, ��ʼֵ=mBlast20, ����һ�ܵл���ֵ��һ
    int cur_moving_tank_number_;		            // ��ǰ���ֵĵл�����
    int killed_enemy_number_;				        // �Ѿ�����ĵл���

    Blast blast_;				                    // ��Ӫ��ը
    bool is_camp_die_;					            // ��Ӫ������, ���� Camp ��ͼ 

    bool is_enemy_pause_;			                // �Ƿ�����ƶ��л�
    TimeClock enemy_pause_timer_;		            // �л���ͣ���

    TimeClock main_timer_;			                // ���ƻ�ͼƵ��
    TimeClock camp_timer_;		                    // Camp ��ը�ٶ�

    int gameover_counter_;
    int gameover_x_;
    int gameover_y_;	// 
    bool gameover_flag_;			                // ��Ϸ�������, �����������, �񳲱�����
    TimeClock gameover_timer_;	                    // ��������

    IMAGE create_map_tank_image_;		            // �Զ����ͼ�е� ̹���α�ͼƬ
    int cmt_image_x_; 	                            // �α����ĵ�����
    int cmt_image_y_;

    bool has_custom_map_;			                // ��ǰ�Ƿ���������ͼ, �����˳������ٴν�������, ����֮ǰ����������
    bool show_score_panel_;		                    // �Ƿ���ʾ�������

    bool is_win_;			                        // ������л�,ʤ��
    int win_counter_;	                            // ���������������ת

    bool show_gameover_after_score_panel_;
    IMAGE msgoas_image_;                            // ��ŵ�Gameover
    int msgoas_y_;		                            // ��������
    int msgoas_counter_;
};
} // namespace battlecity

#endif