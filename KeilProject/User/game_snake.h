#ifndef __GAME_SNAKE_H
#define __GAME_SNAKE_H

#include "menu.h"       /* app_state_t */

void snake_init(void);          /* 进入贪吃蛇: 初始化并绘制 */
app_state_t snake_handle(void); /* 每轮调用: 处理触摸/按键/节拍, 返回下一状态 */

#endif