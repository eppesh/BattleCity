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
const int kSoundBGM = 3;                    // 背景音
const int kSoundGetProp = 4;                // 获得道具
const int kSoundPropOut = 5;
const int kSoundEnemyBomb = 6;              // 敌机爆炸
const int kSoundCampBomb = 7;               // 大本营爆炸
const int kSoundScorePanelDi = 8;           // 分数面板 嘀嘀声
const int kSoundBouns1000 = 9;              // ？
const int kSoundAddLife = 10;
const int kSoundWin = 11;
const int kSoundFail = 12;
const int kSoundPlayerBomb = 13;
const int kSoundPlayerMove = 14;

// 声音类
class MciSound
{
public:
    static void InitSound();
    static void CloseSound();
    static void PlaySounds(int kind);
    static void PauseBGM(bool is_paused);           // 开始或暂停背景音乐
    static void PlayMovingSound(bool is_play);      // 玩家坦克移动声音
};

} // namespace battlecity

#endif