#ifndef BATTLECITY_MCI_SOUND_H_
#define BATTLECITY_MCI_SOUND_H_

namespace battlecity
{
const int kSoundStart = 0;
const int kSoundShoot0 = 100;
const int kSoundShoot1 = 101;
const int kSoundShoot2 = 102;
const int kSoundShoot3 = 103;
const int kSoundBin = 2;
const int kSoundBGM = 3;                    // ������
const int kSoundGetProp = 4;                // ��õ���
const int kSoundPropOut = 5;
const int kSoundEnemyBomb = 6;              // �л���ը
const int kSoundCampBomb = 7;               // ��Ӫ��ը
const int kSoundScorePanelDi = 8;           // ������� ������
const int kSoundBouns1000 = 9;              // ��
const int kSoundAddLife = 10;
const int kSoundWin = 11;
const int kSoundFail = 12;
const int kSoundPlayerBomb = 13;
const int kSoundPlayerMove = 14;

// ������
class MciSound
{
public:
    static void InitSound();
    static void CloseSound();
    static void PlaySounds(int kind);
    static void PauseBGM(bool is_paused);           // ��ʼ����ͣ��������
    static void PlayMovingSound(bool is_play);      // ���̹���ƶ�����
};

} // namespace battlecity

#endif