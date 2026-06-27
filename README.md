# Pedal Guru

> ⚠️ This project is under active develpment. The documentation is growing along the project as it's a work-in-progress. If interested, please check the [development board](https://github.com/users/juliannojungle/projects/1) and the [reference diagram](DIAGRAM.md).

An open-source DIY project to bring useful resources that are appreciated by cyclists to easy-to-work platforms like RP2040 (Raspberry Pi Pico) and ESP32-S3, in a more organized manner than usual. The intention is to include integration with some popular services.

Currently supports **RP2040** (Raspberry Pi Pico) and **ESP32** from a single codebase.

---

## 🖥️ Supported Platforms

| Platform | RP2040 | ESP32 |
|---|---|---|
| Pinout |<img alt="RP2040 LCD 1.28" src="Documentation\Image\RP2040_LCD_1_28.png" width="300px">|<img alt="ESP32-S3 LCD 1.28" src="Documentation\Image\ESP32_S3_LCD_1_28.png" width="300px">|
| Device | [RP2040-LCD-1.28](https://www.waveshare.com/wiki/RP2040-LCD-1.28) | [ESP32-S3-LCD-1.28](https://www.waveshare.com/wiki/ESP32-S3-LCD-1.28) |
| Toolchain | arm-none-eabi-gcc + Pico SDK | xtensa-esp32-elf-gcc + ESP-IDF |
| Output | `.uf2` | `.bin` |

---

## 🛒 B.O.M. (Bill Of Materials)

* One of the target devices:
  * [RP2040-LCD-1.28](https://www.waveshare.com/wiki/RP2040-LCD-1.28)
  * [ESP32-S3-LCD-1.28](https://www.waveshare.com/wiki/ESP32-S3-LCD-1.28)
* MicroSD to SD card adapter (for easy soldering)
* MicroSD card
* Some nice wires (30 AWG recommended)
* 2 x 10K resistors (for pull-up)
* 1 x 10uF capacitor (for decoupling)
* *(Optional)* 2 x [1.27mm 2x20Pin header](https://ae01.alicdn.com/kf/H874df6c6359144e497abad3f5e2a03c6b.jpg_220x220.jpg) (for connecting SD to target device)

---

## 🔌 Wiring

> The below table shows connections in a way that you can create a header connector/shield board to swap between the target devices.\
> ⚠️ *Connect it to the RP2040 **or** to the ESP32. You're **NOT** supposed to wire RP2040 and ESP32 together.*

| RP2040     | ESP32       | R1(10K) | R2(10K) | C1(10uF) | SDCARD           | LCD*       |
|------------|-------------|---------|---------|----------|------------------|------------|
| GP0 (MISO) | GP46 (MISO) |         |  MISO   |          | pin7 (DAT0/MISO) |            |
| GP1 (CS)   | GP45 (CS)   |   CS    |         |          | pin1 (DAT3/CS)   |            |
| GP2 (SCK)  | GP42 (SCK)  |         |         |          | pin5 (CLK/SCK)   |            |
| GP3 (MOSI) | GP41 (MOSI) |         |         |          | pin2 (CMD/MOSI)  |            |
| GP5        | GP39        |         |         |          | pin3 (VSS1/GND)  |            |
| GND        | GND         |         |         |   GND    | pin6 (VSS2/GND)  | GND        |
| 3V3        | 3V3         |   3V3   |  3V3    |   3V3    | pin4 (VDD/3V3)   | VCC (3V3)  |
| GP8        | GP8         |         |         |          |                  | DC         |
| GP9        | GP9         |         |         |          |                  | CS         |
| GP10       | GP10        |         |         |          |                  | CLK        |
| GP11       | GP11        |         |         |          |                  | DIN (MOSI) |
| GP12       | GP12        |         |         |          |                  | RST        |
| GP25       | GP40        |         |         |          |                  | BL         |

<sup>* The LCD connection is internally made on these devices, GP pins just for reference.</sup>

---

## 📋 Software Requirements

- **Host**: Linux or Windows 10 PC with [WSL 2 (Windows Subsystem Linux)](https://learn.microsoft.com/pt-br/windows/wsl/install) - Ubuntu is recommended — code edited on Windows, compiled inside WSL
- **IDE**: [Kiro](https://kiro.dev) or [VSCode](https://code.visualstudio.com/)
- **RP2040**: arm-none-eabi-gcc, Pico SDK (installed via `Setup: RP2040 toolchain`)
- **ESP32**: ESP-IDF ≥ 5.x, xtensa toolchain, espflash (installed via `Setup: ESP32 toolchain`)
- **CMake** ≥ 3.16

> The preferred IDE is **[Kiro](https://kiro.dev)**, which provides agent-assisted development with project-aware context via `AGENTS.md`. The project also works fully with **VS Code** — all tasks and settings are configured in `.vscode/`.

---

## 🚀 Getting Started

### 1. Clone with submodules

```bash
git clone --recurse-submodules --shallow-submodules -b feature/RP2040-migration https://github.com/juliannojungle/pedal.guru
```

This single command clones the repository and all bundled dependencies (gui.ll and it's dependencies) in one step. No separate dependency installation needed.

### 2. Set up the toolchain

Before building for the first time, install the required toolchain for your target platform. The recommended way is via the built-in tasks.

**In Kiro or VS Code** — open the Command Palette (`Ctrl+Shift+P`) and select **Tasks: Run Task**, then choose:

- `Setup: RP2040 toolchain in WSL` — installs arm-none-eabi-gcc and Pico SDK
- `Setup: ESP32 toolchain in WSL` — installs ESP-IDF, xtensa toolchain, Rust, and espflash

These scripts are idempotent — safe to run again if you need to repair an installation.

### 3. Build

Once the toolchain is ready, build using the tasks:

**In Kiro or VS Code** — open the Command Palette (`Ctrl+Shift+P`) → **Tasks: Run Task**:

| Task | Description |
|---|---|
| `Build: Full RP2040` | Clean CMake configure + make for RP2040 |
| `Build: Full ESP32` | Clean idf.py build for ESP32 |
| `Build: Incremental` | Auto-detects platform, runs `make` or `ninja` |

Alternatively, trigger the default build task directly with `Ctrl+Shift+B` (runs **Build: Incremental**).

---

## 📄 License

See [LICENSE](LICENSE).

🎨 This software utilizes [gui.ll](https://github.com/juliannojungle/gui.ll) as a platform agnostic backend graphics interface.

---
<sup>[@juliannojungle](https://github.com/juliannojungle), 2022</sup>
