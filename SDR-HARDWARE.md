# UHSDR SDR Extension — Hardware Design Document

**Version:** 0.1  
**Date:** 2026-09-10  
**Status:** Design — Hardware Specification  
**Target Frequencies:** 700 MHz, 2.3 GHz  
**Target MCUs:** STM32F7 (OVI40), STM32H7 (OVI40)  
**Architecture Reference:** `ARCH.md` Section 15

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [System Block Diagram](#2-system-block-diagram)
3. [RF Frontend Architecture](#3-rf-frontend-architecture)
4. [Local Oscillator / PLL Selection](#4-local-oscillator--pll-selection)
5. [Mixer Selection](#5-mixer-selection)
6. [Power Amplifier (PA) Selection](#6-power-amplifier-pa-selection)
7. [Low-Noise Amplifier (LNA) Selection](#7-low-noise-amplifier-lna-selection)
8. [Filter Design](#8-filter-design)
9. [ADC / DAC Requirements](#9-adc--dac-requirements)
10. [MCU & Host Board](#10-mcu--host-board)
11. [Interface Mapping](#11-interface-mapping)
12. [Power Budget](#12-power-budget)
13. [Thermal Design](#13-thermal-design)
14. [Mechanical & PCB Guidelines](#14-mechanical--pcb-guidelines)
15. [Component Recommendations](#15-component-recommendations)
16. [Bill of Materials (BOM) Sketch](#16-bill-of-materials-bom-sketch)
17. [Regulatory & Compliance](#17-regulatory--compliance)
18. [Risk Assessment](#18-risk-assessment)
19. [References](#19-references)

---

## 1. Executive Summary

Tài liệu này định nghĩa **phần cứng** cho mở rộng UHSDR thành **SDR một kênh video + voice** tại **700 MHz** và **2.3 GHz** sử dụng **COFDM/mesh**. Đây là phần phụ thuộc trực tiếp vào `ARCH.md` Section 15 và `docs/TODO.md` Phase 80.

**Mục tiêu thiết kế:**
- **Frequency range:** 700 MHz ±50 MHz, 2.3 GHz ±100 MHz
- **Modulation:** QPSK/16QAM single-carrier; COFDM (32–64 subcarriers) later
- **Service:** 1 kênh voice (Codec2 1300/3200 bit/s) + 1 kênh video (MJPEG/H.264)
- **Mesh:** 3–10 nodes, no IP stack
- **MCU target:** STM32F7 minimum; STM32H7 preferred for video + COFDM

**Các khối phần cứng chính:**
1. LO / PLL (tạo tần số local oscillator)
2. Mixer (up/down conversion)
3. PA / LNA (truyền/nhận)
4. Filter (BPF/LPF, chọn băng, loại bỏ spurious)
5. ADC / DAC (chuyển đổi số/analog, ngoại vi)
6. MCU + board (FPGA/SoC optional cho COFDM nặng)

---

## 2. System Block Diagram

```
                    ┌───────────────────────────────────────────┐
                    │           UHSDR HOST BOARD                │
                    │         (mcHF UI / OVI40)                │
                    │                                           │
                    │  ┌─────────────┐    ┌─────────────────┐  │
                    │  │   STM32     │    │   Codec / Audio │  │
                    │  │   F7/H7     │◄──►│   (WM8731/etc)  │  │
                    │  └──────┬──────┘    └─────────────────┘  │
                    │         │                                │
                    │  ┌──────▼──────┐    ┌─────────────────┐  │
                    │  │   Camera    │    │   Video Encoder │  │
                    │  │ (MIPI/USB)  │◄──►│   (H.264/MJPEG) │  │
                    │  └──────┬──────┘    └─────────────────┘  │
                    │         │                                │
                    │  ┌──────▼──────┐    ┌─────────────────┐  │
                    │  │  Modem /    │    │   Mesh MAC /    │  │
                    │  │  COFDM IP   │◄──►│   Routing       │  │
                    │  └──────┬──────┘    └─────────────────┘  │
                    │         │                                │
                    │  ┌──────▼──────────────────────────────┐ │
                    │  │          Baseband I/O               │ │
                    │  │  ┌──────────┐    ┌───────────────┐ │ │
                    │  │  │  DAC     │    │    ADC       │ │ │
                    │  │  │(baseband)│    │ (baseband)   │ │ │
                    │  │  └────┬─────┘    └──────┬───────┘ │ │
                    │  └───────┼────────────────┼──────────┘ │
                    └──────────┼────────────────┼────────────┘
                               │  SPI / I2C /   │
                               │  Parallel Bus  │
                    ┌───────────▼────────────────▼────────────┐
                    │         SDR EXTENSION BOARD             │
                    │                                           │
                    │  ┌─────────────┐    ┌─────────────────┐ │
                    │  │    LO /     │    │    Mixer /      │ │
                    │  │    PLL      │───►│  Up/Down Conv   │ │
                    │  │ (ADF4351/   │    │  (ADL5801/etc)  │ │
                    │  │  LMX2582)   │    └────────┬────────┘ │
                    │  └─────────────┘             │          │
                    │                               │          │
                    │  ┌─────────────┐    ┌────────▼────────┐ │
                    │  │    LNA      │    │  Filter (BPF/   │ │
                    │  │  (RX path)  │◄──►│  LPF/SAW)       │ │
                    │  └─────────────┘    └────────┬────────┘ │
                    │                               │          │
                    │  ┌─────────────┐    ┌────────▼────────┐ │
                    │  │    PA       │    │  Antenna Switch │ │
                    │  │  (TX path)  │───►│  (PIN diode)    │ │
                    │  └─────────────┘    └─────────────────┘ │
                    │                                           │
                    └───────────────────────────────────────────┘
                                    │
                                    ▼
                            ┌───────────────┐
                            │   Antenna     │
                            │  (700/2300)   │
                            └───────────────┘
```

---

## 3. RF Frontend Architecture

### 3.1 Architecture Options

| Architecture | Pros | Cons | Recommendation |
|---|---|---|---|
| **Direct Conversion** (Zero-IF) | Simple, minimal image | DC offset, LO leakage | Not recommended for 2.3 GHz |
| **Low-IF** | Reduced DC issues | Still needs image reject | Possible for 700 MHz |
| **Superheterodyne** | Image reject, flexible LO | More components, higher NF | **Recommended for both bands** |
| **Direct Sampling** | No mixer needed | ADC speed limit, aliasing | Not feasible at 2.3 GHz |

**Selected Architecture:** **Superheterodyne with IF = 100–200 MHz**
- LO1: high-frequency PLL (700 MHz or 2.3 GHz)
- LO2: fixed IF LO (Si570/Si5351 at 100–200 MHz)
- Image reject via SAW filter at IF
- Lower ADC speed requirement (~50–100 MSPS)

### 3.2 Frequency Plan

#### 700 MHz Band
```
RF: 650–750 MHz (100 MHz bandwidth)
LO1: 550–650 MHz (from ADF4351)
IF: 100 MHz (fixed)
ADC: 100–150 MHz sample rate (undersampled)
```

#### 2.3 GHz Band
```
RF: 2250–2350 MHz (100 MHz bandwidth)
LO1: 2100–2200 MHz (from LMX2582)
IF: 150 MHz (fixed)
ADC: 200–300 MHz sample rate (undersampled)
```

---

## 4. Local Oscillator / PLL Selection

### 4.1 PLL Requirements

| Parameter | 700 MHz | 2.3 GHz |
|---|---|---|
| Min frequency | 550 MHz | 2100 MHz |
| Max frequency | 650 MHz | 2200 MHz |
| Frequency step | < 100 Hz | < 100 Hz |
| Phase noise | <-100 dBc/Hz @ 10 kHz | <-90 dBc/Hz @ 10 kHz |
| Output power | +5 dBm | +5 dBm |
| Harmonics | <-20 dBc | <-20 dBc |
| Spurs | <-70 dBc | <-70 dBc |

### 4.2 Recommended PLLs

#### ADF4351 (for 700 MHz)
- **Frequency range:** 35–4400 MHz
- **Interface:** SPI (3-wire)
- **Phase noise:** -114 dBc/Hz @ 100 kHz offset
- **Output power:** -4 to +5 dBm
- **VCO:** internal
- **Reference:** 10–100 MHz (TCXO)
- **Package:** 24-lead LFCSP
- **Pros:** Proven, cheap, easy to program
- **Cons:** Higher phase noise at close-in offsets

#### LMX2582 (for 2.3 GHz)
- **Frequency range:** 20–8000 MHz
- **Interface:** SPI (4-wire)
- **Phase noise:** -147 dBc/Hz @ 100 MHz offset
- **Output power:** +5 to +15 dBm
- **VCO:** internal, 2x VCOs
- **Reference:** 10–200 MHz
- **Package:** 16-lead WQFN
- **Pros:** Excellent phase noise, wide range
- **Cons:** More complex programming

#### Si570 (for IF LO)
- **Frequency range:** 10–160 MHz (CMOS), 10–280 MHz (C-grade)
- **Interface:** I2C
- **Phase noise:** -155 dBc/Hz @ 100 kHz offset
- **Pros:** Already supported in UHSDR
- **Cons:** Limited max frequency for 2.3 GHz IF

### 4.3 PLL Interface to MCU

| Signal | ADF4351 | LMX2582 | MCU Pin |
|---|---|---|---|
| SCK | SCK | SCK | SPIx_SCK |
| SDI | SDI | SDI | SPIx_MOSI |
| SDO | SDO | SDO | SPIx_MISO |
| LE/CE | LE | CE | GPIO |
| MUXOUT | MUXOUT | MUXOUT | GPIO (lock detect) |
| RF_OUT | RF_OUTA | RF_OUT | SMA connector |

### 4.4 Programming Sequence

```c
// Pseudo-code for ADF4351 initialization
void PLL_ADF4351_Init(void) {
    // 1. Set reference frequency
    // 2. Calculate R, N, MOD, FRAC
    // 3. Write registers in order: R4, R3, R2, R1, R0
    // 4. Wait for MUXOUT lock detect
    // 5. Enable output
}

void PLL_ADF4351_SetFrequency(uint32_t freq_hz) {
    // 1. Calculate N = freq / ref
    // 2. Set fractional accumulator
    // 3. Write R0 (fractional)
    // 4. Write R1 (modulus)
    // 5. Write R2 (N divider)
    // 6. Lock detect timeout
}
```

---

## 5. Mixer Selection

### 5.1 Mixer Requirements

| Parameter | 700 MHz | 2.3 GHz |
|---|---|---|
| Frequency range | 50–1000 MHz | 1–3 GHz |
| Conversion loss | < 8 dB | < 10 dB |
| IP3 | > 15 dBm | > 15 dBm |
| NF | < 6 dB | < 8 dB |
| LO drive | +5 dBm | +5 dBm |
| Interface | Single-ended | Differential preferred |

### 5.2 Recommended Mixers

#### 700 MHz: ADL5801 or LTC5510
- **ADL5801:** 1–1000 MHz, 6 dB NF, 12 dBm IP3, LO drive +5 dBm
- **LTC5510:** 0.1–1000 MHz, 6.5 dB NF, 15 dBm IP3, LO drive +5 dBm
- **Package:** MSOP-8 or QFN

#### 2.3 GHz: HMC219 or ADL5375
- **HMC219:** 1.5–2.5 GHz, 7.5 dB NF, 18 dBm IP3, LO drive +5 dBm
- **ADL5375:** 1.5–3.5 GHz, 7 dB NF, 18 dBm IP3, LO drive +5 dBm
- **Package:** SOT-26 or QFN

### 5.3 Mixer Configuration

```
RX path:
  Antenna → LNA → BPF → Mixer (RF in, LO in, IF out) → IF LPF → ADC

TX path:
  DAC → Baseband LPF → Mixer (IF in, LO in, RF out) → PA → BPF → Antenna
```

---

## 6. Power Amplifier (PA) Selection

### 6.1 PA Requirements

| Parameter | 700 MHz | 2.3 GHz |
|---|---|---|
| Frequency range | 650–750 MHz | 2250–2350 MHz |
| Output power | 500 mW–1 W (27–30 dBm) | 100–500 mW (20–27 dBm) |
| Gain | 20–30 dB | 20–30 dB |
| P1dB | > 30 dBm | > 27 dBm |
| PAE | > 30% | > 20% |
| Modulation | QPSK/16QAM (linear) | QPSK/16QAM (linear) |

### 6.2 Recommended PAs

#### 700 MHz: Skyworks SKY65335 or NXP BGA2869
- **SKY65335:** 500 mW, 27 dBm, 32% PAE, 3.3V supply
- **BGA2869:** 1 W, 30 dBm, 35% PAE, 5V supply

#### 2.3 GHz: Skyworks SKY65338 or Qorvo TGA2577
- **SKY65338:** 250 mW, 24 dBm, 28% PAE, 3.3V supply
- **TGA2577:** 500 mW, 27 dBm, 22% PAE, 5V supply

### 6.3 PA Control

| Signal | Function | MCU Pin |
|---|---|---|
| PA_EN | Enable PA | GPIO |
| PA_VGA | Variable gain | DAC / SPI |
| PA_BYPASS | Bypass PA for RX | GPIO |
| PA_TEMP | Temperature monitor | ADC |

---

## 7. Low-Noise Amplifier (LNA) Selection

### 7.1 LNA Requirements

| Parameter | 700 MHz | 2.3 GHz |
|---|---|---|
| Frequency range | 650–750 MHz | 2250–2350 MHz |
| NF | < 2 dB | < 2.5 dB |
| Gain | 15–20 dB | 15–20 dB |
| IP3 | > 10 dBm | > 10 dBm |
| Supply | 3.3V | 3.3V |

### 7.2 Recommended LNAs

#### 700 MHz: Skyworks SKY65334 or Infineon BGA2869
- **SKY65334:** 2.0 dB NF, 18 dB gain, +8 dBm IP3

#### 2.3 GHz: Qorvo TGA2576 or Skyworks SKY65337
- **TGA2576:** 2.2 dB NF, 17 dB gain, +9 dBm IP3

---

## 8. Filter Design

### 8.1 Filter Requirements

| Filter | Type | Frequency | BW | Insertion Loss | Rejection |
|---|---|---|---|---|---|
| 700 MHz BPF | SAW/Ceramic | 650–750 MHz | 100 MHz | < 3 dB | > 40 dB @ ±150 MHz |
| 2.3 GHz BPF | Microstrip/SAW | 2250–2350 MHz | 100 MHz | < 4 dB | > 35 dB @ ±200 MHz |
| IF BPF | SAW | 100–200 MHz | 20–50 MHz | < 3 dB | > 50 dB |
| LPF (anti-alias) | LC/Surface acoustic | < 2x ADC BW | — | < 2 dB | > 30 dB |

### 8.2 Filter Implementation

**700 MHz:**
- **Bandpass:** TDK SAW filter B4042 or Murata ceramic filter
- **Alternative:** Discrete LC filter (high Q inductors required)
- **PCB:** 50Ω microstrip, FR4 or Rogers 4350

**2.3 GHz:**
- **Bandpass:** Mini-Circuits SAW filter or microstrip filter
- **Implementation:** Copper pour on PCB, simulated with ADS/Momentum
- **PCB:** Rogers 4350B, 10 mil thickness

### 8.3 Filter Board Layout

```
Antenna → LNA → BPF → Mixer
                    ↓
                IF BPF → ADC

TX:
DAC → LPF → Mixer → LPF → PA → BPF → Antenna
```

---

## 9. ADC / DAC Requirements

### 9.1 ADC Requirements (RX)

| Parameter | 700 MHz | 2.3 GHz |
|---|---|---|
| Sample rate | 100–150 MSPS | 200–300 MSPS |
| Resolution | 12–14 bit | 12–14 bit |
| ENOB | > 10 bit | > 10 bit |
| Input bandwidth | > 500 MHz | > 2 GHz |
| Input range | 1–2 Vpp | 1–2 Vpp |
| Interface | LVDS / CMOS | LVDS / CMOS |

### 9.2 DAC Requirements (TX)

| Parameter | 700 MHz | 2.3 GHz |
|---|---|---|
| Sample rate | 100–200 MSPS | 200–500 MSPS |
| Resolution | 12–14 bit | 12–14 bit |
| Output bandwidth | > 500 MHz | > 2 GHz |
| Output range | 0.5–1 Vpp | 0.5–1 Vpp |
| Interface | LVDS / CMOS | LVDS / CMOS |

### 9.3 Recommended ADC/DAC

#### ADC
- **AD9680:** 14-bit, 500 MSPS, JESD204B, 2.5 GHz analog bandwidth
- **LTC2185:** 16-bit, 125 MSPS, 325 MHz bandwidth (700 MHz only)
- **ADS42LB69:** 16-bit, 250 MSPS, 650 MHz bandwidth

#### DAC
- **AD9739:** 14-bit, 500 MSPS, 2.5 GHz bandwidth
- **LTC2000:** 16-bit, 250 MSPS, 1.3 GHz bandwidth
- **AD9154:** 16-bit, 500 MSPS, 2.3 GHz bandwidth

### 9.4 Interface to MCU

STM32F7/H7 does **not** have built-in high-speed ADC/DAC interface for 2.3 GHz.

**Options:**
1. **FPGA bridge:** FPGA handles ADC/DAC + COFDM; MCU does mesh/voice/video
2. **External ADC/DAC with parallel interface:** Use FMC or parallel GPIO (slow)
3. **Dedicated SDR SoC:** Move to Zynq or RFSoC (out of scope for UHSDR)

**Recommended for Phase 1–2:**
- Use **lower IF** (e.g., 10–20 MHz) where MCU can sample with internal ADC (limited)
- Or use **external ADC/DAC with SPI/LVDS** + DMA, accept lower sample rate for single-carrier

---

## 10. MCU & Host Board

### 10.1 MCU Selection

| MCU | Voice | Video | COFDM | Mesh | Power | Notes |
|---|---|---|---|---|---|---|
| **STM32F4** | ✅ | ❌ | ❌ | ⚠️ 3 nodes | 100–200 mA | mcHF current; too constrained |
| **STM32F7** | ✅ | ⚠️ MJPEG | ⚠️ Basic | ✅ 3–5 nodes | 200–300 mA | OVI40 F7; minimum viable |
| **STM32H7** | ✅ | ✅ H.264 | ✅ Feasible | ✅ 5–10 nodes | 300–500 mA | OVI40 H7; preferred |

**Selected:** **STM32H743xx** for video + COFDM + mesh. STM32F767xx acceptable for voice + mesh only.

### 10.2 Host Board

**Option A: Extend OVI40 board**
- Add SDR extension as **daughterboard**
- Interface: FMC (H7) or parallel GPIO + SPI
- Power: 3.3V, 1.8V from main board

**Option B: Standalone SDR extension board**
- MCU on extension board
- USB/Ethernet to host for debug
- Independent power supply

**Recommended:** **Option A** (daughterboard) for Phase 1–2.

### 10.3 MCU Pin Assignment (STM32H7)

| Function | MCU Pin | Notes |
|---|---|---|
| LO1 SPI (ADF4351) | SPI1 | 50 MHz max |
| LO2 I2C (Si570) | I2C1 | Already used |
| ADC SPI/LVDS | SPI3 or GPIO | High-speed |
| DAC SPI/LVDS | SPI4 or GPIO | High-speed |
| PA_EN | GPIO | Digital enable |
| PA_VGA | DAC1 | Analog control |
| LNA_EN | GPIO | Digital enable |
| Filter SW1/2 | GPIO | Band select |
| Lock Detect | GPIO | PLL interrupt |
| ADC Temp | ADC1 | PA temperature |

---

## 11. Interface Mapping

### 11.1 SPI Bus Allocation

| Bus | Device | Speed | Notes |
|---|---|---|---|
| SPI1 | ADF4351 (LO1) | 10 MHz | PLL programming |
| SPI2 | Display / SPI flash | 50 MHz | Already used |
| SPI3 | ADC (if SPI mode) | 50 MHz | New |
| SPI4 | DAC (if SPI mode) | 50 MHz | New |
| SPI5 | LMX2582 (LO2) | 10 MHz | PLL programming |

### 11.2 I2C Bus Allocation

| Bus | Device | Speed | Notes |
|---|---|---|---|
| I2C1 | Si570 (LO2) | 400 kHz | Already used |
| I2C2 | Codec | 400 kHz | Already used |
| I2C4 | EEPROM / sensors | 400 kHz | H7 only |

### 11.3 GPIO Allocation

| Signal | Direction | MCU Pin | Notes |
|---|---|---|---|
| PLL1_LE | Output | PB5 | ADF4351 load enable |
| PLL2_CE | Output | PB6 | LMX2582 chip enable |
| LOCK_DETECT | Input | PB7 | PLL lock (interrupt) |
| PA_EN | Output | PB8 | PA enable |
| PA_VGA | Output | DAC1 | PA gain control |
| LNA_EN | Output | PB9 | LNA enable |
| FILTER_SW0 | Output | PB10 | Filter bank 0 |
| FILTER_SW1 | Output | PB11 | Filter bank 1 |
| ANT_SW | Output | PB12 | Antenna switch (RX/TX) |
| PTT | Output | PB13 | External PTT control |

---

## 12. Power Budget

### 12.1 Power Consumption Estimate

| Block | Voltage | Current (700 MHz) | Current (2.3 GHz) | Power | Notes |
|---|---|---|---|---|---|
| STM32H7 | 3.3V | 200 mA | 200 mA | 0.66 W | Core + peripherals |
| ADF4351 | 3.3V | 30 mA | — | 0.10 W | LO only |
| LMX2582 | 3.3V | — | 35 mA | 0.12 W | LO only |
| Mixer | 3.3V | 20 mA | 25 mA | 0.07 W | Active mixer |
| LNA | 3.3V | 30 mA | 35 mA | 0.10 W | RX path |
| PA | 3.3V / 5V | 500 mA | 300 mA | 1.65 W / 1.50 W | TX peak |
| Filter | — | 0 | 0 | 0 | Passive |
| ADC | 1.8V | 150 mA | 200 mA | 0.27 W / 0.36 W | External |
| DAC | 1.8V | 100 mA | 150 mA | 0.18 W / 0.27 W | External |
| **Total (RX)** | | **530 mA** | **540 mA** | **1.74 W / 2.02 W** | |
| **Total (TX)** | | **800 mA** | **700 mA** | **2.64 W / 2.55 W** | |

### 12.2 Power Supply Requirements

| Rail | Voltage | Current | Source |
|---|---|---|---|
| VDD_CORE | 1.0V | 500 mA | DC-DC from 3.3V |
| VDD_IO | 3.3V | 1 A | LDO from 5V |
| VDD_ANA | 3.3V | 500 mA | LDO (low noise) |
| VDD_PLL | 3.3V | 100 mA | LDO |
| VDD_RF | 5V | 1 A | Switching regulator |
| VDD_ADC | 1.8V | 200 mA | LDO |
| VDD_DAC | 1.8V | 150 mA | LDO |

### 12.3 Power Management

- **Low-power idle:** `__WFI()` in main loop
- **PA shutdown:** When not in TX, PA disabled via PA_EN
- **LNA shutdown:** When not in RX, LNA bypassed
- **PLL standby:** ADF4351/LMX2582 standby mode when not tuning

---

## 13. Thermal Design

### 13.1 Thermal Sources

| Component | Power Dissipation | Temperature Rise | Cooling |
|---|---|---|---|
| STM32H7 | 0.66 W | +20°C | Heatsink optional |
| PA (TX) | 1.5–2.0 W | +40–60°C | **Heatsink required** |
| LNA | 0.1 W | +5°C | None |
| ADC/DAC | 0.5 W | +15°C | Small heatsink |
| PLL | 0.1 W | +5°C | None |

### 13.2 Thermal Recommendations

- **PA:** Aluminum heatsink, 20–30°C/W thermal resistance
- **Board:** 2-layer FR4 with ground plane; thermal vias under PA
- **Temperature sensor:** NTC thermistor on PA; ADC monitoring
- **Thermal protection:** Software shutdown if PA temp > 85°C

---

## 14. Mechanical & PCB Guidelines

### 14.1 PCB Stackup

| Layer | 700 MHz Board | 2.3 GHz Board |
|---|---|---|
| L1 | Signal + GND | Signal + GND |
| L2 | GND plane | GND plane |
| L3 | Power | Power |
| L4 | Signal | Signal + GND |
| L5 | GND plane | GND plane |
| L6 | Signal | Signal |

**Substrate:**
- 700 MHz: FR4, 1.6 mm, 1 oz copper
- 2.3 GHz: Rogers 4350B, 10 mil, 0.5 oz copper

### 14.2 Layout Guidelines

1. **Impedance control:** 50Ω microstrip for all RF traces
2. **Grounding:** Solid ground plane; no slots in RF path
3. **Isolation:** Separate analog/digital ground; star ground
4. **Shielding:** Aluminum can over LO/PLL/PA
5. **Connectors:** SMA for RF, Hirose/ Molex for digital

### 14.3 RF Layout

- Keep LO, RF, IF traces short
- Avoid 90-degree bends; use mitered corners
- Stitching vias around RF enclosures
- Filter and PA components close to connector

---

## 15. Component Recommendations

### 15.1 LO / PLL

| Band | Component | Vendor | Package | Notes |
|---|---|---|---|---|
| 700 MHz | ADF4351 | Analog Devices | 24-LFCSP | Proven, easy |
| 2.3 GHz | LMX2582 | Texas Instruments | 16-WQFN | Best phase noise |
| IF | Si570 | Skyworks | 10-MSOP | Already supported |

### 15.2 Mixer

| Band | Component | Vendor | Package | Notes |
|---|---|---|---|---|
| 700 MHz | ADL5801 | Analog Devices | MSOP-8 | Low NF |
| 700 MHz | LTC5510 | Analog Devices | MSOP-8 | High IP3 |
| 2.3 GHz | HMC219 | Analog Devices | SOT-26 | Compact |
| 2.3 GHz | ADL5375 | Analog Devices | QFN-24 | Differential LO |

### 15.3 PA

| Band | Component | Vendor | Package | Notes |
|---|---|---|---|---|
| 700 MHz | SKY65335 | Skyworks | QFN-16 | 500 mW |
| 700 MHz | BGA2869 | NXP | QFN-16 | 1 W |
| 2.3 GHz | SKY65338 | Skyworks | QFN-16 | 250 mW |
| 2.3 GHz | TGA2577 | Qorvo | QFN-16 | 500 mW |

### 15.4 LNA

| Band | Component | Vendor | Package | Notes |
|---|---|---|---|---|
| 700 MHz | SKY65334 | Skyworks | QFN-16 | 2.0 dB NF |
| 2.3 GHz | TGA2576 | Qorvo | QFN-16 | 2.2 dB NF |

### 15.5 Filter

| Band | Component | Vendor | Package | Notes |
|---|---|---|---|---|
| 700 MHz | B4042 | TDK | SAW | 100 MHz BW |
| 2.3 GHz | Custom | Mini-Circuits | Microstrip | Simulated |

### 15.6 ADC / DAC

| Component | Type | Vendor | Speed | Notes |
|---|---|---|---|---|
| AD9680 | ADC | Analog Devices | 500 MSPS | 14-bit, LVDS |
| LTC2185 | ADC | Analog Devices | 125 MSPS | 16-bit, 700 MHz only |
| AD9739 | DAC | Analog Devices | 500 MSPS | 14-bit |
| AD9154 | DAC | Analog Devices | 500 MSPS | 16-bit |

---

## 16. Bill of Materials (BOM) Sketch

### 16.1 700 MHz Extension Board

| Qty | Part | Description | Vendor | Part Number |
|---|---|---|---|---|
| 1 | MCU | STM32H743xx | ST | STM32H743ZIT6 |
| 1 | PLL | 35–4400 MHz | AD | ADF4351 |
| 1 | Mixer | 1–1000 MHz | AD | ADL5801 |
| 1 | LNA | 650–750 MHz | Skyworks | SKY65334 |
| 1 | PA | 500 mW | Skyworks | SKY65335 |
| 2 | BPF | 650–750 MHz | TDK | B4042 |
| 1 | ADC | 14-bit, 250 MSPS | AD | LTC2185 |
| 1 | DAC | 14-bit, 250 MSPS | AD | LTC2000 |
| 1 | TCXO | 10 MHz, 0.5 ppm | NDK | NZ2520SD |
| 1 | VREG | 3.3V, 1A | TI | LD1117-3.3 |
| 1 | VREG | 1.8V, 500 mA | TI | LD1117-1.8 |
| 2 | VREG | 5V, 1A | TI | LM2596 |
| 1 | CONN | SMA female | Amphenol | 132134 |
| 1 | PCB | FR4, 4-layer | OSHPark | — |
| — | Passives | Caps, resistors, inductors | Various | — |

### 16.2 2.3 GHz Extension Board

| Qty | Part | Description | Vendor | Part Number |
|---|---|---|---|---|
| 1 | MCU | STM32H743xx | ST | STM32H743ZIT6 |
| 1 | PLL | 20–8000 MHz | TI | LMX2582 |
| 1 | Mixer | 1.5–2.5 GHz | AD | HMC219 |
| 1 | LNA | 2.25–2.35 GHz | Qorvo | TGA2576 |
| 1 | PA | 250 mW | Skyworks | SKY65338 |
| 1 | BPF | 2.25–2.35 GHz | Mini-Circuits | Custom |
| 1 | ADC | 14-bit, 500 MSPS | AD | AD9680 |
| 1 | DAC | 14-bit, 500 MSPS | AD | AD9739 |
| 1 | TCXO | 10 MHz, 0.5 ppm | NDK | NZ2520SD |
| 1 | VREG | 3.3V, 1A | TI | LD1117-3.3 |
| 1 | VREG | 1.8V, 500 mA | TI | LD1117-1.8 |
| 2 | VREG | 5V, 1A | TI | LM2596 |
| 1 | CONN | SMA female | Amphenol | 132134 |
| 1 | PCB | Rogers 4350B, 4-layer | OSHPark | — |
| — | Passives | Caps, resistors, inductors | Various | — |

---

## 17. Regulatory & Compliance

### 17.1 Frequency Bands

| Band | Frequency | License Required | Max EIRP | Notes |
|---|---|---|---|---|
| 700 MHz | 650–750 MHz | Varies by country | Usually < 1 W | Check local band plan |
| 2.3 GHz | 2250–2350 MHz | Varies by country | Usually < 1 W | ISM in some regions |

### 17.2 Emission Requirements

- **Spurious emissions:** <-50 dBm @ 3 m (FCC Part 15)
- **Occupied bandwidth:** < 20 MHz (for 100 MHz channel)
- **Out-of-band emission:** <-30 dBm outside channel

### 17.3 Safety

- **RF exposure:** < 10 mW/cm² @ 3 m (general public)
- **SAR:** Not applicable for SDR extension (external antenna)
- **Power limiting:** Software-enforced TX power limit

---

## 18. Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| LO phase noise insufficient | Medium | High | Use LMX2582; verify with spectrum analyzer |
| PA thermal runaway | Medium | High | Thermal monitoring + shutdown |
| Filter insertion loss too high | Low | Medium | Use eval boards; measure with VNA |
| ADC/DAC interface speed | Medium | High | Use FPGA bridge or lower IF |
| PCB layout issues at 2.3 GHz | High | High | Simulate with ADS; use Rogers substrate |
| Regulatory non-compliance | Medium | High | Check local regulations before deployment |
| Power supply noise | Medium | Medium | Use LDOs for analog; separate digital supply |
| EMC/EMI | Medium | Medium | Shielding; ground plane; filtering |

---

## 19. References

- `docs/AGENTS.md` — Platform architecture v6.0
- `docs/ARCH.md` — System architecture, Section 15 SDR extension
- `docs/TODO.md` — Phase 80 implementation tasks
- ADF4351 datasheet — Analog Devices
- LMX2582 datasheet — Texas Instruments
- ADL5801 datasheet — Analog Devices
- HMC219 datasheet — Analog Devices
- Skyworks SKY6533x family datasheets
- Qorvo TGA2576/77 datasheets
- CMSIS-DSP library — ARM
- UHSDR codebase — `mchf-eclipse/drivers/ui/oscillator/`

---

*This document is the single source of truth for hardware design decisions. All hardware implementation must align with this specification.*
