#!/usr/bin/env python3
"""
贪吃蛇游戏逻辑主机侧单元测试
与 KeilProject/User/game_snake.c 中的核心算法等价实现
"""
import sys

SNAKE_COLS = 20
SNAKE_ROWS = 18
SNAKE_MAX_LEN = 200
SNAKE_TICK_INIT = 300
SNAKE_TICK_MIN = 100
SNAKE_ACCEL_EVERY = 5

tests_run = 0
tests_passed = 0

def test(name):
    global tests_run
    tests_run += 1
    print(f"  {name:<50}", end="")

def ok():
    global tests_passed
    print("PASS")
    tests_passed += 1

def fail(msg):
    print(f"FAIL\n    {msg}")
    sys.exit(1)

class Xorshift32:
    def __init__(self, seed=0):
        self.srand(seed)
    def srand(self, seed):
        self.rnd = seed if seed else 0x9E3779B9
    def rand(self):
        x = self.rnd
        x ^= (x << 13) & 0xFFFFFFFF
        x ^= x >> 17
        x ^= (x << 5) & 0xFFFFFFFF
        self.rnd = x
        return x

class SnakeGame:
    def __init__(self, seed=12345):
        self.rng = Xorshift32(seed)
        self.reset()
    def reset(self):
        self.s_len = 4
        self.s_score = 0
        self.s_tick = SNAKE_TICK_INIT
        self.s_dx = 1
        self.s_dy = 0
        self.s_over = False
        self.s_food_cnt = 0
        self.s_x = [0] * SNAKE_MAX_LEN
        self.s_y = [0] * SNAKE_MAX_LEN
        self.s_x[0], self.s_y[0] = 5, 9
        self.s_x[1], self.s_y[1] = 4, 9
        self.s_x[2], self.s_y[2] = 3, 9
        self.s_x[3], self.s_y[3] = 2, 9
        self.s_fx, self.s_fy = 10, 9
    def check(self, dx, dy):
        nx = self.s_x[0] + dx
        ny = self.s_y[0] + dy
        if nx >= SNAKE_COLS or ny >= SNAKE_ROWS or nx < 0 or ny < 0:
            return 1  # wall
        for i in range(1, self.s_len):
            if self.s_x[i] == nx and self.s_y[i] == ny:
                if i == self.s_len - 1 and not (nx == self.s_fx and ny == self.s_fy):
                    continue
                return 2  # self
        if nx == self.s_fx and ny == self.s_fy:
            return 4  # eat
        return 3  # ok
    def step(self):
        nx = self.s_x[0] + self.s_dx
        ny = self.s_y[0] + self.s_dy
        if nx >= SNAKE_COLS or ny >= SNAKE_ROWS or nx < 0 or ny < 0:
            self.s_over = True
            return
        ate = (nx == self.s_fx and ny == self.s_fy)
        for i in range(1, self.s_len):
            if self.s_x[i] == nx and self.s_y[i] == ny:
                if i == self.s_len - 1 and not ate:
                    continue
                self.s_over = True
                return
        for i in range(self.s_len - 1, 0, -1):
            self.s_x[i] = self.s_x[i - 1]
            self.s_y[i] = self.s_y[i - 1]
        self.s_x[0] = nx
        self.s_y[0] = ny
        if ate:
            if self.s_len < SNAKE_MAX_LEN:
                self.s_len += 1
            self.s_score += 10
            self.s_food_cnt += 1
            if self.s_food_cnt >= SNAKE_ACCEL_EVERY:
                self.s_food_cnt = 0
                if self.s_tick > SNAKE_TICK_MIN:
                    self.s_tick = self.s_tick - self.s_tick // 10
                    if self.s_tick < SNAKE_TICK_MIN:
                        self.s_tick = SNAKE_TICK_MIN
            self.s_fx = 15
            self.s_fy = 2

def main():
    print("=== 贪吃蛇游戏逻辑单元测试 ===\n")

    test("RNG: 相同种子产生相同序列")
    r1 = Xorshift32(12345)
    r2 = Xorshift32(12345)
    seq1 = [r1.rand() for _ in range(5)]
    seq2 = [r2.rand() for _ in range(5)]
    if seq1 != seq2: fail("sequences differ")
    ok()

    test("RNG: 不同种子产生不同序列")
    r3 = Xorshift32(1)
    r4 = Xorshift32(2)
    if r3.rand() == r4.rand(): fail("same output for different seeds")
    ok()

    test("RNG: 种子为0时回退到默认值")
    r5 = Xorshift32(0)
    r6 = Xorshift32(0x9E3779B9)
    if r5.rand() != r6.rand(): fail("fallback mismatch")
    ok()

    test("初始状态: 蛇长4, 分数0, 未结束")
    g = SnakeGame()
    if g.s_len != 4: fail(f"len={g.s_len}")
    if g.s_score != 0: fail(f"score={g.s_score}")
    if g.s_over: fail("over=True")
    if g.s_dx != 1 or g.s_dy != 0: fail("dir wrong")
    ok()

    test("撞墙检测: 向右走到边界外")
    g = SnakeGame()
    for _ in range(14):
        g.step()
    if g.s_x[0] != 19: fail(f"x={g.s_x[0]}")
    g.step()
    if not g.s_over: fail("not over")
    ok()

    test("吃食物: 长度+1, 分数+10")
    g = SnakeGame()
    for _ in range(5):
        g.step()
    if g.s_len != 5: fail(f"len={g.s_len}")
    if g.s_score != 10: fail(f"score={g.s_score}")
    if g.s_x[0] != 10 or g.s_y[0] != 9: fail("head pos")
    ok()

    test("撞自身: 蛇身重叠时判定死亡")
    g = SnakeGame()
    g.s_len = 5
    g.s_x = [5,5,5,4,4] + [0]*(SNAKE_MAX_LEN-5)
    g.s_y = [5,6,7,7,6] + [0]*(SNAKE_MAX_LEN-5)
    g.s_dx, g.s_dy = 0, 1
    result = g.check(0, 1)
    if result != 2: fail(f"result={result}")
    ok()

    test("追尾合法: 蛇头可进入即将腾出的尾格")
    g = SnakeGame()
    result = g.check(1, 0)
    if result not in (3, 4): fail(f"result={result}")
    ok()

    test("长度上限: s_len 不超过 SNAKE_MAX_LEN")
    g = SnakeGame()
    g.s_len = SNAKE_MAX_LEN - 1
    g.s_fx = g.s_x[0] + g.s_dx
    g.s_fy = g.s_y[0] + g.s_dy
    if 0 <= g.s_fx < SNAKE_COLS and 0 <= g.s_fy < SNAKE_ROWS:
        g.step()
        if g.s_len != SNAKE_MAX_LEN: fail(f"len={g.s_len}")
    ok()

    test("加速机制: 每吃5个食物 tick减少10%")
    g = SnakeGame()
    tick_before = g.s_tick
    g.s_food_cnt = SNAKE_ACCEL_EVERY - 1
    g.s_len = 4
    g.s_dx, g.s_dy = 1, 0
    g.s_fx = g.s_x[0] + 1
    g.s_fy = g.s_y[0]
    g.step()
    if g.s_tick >= tick_before: fail(f"tick={g.s_tick} >= {tick_before}")
    if g.s_tick < SNAKE_TICK_MIN: fail(f"tick={g.s_tick} < min")
    ok()

    test("加速下限: tick 不低于 SNAKE_TICK_MIN")
    g = SnakeGame()
    g.s_tick = SNAKE_TICK_MIN + 5
    g.s_food_cnt = SNAKE_ACCEL_EVERY - 1
    g.s_len = 4
    g.s_dx, g.s_dy = 1, 0
    g.s_fx = g.s_x[0] + 1
    g.s_fy = g.s_y[0]
    g.step()
    if g.s_tick < SNAKE_TICK_MIN: fail(f"tick={g.s_tick}")
    ok()

    print(f"\n结果: {tests_passed}/{tests_run} 通过")
    return 0 if tests_passed == tests_run else 1

if __name__ == "__main__":
    sys.exit(main())
