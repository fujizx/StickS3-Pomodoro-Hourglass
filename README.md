# Stick S3 Toys

M5Stack Stick S3 experiments.

## First toy: shake dice

The starter sketch displays a die face on the built-in screen. Press BtnA or shake the device to roll.

## Build

This project is set up for PlatformIO:

```powershell
pio run
pio run -t upload
pio device monitor
```

The local machine currently does not have `pio` or `arduino-cli` on `PATH`. Install PlatformIO first, then build from this directory.

### Install PlatformIO from Trae CN

Trae CN is VS Code-like, so try the normal extension path first:

1. Open this folder in Trae CN: `D:\sticks3`.
2. Open Extensions.
3. Search for `PlatformIO IDE`.
4. Install the extension named `platformio.platformio-ide`.
5. Restart Trae CN.
6. Open PlatformIO Home and wait for it to install its core tools.

After installation, open a new terminal in Trae CN and check:

```powershell
pio --version
```

If Trae CN cannot find the extension in its marketplace, install PlatformIO Core from a terminal instead:

```powershell
python -m pip install -U platformio
```

## First connection with M5Burner

M5Burner is useful for validating the Windows driver, USB cable, boot mode, and COM port before building custom firmware.

1. Connect the Stick S3 with USB-C and keep it in USB mode.
2. Start `tools/M5Burner-v3-beta-win-x64/M5Burner.exe`.
3. Select the StickS3 device family.
4. Select `COM3` and `115200`.
5. Burn a small official demo first.

After a successful M5Burner flash, this repo can be used for custom firmware development with PlatformIO.

## Hardware notes

`platformio.ini` uses `esp32-s3-devkitc-1` as a conservative ESP32-S3 target and `M5Unified` for display, button, speaker, and IMU access. If your exact board has a dedicated PlatformIO board ID, replace the `board` value with that ID.
