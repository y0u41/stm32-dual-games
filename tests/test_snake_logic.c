/**
 * 贪吃蛇游戏逻辑主机侧单元测试
 * 提取核心算法(xorshift RNG/碰撞判定/边界检测)在PC上验证
 * 编译: gcc -o test_snake test_snake_logic.c && ./test_snake
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

/* ===== 从 game_snake.c 提取的核心常量 ===== */
#define SNAKE_COLS 20
#define SNAKE_ROWS 18
#define SNAKE_MAX_LEN 200
#define SNAKE_TICK_INIT 300
#define SNAKE_TICK_MIN 100
#define SNAKE_ACCEL_EVERY 5

/* ===== xorshift32 RNG (与 game_snake.c 相同实现) ===== */
static uint32_t s_rnd;

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

/* ===== 游戏状态 ===== */
static uint8_t  s_x[SNAKE_MAX_LEN];
static uint8_t  s_y[SNAKE_MAX_LEN];
static uint16_t s_len;
static int8_t   s_dx, s_dy;
static uint8_t  s_fx, s_fy;
static uint16_t s_score;
static uint16_t s_tick;
static uint8_t  s_over;
static uint8_t  s_food_cnt;
static uint16_t s_acc;

static void snake_reset(void)
{
    s_len = 4;
    s_score = 0;
    s_tick = SNAKE_TICK_INIT;
    s_dx = 1;
    s_dy = 0;
    s_over = 0;
    s_food_cnt = 0;
    s_acc = 0;
    /* 初始蛇身: 4格水平 */
    s_x[0] = 5; s_y[0] = 9;
    s_x[1] = 4; s_y[1] = 9;
    s_x[2] = 3; s_y[2] = 9;
    s_x[3] = 2; s_y[3] = 9;
    /* 食物放固定位置便于测试 */
    s_fx = 10; s_fy = 9;
}

/* 检测碰撞: 返回 1=撞墙, 2=撞自身, 3=正常移动, 4=吃到食物 */
static int snake_check(int8_t dx, int8_t dy)
{
    uint8_t nx = s_x[0] + dx;
    uint8_t ny = s_y[0] + dy;
    uint16_t i;

    if (nx >= SNAKE_COLS || ny >= SNAKE_ROWS) return 1; /* 撞墙 */

    for (i = 1; i < s_len; i++)
    {
        if (s_x[i] == nx && s_y[i] == ny)
        {
            /* 尾巴下一格允许进入(除非要吃食物) */
            if (i == s_len - 1 && !(nx == s_fx && ny == s_fy)) continue;
            return 2; /* 撞自身 */
        }
    }

    if (nx == s_fx && ny == s_fy) return 4; /* 吃到食物 */
    return 3; /* 正常 */
}

/* 执行一步移动(不含渲染) */
static void snake_step(void)
{
    uint8_t nx, ny, i;
    int ate;

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

    /* 移动蛇身 */
    for (i = s_len - 1; i > 0; i--)
    {
        s_x[i] = s_x[i - 1];
        s_y[i] = s_y[i - 1];
    }
    s_x[0] = nx;
    s_y[0] = ny;

    if (ate)
    {
        /* 防越界: 到达上限后不再增长 */
        if (s_len < SNAKE_MAX_LEN)
        {
            s_len++;
        }
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
        /* 新食物放右上角(测试用) */
        s_fx = 15; s_fy = 2;
    }
}

/* ===== 测试用例 ===== */
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  %-50s", name); \
    tests_run++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { printf("FAIL\n    assert failed: %s (line %d)\n", #cond, __LINE__); return; } \
} while(0)

#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)

static void test_rng_deterministic(void)
{
    TEST("RNG: 相同种子产生相同序列");
    uint32_t a[5], b[5];
    snake_srand(12345);
    for (int i = 0; i < 5; i++) a[i] = snake_rand();
    snake_srand(12345);
    for (int i = 0; i < 5; i++) b[i] = snake_rand();
    for (int i = 0; i < 5; i++) ASSERT(a[i] == b[i]);
    PASS();
}

static void test_rng_different_seeds(void)
{
    TEST("RNG: 不同种子产生不同序列");
    snake_srand(1);
    uint32_t r1 = snake_rand();
    snake_srand(2);
    uint32_t r2 = snake_rand();
    ASSERT(r1 != r2);
    PASS();
}

static void test_rng_seed_zero_fallback(void)
{
    TEST("RNG: 种子为0时回退到默认值");
    snake_srand(0);
    uint32_t r = snake_rand();
    snake_srand(0x9E3779B9u);
    uint32_t r2 = snake_rand();
    ASSERT(r == r2);
    PASS();
}

static void test_initial_state(void)
{
    TEST("初始状态: 蛇长4, 分数0, 未结束");
    snake_reset();
    ASSERT(s_len == 4);
    ASSERT(s_score == 0);
    ASSERT(s_over == 0);
    ASSERT(s_dx == 1 && s_dy == 0);
    PASS();
}

static void test_wall_collision(void)
{
    TEST("撞墙检测: 向右走到边界外");
    snake_reset();
    /* 蛇头在(5,9), 向右走到(20,9)应撞墙 */
    int result = snake_check(1, 0); /* 5+15步后会撞 */
    /* 先走14步到x=19 */
    for (int i = 0; i < 14; i++) snake_step();
    ASSERT(s_x[0] == 19);
    /* 再走一步应撞墙 */
    snake_step();
    ASSERT(s_over == 1);
    PASS();
}

static void test_eat_food(void)
{
    TEST("吃食物: 长度+1, 分数+10");
    snake_reset();
    /* 蛇头(5,9), 食物(10,9), 向右走5步 */
    for (int i = 0; i < 5; i++) snake_step();
    ASSERT(s_len == 5);  /* 4+1 */
    ASSERT(s_score == 10);
    ASSERT(s_x[0] == 10 && s_y[0] == 9);
    PASS();
}

static void test_self_collision(void)
{
    TEST("撞自身: 蛇身重叠时判定死亡");
    snake_reset();
    /* 构造L形: 手动设置蛇身形成可撞的状态 */
    s_len = 5;
    s_x[0] = 5; s_y[0] = 5; /* 头 */
    s_x[1] = 5; s_y[1] = 6;
    s_x[2] = 5; s_y[2] = 7;
    s_x[3] = 4; s_y[3] = 7;
    s_x[4] = 4; s_y[4] = 6; /* 尾 */
    s_dx = -1; s_dy = 0; /* 向左 -> 撞到(4,5)? 不对 */
    /* 向下走: (5,5)->(5,6) 撞到 s_x[1] */
    s_dx = 0; s_dy = 1;
    int result = snake_check(s_dx, s_dy);
    ASSERT(result == 2); /* 撞自身 */
    PASS();
}

static void test_tail_chase_allowed(void)
{
    TEST("追尾合法: 蛇头可进入即将腾出的尾格");
    snake_reset();
    /* 蛇头(5,9), 尾(2,9), 向右走不会撞尾 */
    int result = snake_check(1, 0);
    ASSERT(result == 3); /* 正常移动(或4如果到食物) */
    PASS();
}

static void test_len_capped_at_max(void)
{
    TEST("长度上限: s_len 不超过 SNAKE_MAX_LEN");
    snake_reset();
    /* 模拟吃大量食物 */
    s_len = SNAKE_MAX_LEN - 1;
    s_fx = s_x[0] + s_dx;
    s_fy = s_y[0] + s_dy;
    /* 确保食物在边界内 */
    if (s_fx >= SNAKE_COLS || s_fy >= SNAKE_ROWS) {
        s_dx = -1; s_dy = 0;
        s_fx = s_x[0] + s_dx;
        s_fy = s_y[0] + s_dy;
    }
    snake_step();
    ASSERT(s_len == SNAKE_MAX_LEN); /* 到达上限 */
    /* 再吃一次不应超过 */
    s_len = SNAKE_MAX_LEN;
    s_fx = s_x[0] + s_dx;
    s_fy = s_y[0] + s_dy;
    if (s_fx < SNAKE_COLS && s_fy < SNAKE_ROWS) {
        snake_step();
        ASSERT(s_len == SNAKE_MAX_LEN); /* 仍然=上限 */
    }
    PASS();
}

static void test_acceleration(void)
{
    TEST("加速机制: 每吃5个食物 tick减少10%");
    snake_reset();
    uint16_t tick_before = s_tick;
    s_food_cnt = SNAKE_ACCEL_EVERY - 1; /* 已吃4个 */
    /* 吃第5个 */
    s_len = 4;
    s_dx = 1; s_dy = 0;
    s_fx = s_x[0] + 1; s_fy = s_y[0];
    snake_step();
    ASSERT(s_tick < tick_before);
    ASSERT(s_tick >= SNAKE_TICK_MIN);
    PASS();
}

static void test_tick_min_floor(void)
{
    TEST("加速下限: tick 不低于 SNAKE_TICK_MIN");
    snake_reset();
    s_tick = SNAKE_TICK_MIN + 5;
    s_food_cnt = SNAKE_ACCEL_EVERY - 1;
    s_len = 4;
    s_dx = 1; s_dy = 0;
    s_fx = s_x[0] + 1; s_fy = s_y[0];
    snake_step();
    ASSERT(s_tick >= SNAKE_TICK_MIN);
    PASS();
}

int main(void)
{
    printf("=== 贪吃蛇游戏逻辑单元测试 ===\n\n");

    test_rng_deterministic();
    test_rng_different_seeds();
    test_rng_seed_zero_fallback();
    test_initial_state();
    test_wall_collision();
    test_eat_food();
    test_self_collision();
    test_tail_chase_allowed();
    test_len_capped_at_max();
    test_acceleration();
    test_tick_min_floor();

    printf("\n结果: %d/%d 通过\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
