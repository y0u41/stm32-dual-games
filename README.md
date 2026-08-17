# STM32 掌上双游戏机（五子棋 + 贪吃蛇）

![License](https://img.shields.io/badge/license-MIT-green) ![Language](https://img.shields.io/badge/language-C-555555) ![Platform](https://img.shields.io/badge/platform-STM32F103ZET6-blue) ![Build](https://img.shields.io/badge/build-0%20Error-brightgreen)

基于 **正点原子精英版 STM32F103ZET6** 开发板和 **3.5 寸电阻触摸屏** 的掌上双游戏机：主菜单一键切换两个游戏，全程触摸操作，纯裸机实现，无操作系统依赖。

## 功能特性

- 🎮 **主菜单**：GOMOKU / SNAKE 双按钮进入，纯蓝底色，游戏间自由返回
- ⚫⚪ **五子棋**（双人对战）
  - 15×15 棋盘，黑先白后，触摸落子自动吸附交叉点
  - 四方向五连判定，获胜五子红圈高亮
  - 悔棋（最多 20 步）、重开、和棋判定
- 🐍 **贪吃蛇**
  - 20×18 网格地图，吃食物加分，每 5 个加速 10%（300ms → 100ms/格）
  - **最高分掉电保存**（24C02），顶部显示 SCORE / BEST
  - 触摸方向键控制（禁止 180° 掉头），物理按键备用（KEY0 左转 / KEY1 右转 / WK_UP 暂停）
  - 撞墙/撞身结束，GAME OVER 可重开
- 📱 **电阻触摸**：首次上电四角校准，校准数据保存至板载 24C02，此后开机自动跳过
- 💾 **资源占用小**：Flash 约 56KB / SRAM 约 3KB（64KB SRAM 下余量充足）
- 🔤 纯 ASCII 英文界面，不依赖 SD 卡字库

## 硬件要求

| 部件 | 说明 |
|---|---|
| 主控 | 正点原子**精英版** STM32F103ZET6（72MHz / 512KB Flash / 64KB SRAM） |
| 屏幕 | 3.5 寸 TFTLCD **电阻触摸屏**（320×480 竖屏，FSMC 16 位并口，XPT2046 触摸） |
| 下载器 | ST-Link（SWD 四线：SWDIO / SWCLK / GND / 3.3V） |

**触摸引脚（XPT2046 软件 SPI，精英版）**

| 信号 | 引脚 |
|---|---|
| T_PEN | PF10 |
| T_CS | PF11 |
| T_MISO | PB2 |
| T_MOSI | PF9 |
| T_CLK | PB1 |

**按键**：KEY0=PE4、KEY1=PE3、KEY2=PE2、WK_UP=PA0（贪吃蛇备用控制）

## 目录结构

```
STM32双游戏机/
├── README.md              项目说明（本文件）
├── .gitignore             Git 忽略规则（构建产物等）
├── keilkill.bat           一键清理 MDK 构建产物
└── KeilProject/           Keil 工程
    ├── Drivers/           底层驱动（SYSTEM / BSP：LCD、TOUCH、KEY、LED 等）
    ├── User/              应用层
    │   ├── main.c         主循环状态机（MENU / GOMOKU / SNAKE）
    │   ├── menu.c         主菜单
    │   ├── ui.c           按钮组件（绘制 / 命中检测 / 触摸点击状态机）
    │   ├── game_gomoku.c  五子棋
    │   └── game_snake.c   贪吃蛇
    └── Projects/
        └── MDK-ARM/
            └── atk_f103.uvprojx  工程文件
```

## 编译与烧录

1. 安装 **Keil MDK 5.43a**（编译器选 **AC6 / armclang**）
2. 打开 `KeilProject\Projects\MDK-ARM\atk_f103.uvprojx`
3. `Options for Target → Debug`：选择 **ST-Link Debugger**，`Settings → Port = SW`
4. `Flash Download`：勾选 **STM32F10x_512.FLM**，起始地址 `0x08000000`，大小 `0x80000`
5. `F7` 编译（0 Error），`LOAD` / `F8` 下载

命令行编译：

```bat
"D:\MDK5.36\UV4\UV4.exe" -b KeilProject\Projects\MDK-ARM\atk_f103.uvprojx -j0
```

**构建结果**：`0 Error / 0 Warning`，`Code=45088 RO-data=13492 RW-data=324 ZI-data=2692`

## 使用说明

### 首次上电：触摸校准

首次上电无校准数据时，屏幕自动进入校准界面：用触控笔依次点按屏幕上的四个角十字，校准完成后自动保存到 24C02，此后每次上电自动读取、跳过校准。

### 操作

- **主菜单**：点 GOMOKU 进五子棋，点 SNAKE 进贪吃蛇
- **五子棋**：触摸交叉点落子；RESTART 重开、UNDO 悔棋、BACK 返回菜单；顶部显示 BLACK/WHITE TURN 与胜负
- **贪吃蛇**：触摸底部方向键（UP/DOWN/LEFT/RIGHT）转向；顶部 PAUSE 暂停/继续、BACK 返回；KEY0 左转、KEY1 右转、WK_UP 暂停（备用）

## 致谢与版权

- 底层驱动（`sys / delay / usart / lcd / touch` 等）基于**正点原子精英版例程**，版权归 **广州市星翼电子科技有限公司** 所有
- 游戏应用层代码由 **DeepSeek Harness** 辅助生成，并经过人工修改完善
- 本仓库以 **MIT 许可证** 发布，详见 [LICENSE](LICENSE)
