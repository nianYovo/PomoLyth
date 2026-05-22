# PomoLyth

PomoLyth 是一个基于 **C++ / Qt Widgets** 的智能桌宠番茄钟专注系统。它把番茄钟、桌宠情绪状态、AI 任务规划、分心检测、本地数据记录、成就成长和 Dashboard 统计整合到一个桌面应用里。

项目当前默认使用 Mock AI，因此不需要 API Key 就可以完整运行；如果需要，也可以在设置中切换到 OpenAI-compatible API。

## 项目定位

传统番茄钟通常只负责倒计时。PomoLyth 希望提供一个更完整的专注闭环：

```text
输入任务
-> AI 拆解本轮目标
-> 开始番茄钟
-> 桌宠进入陪伴状态
-> 检测窗口分心行为
-> 结束后填写复盘
-> AI 生成复盘总结
-> 保存专注记录
-> 更新桌宠经验、等级和成就
-> Dashboard 展示长期数据和日报
```

它更像一个“会记录、会提醒、会成长”的桌面专注伙伴，而不是单纯的计时器。

## 当前功能

- 任务输入与 AI 任务规划
- Mock AI 默认兜底，无需联网即可运行
- 可选 OpenAI-compatible Chat Completions 客户端
- AI 任务规划、复盘、日报均在后台线程执行，避免 UI 卡死
- 番茄钟开始、暂停、继续、结束
- 完成复盘后自动进入短休息
- 事件驱动的桌宠情绪状态机
- 主界面桌宠展示
- 无边框、置顶、可拖动的悬浮桌宠窗口
- Windows 前台窗口检测
- 黑名单 / 白名单分心识别
- 系统托盘菜单和桌面通知
- 设置窗口，可配置时长、检测间隔、AI Provider、黑白名单
- SQLite 本地保存专注记录
- SQLite 本地保存分心事件
- SQLite 本地保存成就和日报
- 桌宠经验、等级、亲密度、累计专注数据
- Dashboard 展示今日、近 7 天、分心源、成就、日报、长期记忆
- UserMemory 根据历史数据生成长期专注摘要
- CSV 导出专注记录
- 关闭主窗口时隐藏到托盘，避免误关
- 结束当前计时时有二次确认

## 技术栈

- 语言：C++17
- GUI：Qt 6 Widgets
- 构建：CMake
- 本地数据库：SQLite，使用 Qt Sql 模块
- 网络请求：Qt Network
- 后台任务：Qt Concurrent
- Windows 窗口检测：Win32 API

## 目录结构

```text
PomoLyth/
├── app/                 应用装配、入口、系统托盘
├── ai/                  AI 接口、Mock AI、OpenAI 客户端、Prompt、规划和复盘
├── core/                计时器、事件总线、会话管理、桌宠状态机、成就系统
├── models/              FocusTask、FocusSession、PetProfile、AppEvent 等数据结构
├── monitor/             前台窗口检测、分心识别
├── storage/             SQLite、JSON 配置、UserMemory
├── ui/                  主窗口、计时面板、桌宠、复盘窗口、Dashboard、设置窗口
├── config/              默认配置、黑名单、白名单
├── assets/              资源目录，当前预留
├── CMakeLists.txt
└── README.md
```

## 架构说明

项目采用分层和事件驱动设计。

### UI 层

负责窗口展示和用户交互，不直接处理复杂业务逻辑。

主要类：

- `MainWindow`
- `TimerPanel`
- `PetWidget`
- `PetWindow`
- `ReviewDialog`
- `DashboardWindow`
- `SettingsDialog`

### 核心业务层

负责番茄钟生命周期、专注会话、桌宠状态和成就成长。

主要类：

- `PomodoroTimer`
- `FocusSessionManager`
- `PetStateMachine`
- `AchievementSystem`
- `EventBus`

### AI 层

通过 `IAiClient` 抽象 AI 能力。

当前实现：

- `MockAiClient`：默认启用，不需要 API Key
- `OpenAiClient`：可选启用，兼容 OpenAI Chat Completions 接口

业务封装：

- `TaskPlanner`
- `ReviewGenerator`
- `PromptBuilder`

### 监控层

负责检测当前前台窗口，并根据黑白名单判断是否分心。

主要类：

- `IWindowMonitor`
- `WindowsWindowMonitor`
- `FocusMonitor`

### 存储层

负责配置和本地数据持久化。

主要类：

- `SQLiteStorage`
- `JsonStorage`
- `UserMemory`

## 数据存储

程序会使用 SQLite 保存以下数据：

- `focus_sessions`：专注记录
- `distraction_events`：分心事件
- `pet_profile`：桌宠等级、经验、亲密度等
- `achievements`：已解锁成就
- `daily_reports`：每日专注报告

数据库文件默认保存在 Qt 的 `AppDataLocation` 目录中，具体路径由系统和 Qt 决定。

配置文件位于：

```text
config/app_config.json
config/blacklist.json
config/whitelist.json
```

## 构建环境

当前项目在以下环境验证通过：

- Windows
- Qt 6.11.0 MinGW 64-bit
- MinGW 13.1
- CMake 3.30+

## 构建命令

在项目根目录运行：

```powershell
cmake -S . -B build -G "MinGW Makefiles" `
  -DCMAKE_PREFIX_PATH=E:/Qt/6.11.0/mingw_64 `
  -DCMAKE_CXX_COMPILER=E:/Qt/Tools/mingw1310_64/bin/g++.exe

cmake --build build -j 4
```

运行：

```powershell
.\build\PomoLyth.exe
```

如果直接运行时提示找不到 Qt DLL，可以临时把 Qt bin 加入 `PATH`：

```powershell
$env:PATH = "E:\Qt\6.11.0\mingw_64\bin;E:\Qt\Tools\mingw1310_64\bin;" + $env:PATH
.\build\PomoLyth.exe
```

或者使用 `windeployqt` 打包运行依赖。

## VSCode 配置

项目已包含：

```text
.vscode/c_cpp_properties.json
.vscode/settings.json
```

并在 CMake 中开启：

```cmake
CMAKE_EXPORT_COMPILE_COMMANDS ON
```

如果 VSCode 仍然出现大量 IntelliSense 红线，请执行：

1. `Ctrl + Shift + P`
2. 运行 `C/C++: Reset IntelliSense Database`
3. 运行 `Developer: Reload Window`
4. 确认打开的是项目根目录 `PomoLyth`
5. 确认 C/C++ 配置为 `PomoLyth-Qt-MinGW`

注意：命令行 `cmake --build build -j 4` 通过，才代表真实编译通过；VSCode 红线很多时候只是 IntelliSense 配置问题。

## AI Provider 配置

默认配置使用 Mock：

```json
{
  "aiProvider": "mock"
}
```

这种模式无需联网、无需 API Key，适合开发和演示基础流程。

如果要使用 OpenAI-compatible API：

1. 设置环境变量：

```powershell
$env:OPENAI_API_KEY="your_api_key"
```

2. 打开应用设置，将：

```text
AI provider = openai
OpenAI base URL = https://api.openai.com/v1
OpenAI model = 你要使用的模型
API key env var = OPENAI_API_KEY
```

3. 重启应用。

说明：

- API Key 不会写入配置文件。
- 程序只保存环境变量名。
- 如果没有 API Key 或请求失败，AI 层会返回空结果，业务层仍有默认兜底内容。

## 使用流程

1. 打开 PomoLyth。
2. 输入本轮任务，例如：

```text
Review C++ STL vector and map
```

3. 点击 `Plan` 生成任务计划。
4. 点击 `Start` 开始番茄钟。
5. 专注期间桌宠会切换到专注状态。
6. 如果打开命中黑名单的窗口，会记录分心事件并触发提醒。
7. 计时结束后填写复盘。
8. AI 生成复盘总结。
9. 系统保存专注记录，更新桌宠成长和成就。
10. 自动进入短休息。
11. 在 Dashboard 查看统计、成就、日报和长期记忆。

## Dashboard 内容

Dashboard 当前展示：

- 今日专注分钟
- 今日完成番茄数
- 近 7 天专注分钟
- 平均分心次数
- 常见分心源
- UserMemory 长期记忆摘要
- 最近日报
- 最近专注记录
- 最近分心记录
- 最近解锁成就

并支持：

- 生成今日报告
- 导出 CSV

## 分心检测

分心检测基于 Windows 前台窗口标题和进程名。

专注期间还会监测键盘和鼠标活动状态：

- 长时间没有输入时提醒用户确认是否仍在专注


黑名单示例：

```json
[
  "bilibili",
  "youtube",
  "steam",
  "tiktok",
  "douyin",
  "game",
  "video"
]
```

白名单示例：

```json
[
  "Qt Creator",
  "Visual Studio Code",
  "CLion",
  "Microsoft Word",
  "Obsidian"
]
```

规则：

- 命中白名单：不判定为分心
- 未命中白名单但命中黑名单：记录为分心
- 检测间隔可在设置中调整

## 当前限制

- 当前 UI 以功能完整为主，还不是最终视觉设计。
- 桌宠目前使用代码绘制的基础形象，暂未接入 PNG / GIF / Live2D。
- OpenAI 客户端当前使用 Chat Completions 兼容接口。
- OpenAI 请求已放到后台线程，但当前没有流式输出。
- 分心检测目前优先支持 Windows。
- 数据库迁移目前较简单，适合开发阶段使用。

## 后续计划

- 增加真实桌宠素材和动画
- 增加皮肤系统
- 增加更完整的成就面板
- 增加趋势图表
- 增加日报历史查看窗口
- 增加应用打包脚本
- 增加单元测试
- 增加更完善的数据库迁移
- 支持更多 AI Provider
- 支持更细的任务分类和标签

## 一句话总结

PomoLyth 不是一个普通番茄钟，而是一个带有桌宠陪伴、AI 规划复盘、分心检测、成长系统和长期记忆的 C++ / Qt 桌面专注工具。
