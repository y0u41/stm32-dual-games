#include "./BSP/LCD/lcd.h"
#include "./BSP/TOUCH/touch.h"
#include "ui.h"
#include <string.h>

/* 计算ASCII文本像素宽度(半角字符宽 = size/2) */
uint16_t ui_text_width(const char *s, uint8_t size)
{
    return (uint16_t)(size * (uint16_t)strlen(s)) / 2;
}

/* 居中显示一行文本 */
void ui_show_center(uint16_t y, uint8_t size, const char *s, uint16_t color)
{
    uint16_t tw = ui_text_width(s, size);
    uint16_t x = (tw < lcddev.width) ? ((lcddev.width - tw) / 2) : 0;
    lcd_show_string(x, y, lcddev.width, size, size, (char *)s, color);
}

/* 画按钮: 填充 + 描边 + 居中文字 */
void ui_draw_button(const btn_t *b)
{
    uint16_t tw = ui_text_width(b->label, b->size);
    uint16_t tx = b->x + ((b->w - tw) / 2);
    uint16_t ty = b->y + ((b->h - b->size) / 2);

    lcd_fill(b->x, b->y, b->x + b->w - 1, b->y + b->h - 1, b->bg_color);

    lcd_show_string(tx, ty, b->w, b->size, b->size, (char *)b->label, b->text_color);
}

/* 画按钮(按下反色) */
void ui_draw_button_pressed(const btn_t *b)
{
    uint16_t tw = ui_text_width(b->label, b->size);
    uint16_t tx = b->x + ((b->w - tw) / 2);
    uint16_t ty = b->y + ((b->h - b->size) / 2);

    lcd_fill(b->x, b->y, b->x + b->w - 1, b->y + b->h - 1, b->text_color);

    lcd_show_string(tx, ty, b->w, b->size, b->size, (char *)b->label, b->bg_color);
}

/* 命中检测: 返回按钮下标, 未命中返回 -1 */
int8_t ui_hit_test(const btn_t *btns, uint8_t n, uint16_t x, uint16_t y)
{
    uint8_t i;
    for (i = 0; i < n; i++)
    {
        if (x >= btns[i].x && x < (uint16_t)(btns[i].x + btns[i].w) &&
            y >= btns[i].y && y < (uint16_t)(btns[i].y + btns[i].h))
        {
            return (int8_t)i;
        }
    }
    return -1;
}

/* 触摸点击状态机: 按下锁存坐标, 抬起时返回一次点击 */
static uint8_t  s_pressed = 0;
static uint16_t s_lx = 0, s_ly = 0;
static uint8_t  s_debounce = 0;

uint8_t ui_get_click(uint16_t *px, uint16_t *py)
{
    uint8_t down;

    tp_dev.scan(0);
    down = (tp_dev.sta & TP_PRES_DOWN) ? 1 : 0;

    if (s_debounce) s_debounce--;

    if (down && !s_pressed)         /* 按下沿: 锁存坐标 */
    {
        s_pressed = 1;
        s_lx = tp_dev.x[0];
        s_ly = tp_dev.y[0];
        return 0;
    }

    if (!down && s_pressed)         /* 抬起沿: 完成一次点击 */
    {
        s_pressed = 0;
        if (px) *px = s_lx;
        if (py) *py = s_ly;
        s_debounce = 10;            /* 100ms 防抖 */
        return 1;
    }

    return 0;
}