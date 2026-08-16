#ifndef __UI_H
#define __UI_H

#include <stdint.h>

/* 按钮结构体 */
typedef struct {
    uint16_t x, y, w, h;    /* 左上角坐标与宽高 */
    const char *label;      /* 按钮文字(ASCII) */
    uint8_t  size;          /* 字号 12/16/24/32 */
    uint16_t bg_color;      /* 背景色 */
    uint16_t text_color;    /* 文字色 */
} btn_t;

void ui_draw_button(const btn_t *b);                 /* 画按钮(常态) */
void ui_draw_button_pressed(const btn_t *b);         /* 画按钮(按下反色) */
uint16_t ui_text_width(const char *s, uint8_t size); /* 文本像素宽度 */
int8_t ui_hit_test(const btn_t *btns, uint8_t n, uint16_t x, uint16_t y); /* 命中检测 */
uint8_t ui_get_click(uint16_t *px, uint16_t *py);    /* 触摸点击(按下-抬起+防抖) */
void ui_show_center(uint16_t y, uint8_t size, const char *s, uint16_t color); /* 居中文本 */

#endif