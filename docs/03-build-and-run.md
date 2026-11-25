# 运行与部署

## 1. 环境准备

- Windows 7 / 10 x64；
- Visual Studio 2019（Desktop development with C++ 工作负载，`v142` 工具集）；
- Qt 5.x（含 `Qt Core` / `Gui` / `Widgets` / `Charts` / `PrintSupport` 模块）与 Qt VS Tools 插件；
- `QCustomPlot` 源码（`qcustomplot.cpp` / `qcustomplot.h`）；
- `USB9825` 采集卡 SDK：`USB9825.h`、`USB9825.lib` 及对应设备驱动。

## 2. 编译

1. 将 `qcustomplot.cpp` / `qcustomplot.h` 放入 `src/`（`.vcxproj` 已引用这两个文件，但源码未随发布包分发）。
2. 用 Visual Studio 打开 `src/raindrop_spectrum.sln`。
3. 确认项目属性中的 Qt 版本与本机一致；`.vcxproj` 中 `QtInstall` 为 `msvc2015_64` / `msvc2017` 的历史配置，需按本机 Qt 安装调整。
4. 确认链接器附加依赖 `USB9825.lib` 可被找到，且 SDK 的 DLL 在运行时可用。
5. 选择 `Debug | x64`（或 `Win32`）生成解决方案，输出 `raindrop_spectrum.exe`。

> 源码 `.cpp/.h` 为 **GBK** 编码（含中文注释与界面字符串），`.ui/.sln/.vcxproj` 为 **UTF-8**；
> 请勿整体转码，详见 [注意事项](06-notes.md)。

## 3. 硬件连接

1. 将 USB9825 采集卡接入计算机并安装厂商驱动；
2. 将光电探测器信号接入采集卡对应通道（工程配置为单通道、内部时钟、外部上升沿触发）；
3. 启动程序后点击「连接」，若遍历 0–31 号设备均未成功，按钮显示「连接失败」，请检查驱动与设备占用。

## 4. 运行

1. 首次运行会在 `C:\rain_data` 创建数据目录与当日 `Rain_recordYYYY-MM-DD.csv`，需保证当前账户对该路径有写权限。
2. 双击运行 `raindrop_spectrum.exe`，程序启动时自动执行 `connect_to_card()` 与 `read_consecutive()` 初始化图表与定时器。
3. 点击「开始采集」进入连续采集态，`timer1` 驱动雨滴识别、`timer_ra` 每分钟汇总雨量；点击「暂停采集」调用 `stop_read()` 停止。
4. 在谱图上拖拽/滚轮缩放查看分布，鼠标移动时 `label` / `label_2` 实时显示幅度与速度读数。

## 5. 打包发布（Windows）

使用 Qt 自带部署工具收集运行库：

```bat
windeployqt raindrop_spectrum.exe --release
```

将 `USB9825.lib` 对应的运行时 DLL、采集卡驱动、示例 CSV 一并分发。第三方采集卡 SDK 由用户自行获取，本项目不含其二进制文件。
