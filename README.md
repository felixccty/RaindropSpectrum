# RaindropSpectrum

> 基于 Qt 5 / Qt Charts / QCustomPlot 的雨滴谱仪上位机控制与数据处理软件。

[![Platform](https://img.shields.io/badge/platform-Windows%20x64-blue.svg)](https://www.microsoft.com/windows)
[![Language](https://img.shields.io/badge/language-C%2B%2B-orange.svg)](https://isocpp.org/)
[![Framework](https://img.shields.io/badge/framework-Qt%205-green.svg)](https://www.qt.io/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

`RaindropSpectrum` 是一套面向激光雨滴谱观测的**数据采集与可视化上位机**。软件通过 `USB9825` 数据采集卡读取光电探测器输出信号，以阈值穿越法实时识别单个雨滴，提取雨滴信号幅度（对应雨滴大小）与脉冲持续时长（对应雨滴速度），并以**实时波形曲线**、**雨滴谱彩色图**（幅度 × 速度二维分布）与**每分钟雨量柱状图**三种视图同步展示，同时将每次雨滴事件记录落盘为 CSV，便于后续统计分析与离线复核。界面为深色风格，支持谱图拖拽/缩放与鼠标坐标读数，适合观测站现场采集与实验分析。

数据以 CSV 形式写入 `C:\rain_data\Rain_recordYYYY-MM-DD.csv`，采集卡 SDK 由第三方提供，便于与既有观测系统集成。

---

## 核心特性

- **采集卡连接**：`connect_to_card()` 扫描 0–31 号设备并调用 `USB9825_Link()` 建立连接，按钮反馈「连接成功 / 连接失败」。
- **连续采集**：`read_consecutive()` 通过 `USB9825_PARA_INIT` 配置后触发采集（外上升沿触发、内部时钟、单通道、触发长度 1024、触发电平 2048），并以软触发启动。
- **实时信号波形**：`QSplineSeries`（`spY1`）+ `QValueAxis`（`vaX` / `vaY1`）绘制滑动窗口波形，`QCategoryAxis`（`axis_th`）叠加阈值参考线。
- **雨滴识别与测速**：`timeSlot()` 以 `background_sg`（默认 2200）为阈值检测下降沿与回升沿，记录雨滴最低幅度 `lowest_v` 与持续时长 `duration`，并维护 `rain_counter` 计数。
- **雨滴谱彩色图**：`show_spectrum()` 用 `QCustomPlot` 的 `QCPColorMap` 绘制「幅度 × 速度」二维热图，颜色梯度 `#f6efa6` → `#bf444c`，支持拖拽与缩放。
- **每分钟雨量统计**：`timeSlot_ra()` / `show_rain_amount()` 用 `QBarSeries`（`drop_set` / `series_drop`）绘制按分钟聚合的雨滴计数柱状图，并给出晴/小雨/中雨/大雨/暴雨分级指示。
- **阈值在线调整**：`timeSlot_signal_ui()` 由 `spinBox_3` 修改阈值并同步更新参考线位置。
- **鼠标交互**：`mouseMoved()` / `showPointToolTip()` 显示谱图光标处的幅度与速度读数。
- **数据落盘**：每次雨滴事件写入时间戳、`lowest_v` 与 `duration` 到 `C:\rain_data\Rain_recordYYYY-MM-DD.csv`。

---

## 技术栈

| 类别 | 选型 |
|------|------|
| 语言 | C++11（MSVC `v142` 工具集） |
| UI 框架 | Qt 5（Widgets / Charts / PrintSupport） |
| 绘图库 | QCustomPlot（`QCPColorMap` 雨滴谱热图） |
| 开发工具 | Visual Studio 2019 + Qt VS Tools |
| 采集硬件 | USB9825 数据采集卡（`USB9825.h` / `USB9825.lib`） |
| 数据格式 | CSV（雨滴事件记录） |
| 操作系统 | Windows 7 / 10 x64 |
| 构建产物 | `raindrop_spectrum.exe` |

---

## 快速开始

```bash
git clone https://github.com/felixccty/RaindropSpectrum.git
cd RaindropSpectrum
```

1. 安装 Visual Studio 2019（C++ 桌面开发负载）与 Qt 5 + Qt VS Tools。
2. 准备 `QCustomPlot` 源码（`qcustomplot.cpp` / `qcustomplot.h`）并放入 `src/`，同时获取 `USB9825.lib` 与采集卡驱动。
3. 用 VS 打开 `src/raindrop_spectrum.sln`，选择 `Debug | x64` 编译。
4. 确认采集卡已接入，运行 `raindrop_spectrum.exe`，点击「连接」建立设备连接。
5. 点击「开始采集」进入采集态，观察波形、雨滴谱热图与雨量统计（见 [使用示例](docs/05-usage.md)）。

完整步骤见 [运行与部署](docs/03-build-and-run.md) 与 [依赖配置](docs/04-dependencies.md)。

---

## 目录结构

```
RaindropSpectrum/
├── README.md
├── LICENSE / CHANGELOG.md / CONTRIBUTING.md / .gitignore
├── src/                           # 核心源码（GBK 编码，含中文注释）
│   ├── main.cpp                   # 程序入口，初始化并显示主窗口
│   ├── raindrop_spectrum.h        # 主窗口类声明（图表、采集卡、定时器成员）
│   ├── raindrop_spectrum.cpp      # 业务逻辑：采集、雨滴识别、谱图、统计与存储
│   ├── raindrop_spectrum.ui       # Qt Designer 界面定义
│   ├── raindrop_spectrum.qrc      # Qt 资源描述（当前为空）
│   ├── USB9825.h                  # 采集卡 SDK 头文件与 API 声明
│   ├── raindrop_spectrum.sln      # VS 解决方案
│   ├── raindrop_spectrum.vcxproj(.filters)
│   └── qcustomplot.cpp/.h         # 第三方绘图库（需自行准备，未随包分发）
├── resources/samples/             # 示例雨滴记录 CSV
├── docs/                          # 本文档体系 + images/ 示意图
└── (本地按需) USB9825.lib / 采集卡驱动 / C:\rain_data 数据目录
```

---

## 文档索引

| 文档 | 内容 |
|------|------|
| [功能说明](docs/01-features.md) | 设备连接、连续采集、雨滴识别、谱图、雨量统计与数据落盘 |
| [目录结构](docs/02-structure.md) | 源文件职责、关键成员与数据流 |
| [运行与部署](docs/03-build-and-run.md) | 编译、配置、运行与 windeployqt 打包 |
| [依赖配置](docs/04-dependencies.md) | Qt 模块、QCustomPlot、采集卡 SDK 与系统要求 |
| [使用示例](docs/05-usage.md) | 接口配置、CSV 数据格式与典型操作 |
| [注意事项](docs/06-notes.md) | 编码、固定路径、硬件依赖与已知限制 |
| [架构与流程](docs/07-architecture.md) | 分层架构与关键流程图解 |

## 数据来源说明

本项目仅提供**数据采集、识别与可视化框架**。雨滴谱的物理标定（幅度→雨滴直径、脉冲时长→雨滴速度）需由使用者依据传感器特性自行标定；程序记录的 `lowest_v` 与 `duration` 为原始量值。详见 [使用示例](docs/05-usage.md)。

---

## 许可与作者

- **许可**：[MIT License](LICENSE)
- **作者 / 项目归属**：Felix
- **仓库**：https://github.com/felixccty/RaindropSpectrum

本项目以学习和工程交流为目的开源，欢迎 Issue 与 Pull Request。使用前请自行评估其在目标场景下的适用性与安全性。
