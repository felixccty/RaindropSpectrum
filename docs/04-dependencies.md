# 依赖配置

## 编译/运行依赖

| 类别 | 版本/组件 | 说明 |
|------|-----------|------|
| 语言 | C++11 | `v142` 工具集（VS2019） |
| IDE | Visual Studio 2019 | 桌面 C++ 开发负载 + Qt VS Tools |
| Qt | 5.x | 需 `QtCore` / `QtGui` / `QtWidgets` / `QtCharts` / `QtPrintSupport` |
| QCustomPlot | 1.x（源码） | 提供 `QCPColorMap` / `QCPColorScale`；需将 `qcustomplot.cpp/.h` 放入 `src/` |
| 采集卡 SDK | USB9825 | `USB9825.h` / `USB9825.lib`，链接器附加依赖 `USB9825.lib` |
| 数据 | 雨滴记录 CSV | 由本程序运行时生成于 `C:\rain_data` |

`.vcxproj` 的 `QtModules` 配置为 `charts;core;gui;printsupport;widgets`（Release 配置为 `core;gui;widgets;charts`），其中 `printsupport` 为 QCustomPlot 打印支持所需模块。

## 第三方依赖

- **QCustomPlot**：开源绘图库，用于雨滴谱彩色图（热图）与色标。其源码未随发布包分发，请自行获取并放入 `src/`。
- **USB9825 SDK**：采集卡厂商提供的头文件与导入库，属闭源第三方依赖，需按硬件型号向厂商获取并安装驱动。本仓库仅提供对其 API 的调用代码（`USB9825.h` 中的函数声明）。

本项目不内置任何厂商二进制文件（`.dll` / `.lib` / `.exe`）。

## 系统要求

- Windows 7 / 10 x64；
- 已安装 USB9825 采集卡驱动并可正常枚举设备；
- 需对 `C:\` 根目录具有写权限（数据目录固定为 `C:\rain_data`）；
- 建议 4 GB 以上内存（长时间连续采集时缓冲与图表数据量较大）。
