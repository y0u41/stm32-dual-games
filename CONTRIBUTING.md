# 贡献指南

感谢你对本项目的关注！本文档说明如何参与开发。

## 开发环境

| 工具 | 版本 | 用途 |
|------|------|------|
| Keil MDK | 5.36+ | 固件编译（AC6 编译器） |
| Python | 3.10+ | 主机侧测试 |
| ST-Link | - | 程序下载与调试 |

## 快速开始

```bash
# 1. Fork 并克隆仓库
git clone https://github.com/<your-username>/stm32-dual-games.git
cd stm32-dual-games

# 2. 打开 Keil 工程
#    KeilProject/Projects/MDK-ARM/atk_f103.uvprojx

# 3. 运行主机侧测试
python tests/test_snake_logic.py
```

## 项目结构

```
├── KeilProject/
│   ├── Drivers/          # 底层驱动（勿改）
│   ├── User/             # 应用层（主要修改区域）
│   │   ├── main.c        # 主循环状态机
│   │   ├── menu.c        # 主菜单
│   │   ├── game_snake.c  # 贪吃蛇
│   │   └── game_gomoku.c # 五子棋
│   └── Projects/         # Keil 工程文件
├── tests/                # 主机侧测试
└── images/               # 实拍照片
```

## 开发流程

### 1. 创建分支

```bash
git checkout -b feature/your-feature-name
```

### 2. 修改代码

- **应用层代码**在 `KeilProject/User/` 目录
- **驱动层**（`Drivers/`）原则上不改，如需修改请说明原因
- 保持 C89/C99 兼容，避免使用过新的语言特性

### 3. 编译验证

```bat
:: 命令行编译（必须 0 Error / 0 Warning）
"MDK5.36\UV4\UV4.exe" -b KeilProject\Projects\MDK-ARM\atk_f103.uvprojx -j0 -o build.log
```

### 4. 运行测试

```bash
# 主机侧逻辑测试
python tests/test_snake_logic.py
```

### 5. 硬件验证

如有开发板，请验证：
- [ ] 编译通过（0 Error / 0 Warning）
- [ ] 下载运行正常
- [ ] 触摸操作响应正确
- [ ] 无内存泄漏（观察 SRAM 占用）

### 6. 提交代码

```bash
git add .
git commit -m "feat: 添加 XXX 功能"
git push origin feature/your-feature-name
```

**提交信息规范**：

| 前缀 | 说明 |
|------|------|
| `feat:` | 新功能 |
| `fix:` | 修复 Bug |
| `docs:` | 文档更新 |
| `test:` | 测试 |
| `refactor:` | 重构 |

## 代码规范

### C 语言规范
- 缩进：4 空格
- 大括号：同行放置
- 变量命名：小写下划线 `snake_len`
- 常量命名：全大写下划线 `SNAKE_MAX_LEN`
- 函数命名：模块前缀 `snake_step()`、`gomoku_place()`

### 内存规范
- **禁止动态内存分配**（malloc/free）
- 静态数组需指定最大长度
- 注意栈空间消耗（F103 仅 64KB SRAM）

### 命名约定

| 类型 | 规范 | 示例 |
|------|------|------|
| 全局变量 | 模块前缀 | `g_snake_x` |
| 局部变量 | 小驼峰或下划线 | `head_x`、`food_cnt` |
| 函数 | 模块前缀 | `menu_draw()` |
| 宏/常量 | 全大写 | `MENU_STATE_HOME` |

## Issue 反馈

提交 Issue 时请包含：

1. **硬件环境**：开发板型号、屏幕型号
2. **复现步骤**：操作流程
3. **编译日志**（如有报错）
4. **实拍照片**（如涉及界面问题）

## 行为准则

- 尊重他人，友善交流
- 聚焦技术讨论
- 欢迎新人提问
