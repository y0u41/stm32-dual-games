/**
 * @file        game_gomoku.c
 * @author      宇 (DeepSeek)
 * @version     V1.0
 * @date        2026-08-16
 * @brief       五子棋(双人对战) 15x15, 触摸落子, 悔棋, 和棋
 */
#include "game_gomoku.h"
#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/delay/delay.h"
#include "ui.h"

#define G_X0        27
#define G_Y0        80
#define G_GRID      19
#define G_CELLS     15
#define G_BOARD     (G_GRID * (G_CELLS - 1))

#define G_BG        BLUE

static btn_t s_btn_topback = { 250, 10, 60, 30, "BACK",    24, BLUE,  BLACK };
static btn_t s_btn_restart = {  20, 410, 80, 50, "RESTART", 20, BLUE,  BLACK };
static btn_t s_btn_undo    = { 120, 410, 80, 50, "UNDO",    24, BLUE,  BLACK };
static btn_t s_btn_back    = { 220, 410, 80, 50, "BACK",    24, BLUE,  BLACK };

static uint8_t s_board[G_CELLS][G_CELLS];
static uint8_t s_turn;
static uint8_t s_winner;
static uint8_t s_move_cnt;
static uint8_t s_win_c[5], s_win_r[5];
static uint8_t s_hist_c[20], s_hist_r[20], s_hist_clr[20];
static uint8_t s_hist_cnt;

static void gomoku_draw_piece(uint8_t col, uint8_t row, uint8_t color)
{
    uint16_t px = G_X0 + (uint16_t)col * G_GRID;
    uint16_t py = G_Y0 + (uint16_t)row * G_GRID;
    if (color == 1)
    {
        lcd_fill_circle(px, py, 8, BLACK);
        lcd_draw_circle(px, py, 8, WHITE);
    }
    else
    {
        lcd_fill_circle(px, py, 8, WHITE);
        lcd_draw_circle(px, py, 8, BLACK);
    }
}

static void gomoku_draw_board(void)
{
    uint8_t i;
    lcd_clear(G_BG);
    for (i = 0; i < G_CELLS; i++)
    {
        lcd_draw_line(G_X0, G_Y0 + i * G_GRID, G_X0 + G_BOARD, G_Y0 + i * G_GRID, GRAY);
        lcd_draw_line(G_X0 + i * G_GRID, G_Y0, G_X0 + i * G_GRID, G_Y0 + G_BOARD, GRAY);
    }
    ui_show_center(54, 16, "GOMOKU", GREEN);   /* 游戏标题 */
}

static void gomoku_draw_buttons(void)
{
    ui_draw_button(&s_btn_restart);
    ui_draw_button(&s_btn_undo);
    ui_draw_button(&s_btn_back);
}

static void gomoku_draw_status(void)
{
    lcd_fill(0, 0, lcddev.width - 1, 49, GRAYBLUE);
    ui_draw_button(&s_btn_topback);
    if (s_winner == 1)      lcd_show_string(10, 12, 230, 16, 16, "BLACK WINS!", WHITE);
    else if (s_winner == 2) lcd_show_string(10, 12, 230, 16, 16, "WHITE WINS!", WHITE);
    else if (s_winner == 3) lcd_show_string(10, 12, 230, 16, 16, "DRAW!", WHITE);
    else if (s_turn == 1)   lcd_show_string(10, 12, 230, 16, 16, "BLACK TURN", WHITE);
    else                    lcd_show_string(10, 12, 230, 16, 16, "WHITE TURN", WHITE);
}

static void gomoku_push(uint8_t col, uint8_t row, uint8_t clr)
{
    if (s_hist_cnt < 20)
    {
        s_hist_c[s_hist_cnt] = col;
        s_hist_r[s_hist_cnt] = row;
        s_hist_clr[s_hist_cnt] = clr;
        s_hist_cnt++;
    }
    else
    {
        uint8_t i;
        for (i = 0; i < 19; i++)
        {
            s_hist_c[i] = s_hist_c[i + 1];
            s_hist_r[i] = s_hist_r[i + 1];
            s_hist_clr[i] = s_hist_clr[i + 1];
        }
        s_hist_c[19] = col;
        s_hist_r[19] = row;
        s_hist_clr[19] = clr;
    }
}

static uint8_t gomoku_pop(uint8_t *col, uint8_t *row, uint8_t *clr)
{
    if (s_hist_cnt == 0) return 0;
    s_hist_cnt--;
    *col = s_hist_c[s_hist_cnt];
    *row = s_hist_r[s_hist_cnt];
    *clr = s_hist_clr[s_hist_cnt];
    return 1;
}

static uint8_t gomoku_check_win(uint8_t col, uint8_t row, uint8_t color)
{
    static const int8_t dirs[4][2] = { {1,0},{0,1},{1,1},{1,-1} };
    uint8_t d;
    for (d = 0; d < 4; d++)
    {
        int8_t dx = dirs[d][0], dy = dirs[d][1];
        int16_t sc = (int16_t)col, sr = (int16_t)row;
        uint8_t cnt = 0;

        while (sc - dx >= 0 && sc - dx < G_CELLS && sr - dy >= 0 && sr - dy < G_CELLS &&
               s_board[sr - dy][sc - dx] == color)
        {
            sc -= dx;
            sr -= dy;
        }

        while (sc >= 0 && sc < G_CELLS && sr >= 0 && sr < G_CELLS && s_board[sr][sc] == color)
        {
            if (cnt < 5) { s_win_c[cnt] = (uint8_t)sc; s_win_r[cnt] = (uint8_t)sr; }
            cnt++;
            sc += dx;
            sr += dy;
        }

        if (cnt >= 5) return 1;
    }
    return 0;
}

static void gomoku_show_win(void)
{
    uint8_t i;
    for (i = 0; i < 5; i++)
    {
        uint16_t px = G_X0 + (uint16_t)s_win_c[i] * G_GRID;
        uint16_t py = G_Y0 + (uint16_t)s_win_r[i] * G_GRID;
        lcd_draw_circle(px, py, 10, RED);
    }
    gomoku_draw_status();
}

static void gomoku_undo(void)
{
    uint8_t col, row, clr;
    uint16_t px, py;
    int16_t x1, x2, y1, y2;

    if (s_winner != 0) return;
    if (!gomoku_pop(&col, &row, &clr)) return;

    s_board[row][col] = 0;
    s_move_cnt--;

    px = G_X0 + (uint16_t)col * G_GRID;
    py = G_Y0 + (uint16_t)row * G_GRID;

    lcd_fill_circle(px, py, 9, G_BG);
    x1 = (int16_t)px - 9; if (x1 < G_X0) x1 = G_X0;
    x2 = (int16_t)px + 9; if (x2 > G_X0 + G_BOARD) x2 = G_X0 + G_BOARD;
    y1 = (int16_t)py - 9; if (y1 < G_Y0) y1 = G_Y0;
    y2 = (int16_t)py + 9; if (y2 > G_Y0 + G_BOARD) y2 = G_Y0 + G_BOARD;
    lcd_draw_line((uint16_t)x1, py, (uint16_t)x2, py, GRAY);
    lcd_draw_line(px, (uint16_t)y1, px, (uint16_t)y2, GRAY);

    s_turn = clr;
    gomoku_draw_status();
}

static void gomoku_restart(void)
{
    uint8_t i, j;
    for (i = 0; i < G_CELLS; i++)
        for (j = 0; j < G_CELLS; j++)
            s_board[i][j] = 0;
    s_turn = 1;
    s_winner = 0;
    s_move_cnt = 0;
    s_hist_cnt = 0;
    gomoku_draw_board();
    gomoku_draw_status();
    gomoku_draw_buttons();
}

void gomoku_init(void)
{
    gomoku_restart();
}

app_state_t gomoku_handle(void)
{
    uint16_t x, y;
    int16_t c, r;
    uint8_t col, row;

    if (!ui_get_click(&x, &y)) return APP_GOMOKU;

    if (ui_hit_test(&s_btn_topback, 1, x, y) == 0) return APP_MENU;

    if (ui_hit_test(&s_btn_restart, 1, x, y) == 0) { gomoku_restart(); return APP_GOMOKU; }
    if (ui_hit_test(&s_btn_back, 1, x, y) == 0)     { return APP_MENU; }
    if (ui_hit_test(&s_btn_undo, 1, x, y) == 0)     { gomoku_undo(); return APP_GOMOKU; }

    if (s_winner != 0) return APP_GOMOKU;

    c = (int16_t)x - G_X0 + 9;
    r = (int16_t)y - G_Y0 + 9;
    if (c < 0 || r < 0) return APP_GOMOKU;
    col = (uint8_t)(c / G_GRID);
    row = (uint8_t)(r / G_GRID);
    if (col >= G_CELLS || row >= G_CELLS) return APP_GOMOKU;
    if (s_board[row][col] != 0) return APP_GOMOKU;

    s_board[row][col] = s_turn;
    gomoku_draw_piece(col, row, s_turn);
    gomoku_push(col, row, s_turn);
    s_move_cnt++;

    if (gomoku_check_win(col, row, s_turn))
    {
        s_winner = s_turn;
        gomoku_show_win();
        return APP_GOMOKU;
    }
    if (s_move_cnt >= G_CELLS * G_CELLS)
    {
        s_winner = 3;
        gomoku_draw_status();
        return APP_GOMOKU;
    }

    s_turn = (s_turn == 1) ? 2 : 1;
    gomoku_draw_status();
    return APP_GOMOKU;
}