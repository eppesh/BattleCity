#include "stdafx.h"
#include "mci_sound.h"

namespace battlecity
{
void MciSound::InitSound()
{
    mciSendString(_T("open ./res/music/start.wav alias start"), NULL, 0, NULL);
    mciSendString(_T("open ./res/music/shoot.wav alias shoot0"), NULL, 0, NULL);        // 一个玩家两个子弹
    mciSendString(_T("open ./res/music/shoot.wav alias shoot1"), NULL, 0, NULL);        // 
    mciSendString(_T("open ./res/music/shoot.wav alias shoot2"), NULL, 0, NULL);        // 
    mciSendString(_T("open ./res/music/shoot.wav alias shoot3"), NULL, 0, NULL);        //

    mciSendString(_T("open ./res/music/bin.wav alias bin"), NULL, 0, NULL);             //
    mciSendString(_T("open ./res/music/bk_sound.wav alias bk_sound"), NULL, 0, NULL);   // 背景音
    mciSendString(_T("open ./res/music/get-prop.wav alias get_prop"), NULL, 0, NULL);   //
    mciSendString(_T("open ./res/music/enemy-bomb.wav alias enemy_bomb"), NULL, 0, NULL); 
    mciSendString(_T("open ./res/music/scorepanel_di.wav alias scorepanel_di"), NULL, 0, NULL); 
    mciSendString(_T("open ./res/music/bouns1000.wav alias bouns1000"), NULL, 0, NULL); //
    mciSendString(_T("open ./res/music/addlife.wav alias addlife"), NULL, 0, NULL);     //
    mciSendString(_T("open ./res/music/fail.wav alias fail"), NULL, 0, NULL);           //
    mciSendString(_T("open ./res/music/player_bomb.wav alias player_bomb"), NULL, 0, NULL); 
    mciSendString(_T("open ./res/music/player_move.wav alias player_move"), NULL, 0, NULL); 
    mciSendString(_T("open ./res/music/prop-out.wav alias prop_out"), NULL, 0, NULL);   //
}

void MciSound::CloseSound()
{
    mciSendString(_T("close start"), NULL, 0, NULL);
    mciSendString(_T("close shoot0"), NULL, 0, NULL);
    mciSendString(_T("close shoot1"), NULL, 0, NULL);
    mciSendString(_T("close shoot2"), NULL, 0, NULL);
    mciSendString(_T("close shoot3"), NULL, 0, NULL);
    mciSendString(_T("close bin"), NULL, 0, NULL);
    mciSendString(_T("close bk_sound"), NULL, 0, NULL);
    mciSendString(_T("close get_prop"), NULL, 0, NULL);
    mciSendString(_T("close enemy_bomb"), NULL, 0, NULL);
    mciSendString(_T("close scorepanel_di"), NULL, 0, NULL);
    mciSendString(_T("close bouns1000"), NULL, 0, NULL);
    mciSendString(_T("close addlife"), NULL, 0, NULL);
    mciSendString(_T("close fail"), NULL, 0, NULL);
    mciSendString(_T("close player_bomb"), NULL, 0, NULL);
    mciSendString(_T("close player_move"), NULL, 0, NULL);
    mciSendString(_T("close prop_out"), NULL, 0, NULL);
}

void MciSound::PlaySounds(int kind)
{
    switch (kind)
    {
    case kSoundStart:
        mciSendString(_T("play start from 0"), NULL, 0, NULL);
        break;
    case kSoundShoot0:
        mciSendString(_T("play shoot0 from 0"), NULL, 0, NULL);
        break;
    case kSoundShoot1:
        mciSendString(_T("play shoot1 from 0"), NULL, 0, NULL);
        break;
    case kSoundShoot2:
        mciSendString(_T("play shoot2 from 0"), NULL, 0, NULL);
        break;
    case kSoundShoot3:
        mciSendString(_T("play shoot3 from 0"), NULL, 0, NULL);
        break;
    case kSoundBin:
        mciSendString(_T("play bin from 0"), NULL, 0, NULL);
        break;
    case kSoundBGM:
        mciSendString(_T("play bk_sound from 0"), NULL, 0, NULL);
        break;
    case kSoundGetProp:
        mciSendString(_T("play get_prop from 0"), NULL, 0, NULL);
        break;
    case kSoundPropOut:
        mciSendString(_T("play prop_out from 0"), NULL, 0, NULL);
        break;
    case kSoundEnemyBomb:
        mciSendString(_T("play enemy_bomb from 0"), NULL, 0, NULL);
        break;
    case kSoundCampBomb:
        mciSendString(_T("play player_bomb from 0"), NULL, 0, NULL);
        break;
    case kSoundScorePanelDi:
        mciSendString(_T("play scorepanel_di from 0"), NULL, 0, NULL);
        break;
    case kSoundBouns1000:
        mciSendString(_T("play bouns1000 from 0"), NULL, 0, NULL);
        break;
    case kSoundAddLife:
        mciSendString(_T("play addlife from 0"), NULL, 0, NULL);
        break;
    case kSoundFail:
        mciSendString(_T("play fail from 0"), NULL, 0, NULL);
        break;
    case kSoundPlayerBomb:
        mciSendString(_T("play player_bomb from 0"), NULL, 0, NULL);
        break;
    case kSoundPlayerMove:
        mciSendString(_T("play player_move from 0"), NULL, 0, NULL);
        break;

    default:
        break;
    }
}

void MciSound::PauseBGM(bool is_paused)
{
    if (is_paused)
    {
        mciSendString(_T("pause bk_sound"), NULL, 0, NULL);
    }
    else
    {
        mciSendString(_T("play bk_sound from 0"), NULL, 0, NULL);
    }
}

void MciSound::PlayMovingSound(bool is_play)
{
    if (is_play)
    {
        mciSendString(_T("play player_move from 0"), NULL, 0, NULL);
    }
    else
    {
        mciSendString(_T("pause player_move"), NULL, 0, NULL);
    }
}
} // namespace battlecity