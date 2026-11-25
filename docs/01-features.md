# 功能说明

本文档逐项说明 `RaindropSpectrum` 的实际功能，名称与源码一致。

## 1. 采集卡连接与采集控制

| 功能 | 关键函数 | 说明 |
|------|----------|------|
| 设备连接 | `connect_to_card()` | 遍历 0–31 号设备，调用 `USB9825_Link(i)` 建立连接，按钮显示「连接成功 / 连接失败」，记录 `devNum` |
| 连续采集 | `read_consecutive()` | 配置 `USB9825_PARA_INIT` 后 `USB9825_InitAD()` + `USB9825_ExeSoftTrig()` 启动采集，并初始化图表、定时器与谱图 |
| 停止采集 | `stop_read()` | `USB9825_StopAD()` 停止采集，停止 `timer1` 并释放 `inBuffer` |
| 延时辅助 | `timer(int ms)` | 逐毫秒 `Sleep(1)` 并 `qApp->processEvents()`，用于采集循环中保持界面响应 |

采集参数为固定约定：后触发（`TRIG_MODE_POST`）、外部上升沿触发（`TRIG_SRC_EXT_RISING`）、内部时钟（`ADCLK_INT`）、单通道、触发长度 1024、触发延迟 100、触发电平 2048、直接二进制格式（`ADFMT_STBIN`）。

## 2. 实时信号波形

- `read_consecutive()` 中创建 `QChartView`（`mChart`）与 `QSplineSeries`（`spY1`），深色主题 `ChartThemeDark`，隐藏图例。
- `QValueAxis`：`vaX` 为样本索引轴（0–`MaxSample`，即 0–200），`vaY1` 为采集电压轴（2000–2400），两者标签分别以绿/红着色。
- `QCategoryAxis`（`axis_th`）以 `background_sg` 为阈值参考线，虚线网格样式 `[QPen(QColor(0,170,140),1.2,Qt::DotLine)]`。
- `spY1->setUseOpenGL()` 启用 OpenGL 加速绘制。
- `timeSlot()` 以 `vec_sg` 维护 200 点滑动窗口：`index <= MaxSample` 时追加采样点，超出后整体左移并写入最新值，最后 `spY1->replace(vec_sg)` 刷新波形。

## 3. 雨滴识别与计数

`timeSlot()`（`timer1` 驱动，间隔约 0.1 ms）是核心识别逻辑：

- `USB9825_ReadAD(hDevice, inBuffer, samcnt)` 读取 `samcnt = 1024 * 100 * m_lChcnt` 个采样点到 `inBuffer`；
- **下降沿检测**：当 `inBuffer[index] < background_sg` 且 `check_drop == true`，认定一个雨滴信号到来，`start = clock()` 记录起始时刻，重置 `lowest_v = 2500`，`rain_counter++`，`check_drop = false`；
- **幅度跟踪**：检测期间持续用更低值更新 `lowest_v`；
- **回升检测**：当 `inBuffer[index] >= background_sg` 且 `check_drop == false`，认定信号结束，`finish = clock()` 计算持续时长 `duration = finish - start`，`check_drop = true`；
- 界面 `label_8`（每分钟数）、`label_9`（最低幅度）、`label_11`（时长）实时更新；
- 阈值由 `spinBox_3` 经 `timeSlot_signal_ui()` 在线调整，并同步移动 `axis_th` 参考线。

## 4. 雨滴谱彩色图（幅度 × 速度）

- `show_spectrum()` 配置 `QCustomPlot` 的 `QCPColorMap` 与 `QCPColorScale`：
  - X 轴为信号幅度/电压（范围 2000–2400），Y 轴为雨滴速度（范围 0–200）；
  - `x_axis` / `y_axis` 分别以步长 2 生成坐标轴标签序列；
  - 颜色梯度由 `#f6efa6` 过渡到 `#bf444c`，`fillAlpha(0)` 初始化透明；
  - 深灰背景 `#433D3C`，坐标轴、刻度、网格以白色/点线绘制；
  - `setInteractions(QCP::iRangeDrag | QCP::iRangeZoom)` 支持拖拽与滚轮缩放；
  - `QCPMarginGroup` 令色标与主轴矩形边距对齐。
- 每次雨滴信号结束时，`timeSlot()` 调用 `setAlpha(lowest_v - 2000, duration, 255)` 与 `setCell(lowest_v - 2000, duration, 1)` 在对应网格累积计数，并 `replot()` 刷新热图。

## 5. 每分钟雨量统计

- `timeSlot_ra()`（`timer_ra` 驱动，间隔 1000 ms）检测系统分钟变化：分钟翻转时累加 `max_amount`，将 `rain_counter` 追加到 `drop_set`（`QBarSet`），写入 `series_drop`（`QBarSeries`），重置 `rain_counter` 并递增 `max_drop_time_axis`；`> max_ra_axis`（30）时移除最旧项保持窗口。
- `show_rain_amount(QBarSeries* series_ra)` 用 `chart_ra` 绘制柱状图：
  - 标题「雨量统计」，`QBarCategoryAxis`（`axisX_ra`）为分钟时间轴（标题含当前时分），`QValueAxis`（`axisY_ra`）为雨滴计数轴（0–`max_amount`）；
  - 深色主题、`QRgb(0xd18952)` 轴线、白色标签与浅青阴影，柱内显示数值标签，隐藏图例；
  - `label_12` 按 `rain_counter` 显示天气分级：`0` 为晴，`<5` 小雨，`<15` 中雨，`<30` 大雨，其余为暴雨，并切换背景色。

## 6. 鼠标交互与状态显示

- `mouseMoved(QMouseEvent*)`：`widget_spectrum` 鼠标移动时，通过 `pixelToCoord()` 将像素坐标转换为幅度/速度值，更新 `label`（直径）与 `label_2`（速度）。
- `showPointToolTip(QMouseEvent*)`：将当前坐标格式化为 ToolTip 文本。
- 构造函数中设置 `widget_spectrum->setMouseTracking(true)` 并连接 `mouseMove` 信号。
- `label_5` 显示当前采样索引 `index`，`label_13` / `label_22` 显示日期与时刻。

## 7. 数据记录与初始化

- 构造函数 `raindrop_spectrum()` 调用 `mkdir("C:\\rain_data")` 创建数据目录，并以当天日期生成 `Rain_recordYYYY-MM-DD.csv`，写入表头：
  ```text
  开启时间：,YYYY-MM-DD HH:MM:SS
   时间,雨滴最低幅度(V)(雨滴大小),雨滴速度
  ```
- 每次雨滴信号结束时追加一行：`时间戳,lowest_v,duration`。
- `time_list`（`QStringList`）维护柱状图 X 轴分类标签，超过 `max_ra_axis` 时移除最早项。
