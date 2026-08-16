#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/TOUCH/touch.h"
#include "menu.h"
#include "game_snake.h"
#include "game_gomoku.h"

static app_state_t s_state;     /* 当前状态 */
static uint8_t s_need_init;     /* 进入状态需初始化 */

/* LED0 闪一下(状态切换指示, 可选) */
static void led_flash(void)
{
    LED0(0);
    delay_ms(50);
    LED0(1);
}

int main(void)
{
    sys_stm32_clock_init(9);    /* 72MHz */
    delay_init(72);
    usart_init(72, 115200);     /* 必须在 lcd_init 之前 */
    led_init();
    lcd_init();
    lcd_display_dir(0);         /* 竖屏 320x480 */
    key_init();
    tp_dev.init();              /* 触摸初始化(含自动校准) */

    s_state = APP_MENU;
    s_need_init = 1;

    while (1)
    {
        switch (s_state)
        {
            case APP_MENU:
                if (s_need_init) { menu_draw(); s_need_init = 0; }
                s_state = menu_handle();
                if (s_state != APP_MENU) { led_flash(); s_need_init = 1; }
                break;

            case APP_GOMOKU:
                if (s_need_init) { gomoku_init(); s_need_init = 0; }
                s_state = gomoku_handle();
                if (s_state == APP_MENU) { led_flash(); s_need_init = 1; }
                break;

            case APP_SNAKE:
                if (s_need_init) { snake_init(); s_need_init = 0; }
                s_state = snake_handle();
                if (s_state == APP_MENU) { led_flash(); s_need_init = 1; }
                break;
        }
        delay_ms(10);
    }
}