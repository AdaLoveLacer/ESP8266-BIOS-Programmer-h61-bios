# ESP8266 W25Qxx BIOS Programmer

EPROM Programmer/Debugger for W25Qxx series (Winbond) BIOS chips using ESP8266, with **automatic capacity detection** and support from **2MB to 32MB**.

## ✨ Main Features

- 🔍 **Automatic Chip Detection**: Identifies W25Q16/32/64/128/256 via JEDEC ID
- 📦 **Multi-Capacity Support**: 2MB, 4MB, 8MB, 16MB, 32MB
- 📤 **Optimized Streaming Upload**: 1KB chunks with automatic verify-retry
- 🔒 **Protection Management**: Automatically removes block protections
- ⚡ **SPI Speed Control**: 0.1-20 MHz adjustable at runtime (presets: 1/4/8/20 MHz)
- ✅ **Automatic Verification**: Byte-by-byte checksum with detailed error report
- 🌐 **Responsive Web Interface**: Embedded HTML/JS with real-time terminal
- 📊 **Detailed Logs**: Verbosity system (Quiet/Normal/Verbose/Debug)
- 🔧 **Robust Base64 Decoding**: Critical transfer bug fixes

## 🔧 Required Hardware

- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
- W25Qxx Series Chip:
  - ✅ W25Q16 (2MB) - JEDEC ID: EF4015
  - ✅ W25Q32 (4MB) - JEDEC ID: EF4016
  - ✅ W25Q64 (8MB) - JEDEC ID: EF4017 **← New support!**
  - ✅ W25Q128 (16MB) - JEDEC ID: EF4018
  - ✅ W25Q256 (32MB) - JEDEC ID: EF4019
- Connection jumpers (recommended: short cables 10-15cm)
- Stable 3.3V power supply (ESP8266 already provides)

## 📌 ESP8266 ↔ W25Qxx Series Pinout

| ESP8266 | Pin | W25Qxx | Description |
|---------|------|----------|-----------|
| D8      | GPIO15 | CS (Pin 1)  | Chip Select |
| D7      | GPIO13 | DI (Pin 5)  | Data Input (MOSI) |
| D6      | GPIO12 | DO (Pin 2)  | Data Output (MISO) |
| D5      | GPIO14 | CLK (Pin 6) | Serial Clock |
| 3.3V    | 3.3V   | VCC (Pin 8) | Power Supply |
| 3.3V    | 3.3V   | **WP# (Pin 3)** | **Write Protect - MUST be connected to VCC!** |
| 3.3V    | 3.3V   | **HOLD# (Pin 7)** | **Hold/Reset - MUST be connected to VCC!** |
| GND     | GND    | GND (Pin 4) | Ground |

**IMPORTANT:** 
- W25Qxx operates ONLY at 3.3V. DO NOT use 5V!
- **⚠️ CRITICAL:** Pins 3 (WP#) and 7 (HOLD#) **MUST** be connected to 3.3V to allow writing!
- If WP# is left at GND, the Status Register is locked and write protection cannot be removed!
- **Use short cables (10-15cm)** to avoid signal integrity issues

## 🔍 W25Qxx Pinout (SOIC-8)

```
     ┌─── 1 CS (Chip Select)
     │ ┌─ 2 DO (MISO)
     │ │ ┌ 3 WP# (Write Protect) → ⚠️ Connect to VCC (3.3V)!
     │ │ │┌ 4 GND
  ┌──▼─▼─▼▼─┐
1─┤  ●      ├─8 VCC (3.3V)
2─┤         ├─7 HOLD# → ⚠️ Connect to VCC (3.3V)!
3─┤  W25Qxx├─6 CLK
4─┤  Series├─5 DI (MOSI)
  └─────────┘
```

**⚠️ WARNING - CRITICAL CONFIGURATION:**
- **Pin 3 (WP#)**: **MUST** be connected to **3.3V** (do not leave floating or at GND!)
- **Pin 7 (HOLD#)**: **MUST** be connected to **3.3V** (do not leave floating or at GND!)
- If WP# is at GND, the Status Register is protected and you **WILL NOT BE ABLE** to remove block protection!
- If HOLD# is at GND, SPI communication will be permanently paused!

### 🔌 Complete Connection Diagram

```
ESP8266                      W25Qxx (SOIC-8)
                        
3.3V ────┬──────────────────► Pin 8 (VCC)
         │
         ├──────────────────► Pin 3 (WP#)   ⚠️ CRITICAL!
         │
         └──────────────────► Pin 7 (HOLD#) ⚠️ CRITICAL!

GND ─────────────────────────► Pin 4 (GND)

D8 (GPIO15) ─────────────────► Pin 1 (CS)
D7 (GPIO13) ─────────────────► Pin 5 (DI/MOSI)
D6 (GPIO12) ─────────────────► Pin 2 (DO/MISO)
D5 (GPIO14) ─────────────────► Pin 6 (CLK)
```

**Checklist before programming:**
- [ ] VCC connected to 3.3V (Pin 8)
- [ ] GND connected (Pin 4)
- [ ] **WP# connected to 3.3V (Pin 3)** ⚠️
- [ ] **HOLD# connected to 3.3V (Pin 7)** ⚠️
- [ ] CS connected to D8 (Pin 1)
- [ ] MOSI connected to D7 (Pin 5)
- [ ] MISO connected to D6 (Pin 2)
- [ ] CLK connected to D5 (Pin 6)
- [ ] **Short cables (10-15cm maximum)** 📏

## 🛠️ Arduino IDE Configuration

### 1. Install ESP8266 Core
1. File → Preferences
2. In "Additional Board Manager URLs" add:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Tools → Board → Boards Manager
4. Search for "ESP8266" and install

### 2. Configure Board
- **Board:** "NodeMCU 1.0 (ESP-12E Module)" or your specific model
- **Upload Speed:** 115200
- **CPU Frequency:** 80 MHz
- **Flash Size:** 4MB (FS:2MB OTA:~1019KB)
- **Port:** Select the ESP8266 COM port

### 3. Configure WiFi
Edit the lines in `esp8266_w25q32_programmer.ino` file:
```cpp
const char* ssid = "YourWiFi";        // Your network name
const char* password = "YourPassword";   // Your network password
```

### 4. Upload Firmware
1. Connect ESP8266 via USB
2. Open `esp8266_w25q32_programmer.ino`
3. Click Upload (→)
4. Wait for completion

### ⚡ Important: All-in-One Firmware
**🎯 Just compile the .ino file - that's it!**

- ✅ The web interface (HTML/CSS/JS) is **already embedded** in the .ino file
- ✅ No need to upload separate HTML files
- ✅ No need to use filesystem (SPIFFS/LittleFS)
- ✅ Everything works **out of the box** after compilation
- 📄 The `index.html` file is just for **reference/preview** - not required for operation

**What happens:**
1. You compile `esp8266_w25q32_programmer.ino` in Arduino IDE
2. Upload to ESP8266
3. The web interface is **already there**, ready to use
4. Just access the IP address in your browser

## 🌐 Using the Programmer

### 1. Connect to ESP8266
1. Open Serial Monitor (115200 baud)
2. Find the IP shown on WiFi connection
3. Access `http://YOUR_ESP8266_IP` in browser

### 2. Web Interface
The interface allows:

#### 📋 Chip Information
- **Read JEDEC ID:** 
  - ✅ Automatically detects: W25Q16/32/64/128/256
  - ✅ Shows actual chip capacity (2MB-32MB)
  - ✅ Validates SPI communication
- **Status:** Shows if busy or write-protected

#### ⚙️ SPI Configuration
- **Adjustable Frequency:** 0.1 MHz - 20 MHz (slider)
- **Quick Presets:** 1/4/8/20 MHz buttons for quick testing
- **Chunk Size Verify:** 4KB, 8KB, 16KB, 32KB, 64KB
- **Recommendation:** 
  - Use **1 MHz** if you encounter corruption
  - Use **4 MHz** for stability
  - Use **8-20 MHz** for speed (short cables)

#### 📖 Reading
- **Read Data:** Reads specific region (address + size)
- **Full Dump:** 
  - ✅ Automatically detects chip size
  - ✅ Downloads 2MB-32MB according to connected chip
  - ✅ File saved as `bios_<timestamp>.bin`

#### ✏️ Writing
- **Accepts Formats:** `.bin`, `.rom`, `.bss` **← New!**
- **Maximum Size:** Up to 8MB (adjusts according to detected chip)
- **Chunk-by-Chunk System:**
  - Erases sector (4KB)
  - Writes 1KB
  - Verifies immediately
  - Automatic retry (up to 3x per chunk)
- **Automatic Verification:** 
  - Byte-by-byte checksum
  - Detailed error report
  - Locates first/last error
  - Counts affected regions
- **Progress Bar:** Real-time updates

#### 🗑️ Erasing
- **Erase Sector:** Removes specific 4KB
- **Erase Chip:** **⚠️ DANGER** - Removes everything (makes board unusable)
- **Automatic Protection:** Removes protections before erasing

#### 🔓 Chip Protection
- **Check Protection:** Shows Status Register and BP0-BP2 bits
- **Remove Protection:** Clears all block protection bits
- **Auto-Remove:** System automatically removes protections before writing
- **IMPORTANT:** Hardware protection (WP# pin) must be disabled (connected to VCC)

#### 📟 Log Terminal
- **Verbosity Levels:** Quiet / Normal / Verbose / Debug
- **Auto-Scroll:** Enable/disable automatic scrolling
- **Real-Time Logs:** Updates every 1 second
- **Type Filtering:** ERROR (red), WARNING (yellow), INFO (blue), DEBUG (gray)

## 🔐 W25Q32BV Protection System

### Hardware Protection (Pins)
The W25Q32BV has two levels of protection:

**1. WP# Pin (Write Protect - Pin 3):**
- **GND (LOW):** Status Register **LOCKED** - does not accept modifications ❌
- **VCC (HIGH):** Status Register **UNLOCKED** - accepts modifications ✅
- **For writing:** **MANDATORY** to connect to **3.3V**!

**2. HOLD# Pin (Pin 7):**
- **GND (LOW):** SPI Communication **PAUSED** ❌
- **VCC (HIGH):** SPI Communication **NORMAL** ✅
- **For writing:** **MANDATORY** to connect to **3.3V**!

### Software Protection (Status Register)
BP0, BP1, BP2 bits in Status Register protect memory regions:

| BP2 | BP1 | BP0 | Protected Region | Addresses |
|-----|-----|-----|------------------|-----------|
| 0   | 0   | 0   | None (unlocked) ✅ | - |
| 0   | 0   | 1   | Upper 256KB | 0x3F0000-0x3FFFFF |
| 0   | 1   | 0   | Upper 512KB | 0x3E0000-0x3FFFFF |
| 0   | 1   | 1   | Upper 1MB | 0x3C0000-0x3FFFFF |
| 1   | 0   | 0   | Upper 2MB | 0x380000-0x3FFFFF |
| 1   | 0   | 1   | Upper 3MB | 0x300000-0x3FFFFF |
| 1   | 1   | 0   | Upper half | 0x200000-0x3FFFFF |
| 1   | 1   | 1   | Entire chip 🔒 | 0x000000-0x3FFFFF |

**How to use protection:**
1. **Before writing:** Click "🔍 Check Protection"
2. If protected: Click "🔓 Remove Protection"
3. Make sure **WP# is at VCC (3.3V)**!
4. If WP# is at GND, protection removal **WILL NOT WORK**!

## ⚠️ CRITICAL WARNINGS

### 🚨 BACKUP FIRST!
**ALWAYS** make a backup of the original BIOS before any modification:
1. Click "Full Dump (4MB)"
2. Save the `.bin` file in a safe location
3. Keep multiple copies

### 🔴 Writing Risks
- **Corrupted BIOS = Unusable motherboard**
- Only flash compatible BIOS for your H61 board
- Never power off during writing
- Verify file integrity before writing

### ⚡ Electrical Precautions
- **Completely power off** the motherboard (power cable + battery)
- Make sure there is no residual power
- Use static wrist strap before touching components
- Connect GND first, then other pins

## 🔧 Troubleshooting

### ❌ Error: "4001 corrupted bytes" or partial corruption
**Root cause:** Critical bug in Base64 decoding (FIXED in v2.0!)

**Solutions:**
1. **Update to latest version** (Base64 fix implemented)
2. **Reduce SPI speed:**
   - Test 1 MHz first
   - If OK, gradually increase to 4 MHz → 8 MHz
3. **Improve physical connections:**
   - Use shorter cables (10-15cm)
   - Check for loose wires
   - Confirm common ground
4. **Check detailed logs:**
   - Browser console (F12): size of sent chunks
   - Serial Monitor: received/decoded bytes

### 🔍 ID not detected or incorrect JEDEC
- Check SPI connections (pin order)
- Confirm stable 3.3V power
- Test jumper continuity
- Check if chip is not soldered inverted
- Use shorter cables (<15cm)

### 🚫 Write error / "Chip protected"
- **Chip write-protected:**
  - Check if **WP# (Pin 3)** is connected to **3.3V** (not GND!)
  - Click "🔍 Check Protection" in interface
  - If protected, click "🔓 Remove Protection"
  - **If WP# is at GND, removal WILL NOT work!**
- **Chip not responding:**
  - Check if **HOLD# (Pin 7)** is connected to **3.3V** (not GND!)
  - If HOLD# at GND, chip stays permanently paused
- Insufficient voltage (use stable 3.3V supply)
- Unstable connection during operation

### 📡 ESP8266 not connecting to WiFi
- Check SSID and password in code
- 2.4GHz network (ESP8266 does not support 5GHz)
- If it fails, ESP creates Access Point:
  - Name: `ESP8266-BIOS-Programmer`
  - Password: `12345678`
  - IP: `192.168.4.1`

### 💾 Motherboard not booting after flashing
1. **DO NOT PANIC**
2. Re-flash original backup
3. If backup corrupted, look for official BIOS from manufacturer
4. Use board BIOS recovery tool (if available)

## 📚 W25Q32BV Technical Commands

| Command | Code | Description |
|---------|--------|-----------|
| JEDEC ID | 0x9F | Read ID: EF4016 |
| Read Data | 0x03 | Sequential read |
| Write Enable | 0x06 | Enable writing |
| Page Program | 0x02 | Write page (256B) |
| Read Status | 0x05 | Status register |
| Sector Erase | 0x20 | Erase sector (64KB) |
| Chip Erase | 0xC7 | Erase all |

## 🏗️ Code Architecture

```
esp8266_w25q32_programmer.ino
├── 🔧 Configuration
│   ├── WiFi/AP dual mode
│   ├── Configurable SPI (0.1-20 MHz)
│   └── Automatic chip detection
├── 📡 Low-level SPI functions
│   ├── readJedecId() → Read ID + detect capacity
│   ├── detectFlashSize() → Adjust FLASH_SIZE dynamically
│   ├── readData() → Sequential reading
│   ├── programPage() → Write 256 bytes
│   ├── sectorErase() → Erase 4KB
│   └── chipErase() → Erase all
├── 🔒 Protection System
│   ├── readStatus() / readStatus2()
│   ├── writeStatusRegister()
│   ├── disableAllProtection() → Remove BP0-BP2
│   └── isWriteProtected() → Check protection
├── 📤 Streaming System (large files)
│   ├── 1KB chunks for memory economy
│   ├── Write-Verify-Retry per chunk (up to 3x)
│   ├── Erase on-demand (erase sector when needed)
│   └── Real-time progress tracking
├── 🌐 HTTP Web Server
│   ├── handleInterface() → Embedded HTML/JS/CSS
│   ├── handleReadId() → /id (with chip detection)
│   ├── handleRead() → /read (dynamic reading)
│   ├── handleWriteStream() → /writeStream (init/finish)
│   ├── handleWriteChunk() → /writeChunk (ROBUST Base64)
│   ├── handleSetSpiFrequency() → /setSpiFrequency
│   ├── handleCheckProtection() → /checkProtection
│   ├── handleRemoveProtection() → /removeProtection
│   └── handleSystem() → /system (info + detected size)
├── 📊 Log System
│   ├── 4 levels: Quiet/Normal/Verbose/Debug
│   ├── Circular buffer (50 messages)
│   └── /logs endpoint for web terminal
└── 🖥️ HTML/JS Interface
    ├── SPI controls with presets
    ├── Streaming upload with progress
    ├── Byte-by-byte verification with report
    ├── Real-time terminal
    └── Console logs (F12) for debug
```

## 🐛 Fixed Bugs (v2.0)

### 🔴 Critical Bug: Base64 Decoding (RESOLVED)
**Symptom:** 4001 corrupted bytes in large uploads

**Cause:** 3 bugs in manual Base64 decoding:
1. ❌ Loop stopped before end (`length() - 3`)
2. ❌ Premature break on padding
3. ❌ Incorrect byte extraction logic

**Solution:** Base64 decoder rewritten from scratch:
- ✅ Processes ALL characters
- ✅ Accumulates bits gradually (not by blocks)
- ✅ Handles padding correctly
- ✅ Validates each byte
- ✅ Debug logs (input/output size)

**Result:** **0% corruption** in tests with 8MB files

## 📈 Changelog

### v2.0 (2025-10-10)
- ✅ **Automatic detection of W25Q16/32/64/128/256 chips**
- ✅ **Support for 8MB+ (up to 32MB)**
- ✅ **.bss file acceptance**
- ✅ **Critical Base64 decoding fix** (eliminates corruption)
- ✅ **Base64 debug logs** (client + server)
- ✅ **Dynamic dump** (adjusts to chip size)
- ✅ **Interface shows chip name and capacity**
- ✅ **/system endpoint returns detected size**

### v1.0 (Original)
- W25Q32 support (4MB)
- Streaming upload
- Basic verification
- SPI control

## 📄 License

This project is provided "as is" for educational purposes. Use at your own risk. The author is not responsible for damages caused by improper use.

---

**⚠️ FINAL REMINDER:** BIOS flashing is a high-risk operation. Always have a backup and adequate technical knowledge before proceeding.