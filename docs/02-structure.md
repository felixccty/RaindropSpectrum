# 目录结构

## 发布包目录

```
RaindropSpectrum/
├── README.md / LICENSE / CHANGELOG.md / CONTRIBUTING.md / .gitignore
├── src/                           # 核心源码
│   ├── main.cpp                   # 程序入口
│   ├── raindrop_spectrum.h/.cpp   # 主窗口类声明与业务逻辑
│   ├── raindrop_spectrum.ui       # Qt Designer 界面定义
│   ├── raindrop_spectrum.qrc      # Qt 资源描述（当前为空）
│   ├── USB9825.h                  # 采集卡 SDK 头文件
│   ├── raindrop_spectrum.sln/.vcxproj/.filters
│   └── qcustomplot.cpp/.h         # 第三方绘图库（需另行准备）
├── resources/samples/             # 示例雨滴记录 CSV
├── docs/                          # 功能/结构/构建/依赖/使用/注意/架构 文档
│   └── images/                    # 架构图与流程图
└── (本地按需) USB9825.lib / 驱动 / C:\rain_data 数据目录
```

## 关键类与成员

| 名称 | 类型 | 职责 |
|------|------|------|
| `raindrop_spectrum` | `QMainWindow` 子类 | 主窗口，聚合采集、识别、图表与存储逻辑 |
| `hDevice` | `HANDLE` | 采集卡设备句柄，由 `USB9825_Link()` 返回 |
| `devNum` | `UCHAR` | 当前连接的设备号（0–31） |
| `inBuffer` / `samcnt` | `PUSHORT` / `ULONG` | 采集数据缓冲与样本数（单通道为 102400） |
| `ADRun` | `BOOL` | 采集运行标志，`stop_read()` 置 false |
| `chart_sg` / `mChart` | `QChart*` / `QChartView*` | 实时信号曲线图表与视图 |
| `spY1` / `vec_sg` | `QSplineSeries*` / `QVector<QPointF>` | 波形曲线与 200 点滑动窗口数据 |
| `vaX` / `vaY1` / `axis_th` | `QValueAxis*` / `QCategoryAxis*` | 样本索引轴、电压轴与阈值参考轴 |
| `chart_ra` / `series_drop` / `drop_set` | `QChart*` / `QBarSeries*` / `QBarSet*` | 每分钟雨量柱状图及其数据集 |
| `axisX_ra` / `axisY_ra` / `time_list` | `QBarCategoryAxis*` / `QValueAxis*` / `QStringList*` | 柱状图时间轴、计数轴与分类标签 |
| `colorMap` / `colorScale` / `marginGroup` | `QCPColorMap*` / `QCPColorScale*` / `QCPMarginGroup*` | 雨滴谱热图、色标与边距同步 |
| `timer1` / `timer_ra` | `QTimer*` | 雨滴识别触发（约 0.1 ms）与分钟统计（1000 ms） |
| `background_sg` / `lowest_v` | `int` | 检测阈值（默认 2200）与当前雨滴最低幅度 |
| `rain_counter` / `max_amount` | `int` | 当前分钟内雨滴计数与历史最大计数 |
| `check_drop` | `bool` | 雨滴信号检测状态（下降沿/回升沿判别） |
| `max_ra_axis` / `MaxSample` | `int` | 柱状图时间窗（30 分钟）与波形窗口（200 点） |

## 采集卡 API（`USB9825.h`）

| 函数 | 用途 |
|------|------|
| `USB9825_Link(UCHAR DeviceNO)` | 连接指定编号设备，返回句柄 |
| `USB9825_InitAD(HANDLE, PUSB9825_PARA_INIT)` | 按参数结构初始化 AD 采集 |
| `USB9825_ExeSoftTrig(HANDLE)` | 触发软启动 |
| `USB9825_ReadAD(HANDLE, PUSHORT, ULONG)` | 读取指定长度的 AD 数据 |
| `USB9825_StopAD(HANDLE, UCHAR)` | 停止采集 |
| `USB9825_UnLink(HANDLE)` | 断开设备 |

头文件还定义了 `USB9825_PARA_INIT` 结构与触发模式/触发源/数据格式等枚举常量。

## 数据流

```
USB9825 采集卡 ──USB9825_ReadAD()──> inBuffer
                                      │ (阈值穿越检测)
                    ┌─────────────────┼──────────────────┐
                    ▼                 ▼                  ▼
              spY1 波形曲线     colorMap 雨滴谱热图   rain_counter 计数
              (实时信号)        (幅度 × 速度)              │
                    │                 │            timeSlot_ra() 按分钟聚合
                    └──── 图表/鼠标交互联动 ────┘              │
                                                     QBarSeries 柱状图
                                      │
                        fprintf() ──> C:\rain_data\Rain_recordYYYY-MM-DD.csv
```

配置文件无独立 INI：采集参数在 `read_consecutive()` 中以 `USB9825_PARA_INIT` 结构固定设置，运行数据目录为 `C:\rain_data`（见 [注意事项](06-notes.md)）。
