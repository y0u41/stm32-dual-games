/**
 * @file        menu.c
 * @author      宇 (DeepSeek)
 * @version     V1.4
 * @date        2026-08-16
 * @brief       主菜单: 纵向5等分(标题1份/两游戏各2份) + 纹理 + 图标 + 右下角签名
 */
#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/delay/delay.h"
#include "ui.h"
#include "menu.h"

/* 背景: 纯蓝色 */
static void menu_draw_bg(void)
{
    lcd_clear(BLUE);
}

/* 五子棋图标: 黑色圆形 + 白边 + 中心白子 */
static void menu_draw_gomoku_icon(void)
{
    lcd_fill_circle(160, 170, 35, BLACK);
    lcd_draw_circle(160, 170, 35, WHITE);
    lcd_fill_circle(160, 170, 8, WHITE);
}

/* 贪吃蛇图标: 黑色开放波浪蛇(头大尾小) */
static void menu_draw_snake_icon(void)
{
    lcd_fill_circle(118, 360, 11, BLACK);  /* 头 */
    lcd_fill_circle(134, 371, 9, BLACK);
    lcd_fill_circle(150, 376, 8, BLACK);
    lcd_fill_circle(166, 376, 7, BLACK);
    lcd_fill_circle(180, 371, 6, BLACK);
    lcd_fill_circle(192, 360, 5, BLACK);   /* 尾 */
    lcd_fill_circle(114, 355, 2, WHITE);   /* 眼睛 */
    lcd_fill_circle(121, 355, 2, WHITE);
}

void menu_draw(void)
{
    menu_draw_bg();

    /* 标题: 第1份 (y=0..96) */
    ui_show_center(32, 32, "MINI GAMES", RED);

    /* 五子棋: 第2-3份 (y=96..288) */
    menu_draw_gomoku_icon();
    ui_show_center(222, 24, "GOMOKU", GREEN);

    /* 贪吃蛇: 第4-5份 (y=288..480) */
    menu_draw_snake_icon();
    ui_show_center(415, 24, "SNAKE", GREEN);

    /* 右下角签名 */
    lcd_show_string(296, 456, 20, 16, 16, "yu", YELLOW);
}

app_state_t menu_handle(void)
{
    uint16_t x, y;

    if (!ui_get_click(&x, &y)) return APP_MENU;

    /* 五子棋区域: 第2-3份 */
    if (x >= 60 && x < 260 && y >= 96 && y < 288) return APP_GOMOKU;

    /* 贪吃蛇区域: 第4-5份 */
    if (x >= 60 && x < 260 && y >= 288 && y < 470) return APP_SNAKE;

    return APP_MENU;
}