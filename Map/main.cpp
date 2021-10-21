// ���ƣ�̹�˴�ս��ͼ���ɣ�Battle City - Map��
// ��Ȩ������ѧϰ
// ���ߣ�Sean (eppesh@163.com)
// ������VS2019;EasyX 2021
// ʱ�䣺2021/08/26
// ˵��������̹�˴�ս��Ϸ��map.dat�ļ���

// ��л��xiongfj (https://codebus.cn/xiongfj/fc-tank 837943056@qq.com)

// ˵�������� map.dat �ļ������Ƶ� Tank.sln �� / Tank / res / data / map.dat ��ʹ�á�
// ������ȫ��ʹ�ø���Ŀ���ɣ�ֱ�ӽ��������뵽 map.dat��
// ��ͼ���Լ����0 �յ�; 1 ɭ��; 2 ��; 3 ǽ; 4 ����; 5 ʯͷ;
// ÿһ�����ֶ�Ӧ��Ϸ���ڵ�ÿ�еĿ飨��Ϸ����ͬ������Ϊ 26 * 26 ������飬һ����ռ 4 * 4 ���أ�

#include <conio.h>
#include <fstream>
#include "graphics.h"
#include <iostream>
#include "map.h"
#include <tchar.h>
#include <windows.h>

using namespace battlecity_map;

void WriteMap();
void TestRead();

int main()
{
    WriteMap();
    //TestRead();	// ���Զ�ȡ��ͼ
    _getch();
    return 0;
}

void WriteMap()
{
    Map map[35];
    map[0].map_1();
    map[1].map_2();
    map[2].map_3();
    map[3].map_4();
    map[4].map_5();
    map[5].map_6();
    map[6].map_7();
    map[7].map_8();
    map[8].map_9();
    map[9].map_10();
    map[10].map_11();
    map[11].map_12();
    map[12].map_13();
    map[13].map_14();
    map[14].map_15();
    map[15].map_16();
    map[16].map_17();
    map[17].map_18();
    map[18].map_19();
    map[19].map_20();
    map[20].map_21();
    map[21].map_22();
    map[22].map_23();
    map[23].map_24();
    map[24].map_25();
    map[25].map_26();
    map[26].map_27();
    map[27].map_28();
    map[28].map_29();
    map[29].map_30();
    map[30].map_31();
    map[31].map_32();
    map[32].map_33();
    map[33].map_34();
    map[34].map_35();

    FILE *fp = NULL;
    fopen_s(&fp, "map.dat", "ab+");

    if (fp == nullptr)
    {
        std::cout << "fp Ϊ��" << std::endl;
        return;
    }

    for (int i = 0; i < 35; i++)
    {
        fwrite(&map[i], sizeof(Map), 1, fp);
    }
    fclose(fp);

    std::cout << "map.dat �����ɵ���Ŀ��·���¡�" << std::endl;
}

void TestRead()
{
    Map map;
    FILE *fp = NULL;
    fopen_s(&fp, "map.dat", "rb");

    for (int m = 0; m < 35; m++)
    {
        fread(&map, sizeof(Map), 1, fp);
        for (int i = 0; i < 26; i++)
        {
            std::cout << map.buf_[i] << std::endl;
        }
        std::cout << std::endl;
    }
    fclose(fp);

    std::cout << "��ͼ��ȡ����ȷ��" << std::endl;
}