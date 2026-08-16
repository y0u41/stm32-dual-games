#ifndef __MENU_H
#define __MENU_H

/* Ó¦ÓÃ×´Ì¬ */
typedef enum {
    APP_MENU = 0,
    APP_GOMOKU,
    APP_SNAKE
} app_state_t;

void menu_draw(void);
app_state_t menu_handle(void);

#endif