/**
 * @file        game_snake.c
 * @author      �� (DeepSeek)
 * @version     V1.1
 * @date        2026-08-16
 * @brief       ̰���� (���� 320x480, ��������� + ��ͼ����ת��)
 *   @note      20�� x 18��, ����16px, ��ͼ�� y=72..360, ������ y=368..480
 */
#include "game_snake.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/TOUCH/touch.h"
#include "./BSP/24CXX/24cxx.h"
#include "./SYSTEM/delay/delay.h"
#include "ui.h"
#include <stdio.h>

#define SNAKE_COLS       20
#define SNAKE_ROWS       18
#define SNAKE_CELL       16
#define SNAKE_MAP_Y      72
#define SNAKE_MAP_BOTTOM (SNAKE_MAP_Y + SNAKE_ROWS * SNAKE_CELL)
#define SNAKE_MAX_LEN    200
#define SNAKE_TICK_INIT  300
#define SNAKE_TICK_MIN   100
#define SNAKE_ACCEL_EVERY 5
#define SNAKE_SWIPE      10      /* ������ֵpx, ԽСԽ���� */
#define SNAKE_BEST_ADDR  0x60    /* ��߷ִ洢��ַ(�ܿ�����У׼��40-52) */
#define SNAKE_BEST_MAGIC 0xA5    /* ��߷���Ч��־ */

#define SNAKE_CLS_BG     BLUE
#define SNAKE_CLS_BODY   GREEN
#define SNAKE_CLS_HEAD   YELLOW
#define SNAKE_CLS_FOOD   RED
#define SNAKE_CLS_BAR    GRAYBLUE

static btn_t s_btn_pause  = { 250, 10, 60, 30, "PAUSE", 24, BLUE,  BLACK };
static btn_t s_btn_back   = { 185, 10, 55, 30, "BACK",  24, BLUE,  BLACK };
static btn_t s_dir[4] = {
    {  90, 368, 140, 52, "UP",    32, BLUE,  BLACK },
    {  90, 425, 140, 52, "DOWN",  32, BLUE,  BLACK },
    {  10, 385,  70, 75, "LEFT",  32, BLUE,  BLACK },
    { 240, 385,  70, 75, "RIGHT", 32, BLUE,  BLACK },
};
static btn_t s_btn_restart = { 60, 250, 200, 55, "RESTART", 32, BLUE,  BLACK };
static btn_t s_btn_overbk  = { 60, 320, 200, 55, "BACK",    32, BLUE,  BLACK };

static uint8_t  s_x[SNAKE_MAX_LEN];
static uint8_t  s_y[SNAKE_MAX_LEN];
static uint16_t s_len;
static int8_t   s_dx, s_dy;
static uint8_t  s_fx, s_fy;
static uint16_t s_score;
static uint16_t s_tick;
static uint8_t  s_pause;
static uint8_t  s_over;
static uint8_t  s_dir_locked;
static uint8_t  s_food_cnt;
static uint16_t s_acc;
static uint32_t s_rnd;
static uint16_t s_best;

static uint8_t  s_tdown;
static uint8_t  s_moved;
static uint8_t  s_debounce;
static uint16_t s_ax, s_ay;
static uint16_t s_px, s_py;

static void snake_srand(uint32_t seed)
{
    s_rnd = seed ? seed : 0x9E3779B9u;
}

static uint32_t snake_rand(void)
{
    uint32_t x = s_rnd;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    s_rnd = x;
    return x;
}

static void snake_draw_cell(uint8_t gx, uint8_t gy, uint16_t color)
{
    uint16_t x0 = (uint16_t)gx * SNAKE_CELL + 1;
    uint16_t y0 = SNAKE_MAP_Y + (uint16_t)gy * SNAKE_CELL + 1;
    lcd_fill(x0, y0, x0 + SNAKE_CELL - 3, y0 + SNAKE_CELL - 3, color);
}

static void snake_place_food(void)
{
    uint16_t i;
    for (;;)
    {
        s_fx = (uint8_t)(snake_rand() % SNAKE_COLS);
        s_fy = (uint8_t)(snake_rand() % SNAKE_ROWS);
        for (i = 0; i < s_len; i++)
        {
            if (s_x[i] == s_fx && s_y[i] == s_fy) break;
        }
        if (i >= s_len) break;
    }
}

static void snake_reset(void)
{
    uint8_t i;
    s_len = 4;
    s_score = 0;
    s_tick = SNAKE_TICK_INIT;
    s_dx = 1;
    s_dy = 0;
    s_pause = 0;
    s_over = 0;
    s_dir_locked = 0;
    s_food_cnt = 0;
    s_acc = 0;
    s_tdown = 0;
    s_moved = 0;
    s_debounce = 0;
    s_x[0] = SNAKE_COLS / 2;
    s_y[0] = SNAKE_ROWS / 2;
    for (i = 1; i < s_len; i++)
    {
        s_x[i] = s_x[0] - i;
        s_y[i] = s_y[0];
    }
    snake_place_food();
}

static void snake_best_load(void)
{
    if (at24cxx_read_one_byte(SNAKE_BEST_ADDR) == SNAKE_BEST_MAGIC)
    {
        s_best = ((uint16_t)at24cxx_read_one_byte(SNAKE_BEST_ADDR + 1) << 8) |
                  (uint16_t)at24cxx_read_one_byte(SNAKE_BEST_ADDR + 2);
    }
    else
    {
        s_best = 0;
    }
}

static void snake_best_save(void)
{
    at24cxx_write_one_byte(SNAKE_BEST_ADDR, SNAKE_BEST_MAGIC);
    at24cxx_write_one_byte(SNAKE_BEST_ADDR + 1, (uint8_t)(s_best >> 8));
    at24cxx_write_one_byte(SNAKE_BEST_ADDR + 2, (uint8_t)(s_best & 0xFF));
}

static void snake_draw_title(void)
{
    lcd_fill(0, 50, lcddev.width - 1, SNAKE_MAP_Y - 1, SNAKE_CLS_BG);
    ui_show_center(52, 16, "SNAKE", GREEN);
}

static void snake_draw_field(void)
{
    uint8_t i;
    lcd_fill(0, SNAKE_MAP_Y, lcddev.width - 1, lcddev.height - 1, SNAKE_CLS_BG);
    for (i = 0; i < s_len; i++) snake_draw_cell(s_x[i], s_y[i], SNAKE_CLS_BODY);
    snake_draw_cell(s_x[0], s_y[0], SNAKE_CLS_HEAD);
    snake_draw_cell(s_fx, s_fy, SNAKE_CLS_FOOD);
}

static void snake_show_score(void)
{
    char buf[24];
    lcd_fill(0, 0, 184, 49, SNAKE_CLS_BAR);
    sprintf(buf, "SCORE:%u BEST:%u", (unsigned int)s_score, (unsigned int)s_best);
    lcd_show_string(4, 12, 180, 16, 16, buf, WHITE);
}

static void snake_draw_bar(void)
{
    lcd_fill(0, 0, lcddev.width - 1, 49, SNAKE_CLS_BAR);
    ui_draw_button(&s_btn_back);
    ui_draw_button(&s_btn_pause);
    snake_show_score();
}

static void snake_set_dir(int8_t dx, int8_t dy)
{
    if (dx == -s_dx && dy == -s_dy) return;
    s_dx = dx;
    s_dy = dy;
}

static void snake_step(void)
{
    uint8_t nx, ny, i, tx, ty, ate;

    nx = s_x[0] + s_dx;
    ny = s_y[0] + s_dy;

    if (nx >= SNAKE_COLS || ny >= SNAKE_ROWS) { s_over = 1; return; }

    ate = (nx == s_fx && ny == s_fy) ? 1 : 0;
    for (i = 1; i < s_len; i++)
    {
        if (s_x[i] == nx && s_y[i] == ny)
        {
            if (i == s_len - 1 && !ate) continue;
            s_over = 1;
            return;
        }
    }

    tx = s_x[s_len - 1];
    ty = s_y[s_len - 1];
    for (i = s_len - 1; i > 0; i--)
    {
        s_x[i] = s_x[i - 1];
        s_y[i] = s_y[i - 1];
    }
    s_x[0] = nx;
    s_y[0] = ny;

    snake_draw_cell(s_x[1], s_y[1], SNAKE_CLS_BODY);

    if (ate)
    {
        s_len++;
        s_score += 10;
        s_food_cnt++;
        if (s_food_cnt >= SNAKE_ACCEL_EVERY)
        {
            s_food_cnt = 0;
            if (s_tick > SNAKE_TICK_MIN)
            {
                s_tick = s_tick - s_tick / 10;
                if (s_tick < SNAKE_TICK_MIN) s_tick = SNAKE_TICK_MIN;
            }
        }
        snake_draw_cell(nx, ny, SNAKE_CLS_HEAD);
        snake_place_food();
        snake_draw_cell(s_fx, s_fy, SNAKE_CLS_FOOD);
        snake_show_score();
    }
    else
    {
        snake_draw_cell(tx, ty, SNAKE_CLS_BG);
        snake_draw_cell(nx, ny, SNAKE_CLS_HEAD);
    }
}

static void snake_draw_over(void)
{
    char buf[24];
    if (s_score > s_best) { s_best = s_score; snake_best_save(); }
    lcd_fill(0, 0, lcddev.width - 1, SNAKE_MAP_Y - 1, SNAKE_CLS_BG);
    lcd_fill(0, SNAKE_MAP_Y, lcddev.width - 1, lcddev.height - 1, SNAKE_CLS_BG);
    ui_show_center(130, 24, "GAME OVER", RED);
    sprintf(buf, "SCORE:%u  BEST:%u", (unsigned int)s_score, (unsigned int)s_best);
    ui_show_center(190, 16, buf, WHITE);
    ui_draw_button(&s_btn_restart);
    ui_draw_button(&s_btn_overbk);
}

/* ����: �̰�=���(����1), ��ͼ����=ת�� */
static uint8_t snake_touch(uint16_t *cx, uint16_t *cy)
{
    uint16_t x, y;
    int16_t dx, dy, adx, ady;
    uint8_t r = 0;

    tp_dev.scan(0);

    if (s_debounce) s_debounce--;

    if (tp_dev.sta & TP_PRES_DOWN)
    {
        x = tp_dev.x[0];
        y = tp_dev.y[0];

        if (!s_tdown)
        {
            s_tdown = 1;
            s_moved = 0;
            s_ax = x; s_ay = y;
            s_px = x; s_py = y;
        }
        else
        {
            dx = (int16_t)x - (int16_t)s_ax;
            dy = (int16_t)y - (int16_t)s_ay;
            adx = dx < 0 ? -dx : dx;
            ady = dy < 0 ? -dy : dy;

            if (adx > SNAKE_SWIPE || ady > SNAKE_SWIPE)
            {
                s_moved = 1;
                if (s_py >= SNAKE_MAP_Y && s_py < SNAKE_MAP_BOTTOM && !s_over && !s_pause && !s_dir_locked)
                {
                    s_dir_locked = 1;
                    if (adx > ady) snake_set_dir(dx > 0 ? 1 : -1, 0);
                    else           snake_set_dir(0, dy > 0 ? 1 : -1);
                }
                s_ax = x; s_ay = y;
            }
        }
    }
    else
    {
        if (s_tdown)
        {
            s_tdown = 0;
            if (!s_moved && s_debounce == 0)
            {
                if (cx) *cx = s_px;
                if (cy) *cy = s_py;
                s_debounce = 10;
                r = 1;
            }
        }
    }

    return r;
}

void snake_init(void)
{
    /* 用 SysTick 计数做种子，替代硬编码 0x12345678，使每局食物位置不同 */
    snake_srand(SysTick->VAL ^ (HAL_GetTick() << 16) ^ 0xDEADBEEFu);
    snake_reset();
    snake_best_load();
    snake_draw_field();
    snake_draw_bar();
    snake_draw_title();
}

app_state_t snake_handle(void)
{
    uint16_t x, y;
    uint8_t key;
    int8_t idx;

    if (snake_touch(&x, &y))
    {
        if (ui_hit_test(&s_btn_back, 1, x, y) == 0) return APP_MENU;

        if (!s_over)
        {
            if (ui_hit_test(&s_btn_pause, 1, x, y) == 0)
            {
                s_pause = !s_pause;
                if (s_pause) ui_show_center(200, 24, "PAUSED", YELLOW);
                else { snake_draw_field(); snake_draw_bar(); snake_draw_title(); }
            }
            else if (!s_pause)
            {
                idx = ui_hit_test(s_dir, 4, x, y);
                if (idx >= 0 && !s_dir_locked)
                {
                    s_dir_locked = 1;
                    if (idx == 0) snake_set_dir(0, -1);
                    else if (idx == 1) snake_set_dir(0, 1);
                    else if (idx == 2) snake_set_dir(-1, 0);
                    else snake_set_dir(1, 0);
                }
            }
        }
        else
        {
            if (ui_hit_test(&s_btn_restart, 1, x, y) == 0)
            {
                snake_reset();
                snake_draw_field();
                snake_draw_bar();
            }
            else if (ui_hit_test(&s_btn_overbk, 1, x, y) == 0)
            {
                return APP_MENU;
            }
        }
    }

    key = key_scan(0);
    if (!s_over && !s_pause && !s_dir_locked)
    {
        if (key == KEY0_PRES)
        {
            int8_t d = s_dx;
            s_dx = s_dy;
            s_dy = -d;
            s_dir_locked = 1;
        }
        else if (key == KEY1_PRES)
        {
            int8_t d = s_dx;
            s_dx = -s_dy;
            s_dy = d;
            s_dir_locked = 1;
        }
    }
    if (!s_over && key == WKUP_PRES)
    {
        s_pause = !s_pause;
        if (s_pause) ui_show_center(200, 24, "PAUSED", YELLOW);
        else { snake_draw_field(); snake_draw_bar(); snake_draw_title(); }
    }

    if (!s_pause && !s_over)
    {
        s_acc += 10;
        if (s_acc >= s_tick)
        {
            s_acc = 0;
            s_dir_locked = 0;
            snake_step();
            if (s_over) snake_draw_over();
        }
    }

    return APP_SNAKE;
}