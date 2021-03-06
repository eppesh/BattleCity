# BattleCity

## 项目说明

该项目是经典的模拟FC的坦克大战游戏，仅供学习。

感谢xiongfj(837943056@qq.com)，本项目主要参考学习了[项目](https://codebus.cn/xiongfj/fc-tank)。  

## 游戏截图
![游戏截图1](https://github.com/eppesh/BattleCity/blob/main/Screenshot/pic_1.png)  
![游戏截图2](https://github.com/eppesh/BattleCity/blob/main/Screenshot/pic_2.png) 
![游戏截图3](https://github.com/eppesh/BattleCity/blob/main/Screenshot/pic_3.png) 

## 编译环境

- VS2019
- EasyX 2021  

## 目录说明

- **BattleCity** 主目录

  > - 该目录为主目录，直接运行后即可开始游戏；
  > - **res** 为资源目录，内含游戏所需的贴图（big目录）、地图文件（data目录）及游戏音乐（music目录）；
  > - **main.cpp** 为程序入口；

- **Map** 目录

  > - 该目录是为了生成地图文件map.dat；
  > - 若BattleCity/res/data/目录下没有map.dat时，可用该工程生成map.dat文件，并拷贝过去供游戏使用；

- **Screenshot** 目录

  > - 该目录包含部分游戏截图；  
  
## 游戏操作说明

- 选择玩家个数

  > - 刚进入游戏时是选择面板（见截图1），可通过回车键快速进入选择状态；
  > - 使用键盘上的上下方向键选择玩家个数：“1 PLAYER”表示一个玩家；“2 PLAYER”表示两个玩家；“CONSTRUCTION”表示自定义地图；
  > - 选择完成后，按回车键进入游戏；

- 一个玩家

  > - 使用 **a s d w** 键操作方向；
  > - 使用 **j** 键发射子弹; (注：子弹只有在碰到障碍物或敌方坦克后才会发射下一发)

- 两个玩家

  > - 使用 **方向键** 操作方向；
  > - 使用 **数字键1或者数字1（Q键左上方那个）** 发射子弹；

- 自定义地图

  > - 使用 **方向键** 操作方向；
  > - 使用 **j** 键绘制地图，连续按 **j** 键可以切换不同的地图元素；
  > - 绘制完地图按 **回车键** 退出，并选择玩家个数后开始游戏；
  
