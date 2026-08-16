#ifndef __GAME_GOMOKU_H
#define __GAME_GOMOKU_H

#include "menu.h"       /* app_state_t */

void gomoku_init(void);          /* 进入五子棋: 初始化并绘制 */
app_state_t gomoku_handle(void); /* 每轮调用: 处理触摸/落子, 返回下一状态 */

#endif