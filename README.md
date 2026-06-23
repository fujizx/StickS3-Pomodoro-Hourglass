# StickS3 Pomodoro Hourglass

一个为 M5Stack StickS3 制作的番茄钟沙漏固件。它把番茄钟倒计时做成蓝色沙粒沙漏：沙粒总量固定，按照所选时长稳定漏下；旋转设备时，沙粒会根据 IMU 重力方向重新堆积，但不会影响倒计时准确性。

## 功能

- 支持 15 / 25 / 50 分钟番茄钟
- 默认 25 分钟
- 蓝色沙粒沙漏动画
- 倒计时结束时沙粒刚好漏完
- 设备旋转时，顶部和底部沙粒按重力方向堆积
- 180 度倒置时进入 Reset / Exit 菜单
- 时间到后蜂鸣器滴滴两声，当前音量约 30%
- 本地记录最近 10 条完成/未完成番茄钟记录
- 离屏 Canvas 绘制，减少 ST7789 频闪

## 设备

- M5Stack StickS3
- SoC: ESP32-S3-PICO-1-N8R8
- Flash: 8MB
- PSRAM: 8MB
- Display: ST7789P3 135x240
- Battery: 250mAh

## 操作

- BtnB：选择下一项
- BtnA：确认
- 番茄钟运行时倒置 180 度：进入 Reset / Exit 菜单

## 构建

需要安装 PlatformIO。

```powershell
pio run
```

烧录：

```powershell
pio run --target upload
```

串口监视：

```powershell
pio device monitor -b 115200
```

## M5Burner 发布说明

M5Burner 发布的是完整可烧录固件，而不是源码中的某个模块。这个仓库面向 M5Burner 的发布形态是一个完整 StickS3 番茄钟沙漏固件。

发布包建议包含：

- firmware.bin
- bootloader.bin
- partitions.bin
- GitHub 源码地址
- 设备型号：M5Stack StickS3
- License：MIT

## 开源协议

MIT License
