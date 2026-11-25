# 使用示例

## 1. 采集参数

采集参数在 `read_consecutive()` 中通过 `USB9825_PARA_INIT` 结构固定配置（修改需改源码并重新编译）：

```cpp
para_init.TriggerMode    = TRIG_MODE_POST;       // 后触发采集
para_init.TriggerSource  = TRIG_SRC_EXT_RISING;  // 外部上升沿触发
para_init.lSelADClk      = ADCLK_INT;            // 内部时钟
para_init.lChCnt         = 1;                    // 单通道
para_init.ClkDeci        = 0;                    // 不分频
para_init.TriggerLength  = 1024;                 // 触发长度
para_init.TriggerDelay   = 100;                  // 触发延迟
para_init.TriggerLevel   = 2048;                 // 模拟触发电平
para_init.lADFmt         = ADFMT_STBIN;          // 直接二进制输出
```

采集缓冲大小为 `samcnt = 1024 * 100 * m_lChcnt`（单通道即 102400 点），由 `USB9825_ReadAD()` 周期读取。

## 2. 雨滴记录 CSV 格式

程序在 `C:\rain_data` 下按日期生成文件，表头由构造函数写入：

```csv
开启时间：,2021-06-30 17:22:50
 时间,雨滴最低幅度(V)(雨滴大小),雨滴速度
```

每次识别到一个雨滴信号（下降沿到回升沿）追加一行；示例见 `resources/samples/Rain_record.csv` 与 `a.csv`：

```csv
2021-06-30 17:07:55,1930,40
2021-06-30 17:07:55,1852,19
2021-06-30 17:07:55,1836,29
```

| 列 | 含义 | 代码来源 |
|----|------|----------|
| 时间 | 雨滴信号结束时的系统时间 | `strftime("%G-%m-%d %H:%M:%S")` |
| 雨滴最低幅度(V)(雨滴大小) | 信号低于阈值期间的最低电压 `lowest_v` | `timeSlot()` 中持续取最小值 |
| 雨滴速度 | 信号持续时长 `duration`（`clock()` 计时差值） | `finish - start` |

> `lowest_v` 与 `duration` 为原始量值，物理标定（幅度→直径、时长→速度）需使用者按传感器特性另行确定，详见 [数据来源说明](../README.md#数据来源说明)。

## 3. 典型操作流程

1. 启动程序，确认 `C:\rain_data` 可写；程序自动调用 `connect_to_card()` 与 `read_consecutive()` 完成初始化。
2. 检查「连接」按钮状态；若显示「连接失败」，排查驱动与设备占用后再次点击。
3. 点击「开始采集」进入连续采集态，右侧波形窗口实时滚动显示 200 点信号滑动窗口。
4. 雨滴信号到来时（幅度低于 `background_sg`），「每分钟数」「最低幅度」「时长」等标签实时更新，谱图热图在对应网格累加。
5. 通过 `spinBox_3`（「阈值」）调整检测阈值，阈值参考线随之移动；触发频率、触发长度等输入控件预留于界面。
6. 在雨滴谱图上拖拽/滚轮缩放，鼠标移动时读取光标处的幅度与速度。
7. 每分钟翻转时，`timeSlot_ra()` 将 `rain_counter` 写入柱状图，并按 `rain_counter` 显示晴/小雨/中雨/大雨/暴雨分级。
8. 点击「暂停采集」调用 `stop_read()` 结束采集；当日 CSV 保留在 `C:\rain_data` 供离线分析。

## 4. 雨滴谱热图读图

- 横轴为信号幅度/电压（2000–2400），纵轴为雨滴速度（0–200），颜色深浅表示该网格内雨滴事件计数；
- 色标位于图下方，由 `#f6efa6`（低计数）渐变到 `#bf444c`（高计数）；
- 网格单元索引为 `(lowest_v - 2000, duration)`，因此横轴每格对应 1 个电压单位、纵轴每格对应 1 个计时单位；
- 谱图与柱状图互为补充：谱图反映雨滴尺度—速度分布，柱状图反映时间上的雨强变化。
