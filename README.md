# ESP8266 W25Qxx BIOS Programmer# ESP8266 W25Qxx BIOS Programmer



``````

╔═══════════════════════════════════════════════════════════════╗╔═══════════════════════════════════════════════════════════════╗

║                                                               ║║                                                               ║

║                   ⚠️  WORK IN PROGRESS  ⚠️                    ║║                   ⚠️  WORK IN PROGRESS  ⚠️                    ║

║                                                               ║║                                                               ║

║   This project is under active development and testing.      ║║   This project is under active development and testing.      ║

║   Features may change. Use at your own risk.                 ║║   Features may change. Use at your own risk.                 ║

║                                                               ║║                                                               ║

╚═══════════════════════════════════════════════════════════════╝╚═══════════════════════════════════════════════════════════════╝

``````



EPROM Programmer/Debugger for W25Qxx Series BIOS chips (Winbond) using ESP8266, with **automatic capacity detection** and support from **2MB to 32MB**.EPROM Programmer/Debugger for W25Qxx Series BIOS chips (Winbond) using ESP8266, with **automatic capacity detection** and support from **2MB to 32MB**.



## ✨ Key Features## ✨ Key Features



- 🔍 **Automatic Chip Detection**: Identifies W25Q16/32/64/128/256 via JEDEC ID- 🔍 **Detecção Automática de Chip**: Identifica W25Q16/32/64/128/256 via JEDEC ID

- 📦 **Multi-Capacity Support**: 2MB, 4MB, 8MB, 16MB, 32MB- 📦 **Suporte Multi-Capacidade**: 2MB, 4MB, 8MB, 16MB, 32MB

- 📤 **Optimized Streaming Upload**: 1KB chunks with automatic verify-retry- 📤 **Upload Streaming Otimizado**: Chunks de 1KB com verify-retry automático

- 🔒 **Protection Management**: Automatically removes block protection- 🔒 **Gerenciamento de Proteção**: Remove proteções de bloco automaticamente

- ⚡ **SPI Speed Control**: 0.1-20 MHz adjustable at runtime (presets: 1/4/8/20 MHz)- ⚡ **Controle de Velocidade SPI**: 0.1-20 MHz ajustável em runtime (presets: 1/4/8/20 MHz)

- ✅ **Automatic Verification**: Byte-by-byte checksum with detailed error report- ✅ **Verificação Automática**: Checksum byte-a-byte com relatório detalhado de erros

- 🌐 **Responsive Web Interface**: Embedded HTML/JS with real-time terminal- 🌐 **Interface Web Responsiva**: HTML/JS embutido com terminal em tempo real

- 📊 **Detailed Logging**: Verbosity system (Quiet/Normal/Verbose/Debug)- 📊 **Logs Detalhados**: Sistema de verbosidade (Quiet/Normal/Verbose/Debug)

- 🔧 **Robust Base64 Decoding**: Critical transfer bug fixes- 🔧 **Decodificação Base64 Robusta**: Correção de bugs críticos de transferência



## 🔧 Required Hardware## 🔧 Hardware Necessário



- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)

- W25Qxx Series Chip:- Chip W25Qxx Series:

  - ✅ W25Q16 (2MB) - JEDEC ID: EF4015  - ✅ W25Q16 (2MB) - JEDEC ID: EF4015

  - ✅ W25Q32 (4MB) - JEDEC ID: EF4016  - ✅ W25Q32 (4MB) - JEDEC ID: EF4016

  - ✅ W25Q64 (8MB) - JEDEC ID: EF4017 **← New support!**  - ✅ W25Q64 (8MB) - JEDEC ID: EF4017 **← Novo suporte!**

  - ✅ W25Q128 (16MB) - JEDEC ID: EF4018  - ✅ W25Q128 (16MB) - JEDEC ID: EF4018

  - ✅ W25Q256 (32MB) - JEDEC ID: EF4019  - ✅ W25Q256 (32MB) - JEDEC ID: EF4019

- Jumper wires for connections (recommended: short cables 10-15cm)- Jumpers para conexão (recomendado: cabos curtos 10-15cm)

- Stable 3.3V power supply (ESP8266 already provides)- Fonte 3.3V estável (ESP8266 já fornece)



## 📌 ESP8266 ↔ W25Qxx Series Pinout## 📌 Pinagem ESP8266 ↔ W25Qxx Series



| ESP8266 | Pin | W25Qxx | Description || ESP8266 | Pino | W25Qxx | Descrição |

|---------|------|----------|-----------||---------|------|----------|-----------|

| D8      | GPIO15 | CS (Pin 1)  | Chip Select || D8      | GPIO15 | CS (Pin 1)  | Chip Select |

| D7      | GPIO13 | DI (Pin 5)  | Data Input (MOSI) || D7      | GPIO13 | DI (Pin 5)  | Data Input (MOSI) |

| D6      | GPIO12 | DO (Pin 2)  | Data Output (MISO) || D6      | GPIO12 | DO (Pin 2)  | Data Output (MISO) |

| D5      | GPIO14 | CLK (Pin 6) | Serial Clock || D5      | GPIO14 | CLK (Pin 6) | Serial Clock |

| 3.3V    | 3.3V   | VCC (Pin 8) | Power Supply || 3.3V    | 3.3V   | VCC (Pin 8) | Alimentação |

| 3.3V    | 3.3V   | **WP# (Pin 3)** | **Write Protect - MUST be connected to VCC!** || 3.3V    | 3.3V   | **WP# (Pin 3)** | **Write Protect - DEVE estar em VCC!** |

| 3.3V    | 3.3V   | **HOLD# (Pin 7)** | **Hold/Reset - MUST be connected to VCC!** || 3.3V    | 3.3V   | **HOLD# (Pin 7)** | **Hold/Reset - DEVE estar em VCC!** |

| GND     | GND    | GND (Pin 4) | Ground || GND     | GND    | GND (Pin 4) | Terra |



**IMPORTANT:** **IMPORTANTE:** 

- W25Qxx operates ONLY at 3.3V. DO NOT use 5V!- W25Qxx opera APENAS em 3.3V. NÃO use 5V!

- **⚠️ CRITICAL:** Pins 3 (WP#) and 7 (HOLD#) **MUST** be connected to 3.3V to allow writing!- **⚠️ CRÍTICO:** Pinos 3 (WP#) e 7 (HOLD#) **DEVEM** estar conectados a 3.3V para permitir gravação!

- If WP# is left at GND, the Status Register is locked and write protection cannot be removed!- Se deixar WP# em GND, o Status Register fica bloqueado e não é possível remover proteção de escrita!

- **Use short cables (10-15cm)** to avoid signal integrity issues- **Use cabos curtos (10-15cm)** para evitar problemas de integridade de sinal



## 🔍 W25Qxx Pinout (SOIC-8)## 🔍 Pinout W25Qxx (SOIC-8)



``````

     ┌─── 1 CS (Chip Select)     ┌─── 1 CS (Chip Select)

     │ ┌─ 2 DO (MISO)     │ ┌─ 2 DO (MISO)

     │ │ ┌ 3 WP# (Write Protect) → ⚠️ Connect to VCC (3.3V)!     │ │ ┌ 3 WP# (Write Protect) → ⚠️ Conectar a VCC (3.3V)!

     │ │ │┌ 4 GND     │ │ │┌ 4 GND

  ┌──▼─▼─▼▼─┐  ┌──▼─▼─▼▼─┐

1─┤  ●      ├─8 VCC (3.3V)1─┤  ●      ├─8 VCC (3.3V)

2─┤         ├─7 HOLD# → ⚠️ Connect to VCC (3.3V)!2─┤         ├─7 HOLD# → ⚠️ Conectar a VCC (3.3V)!

3─┤  W25Qxx├─6 CLK3─┤  W25Qxx├─6 CLK

4─┤  Series├─5 DI (MOSI)4─┤  Series├─5 DI (MOSI)

  └─────────┘  └─────────┘

``````



**⚠️ CRITICAL CONFIGURATION:****⚠️ ATENÇÃO - CONFIGURAÇÃO CRÍTICA:**

- **Pin 3 (WP#)**: **MUST** be connected to **3.3V** (do not leave floating or at GND!)- **Pino 3 (WP#)**: **DEVE** estar conectado a **3.3V** (não deixe flutuante ou em GND!)

- **Pin 7 (HOLD#)**: **MUST** be connected to **3.3V** (do not leave floating or at GND!)- **Pino 7 (HOLD#)**: **DEVE** estar conectado a **3.3V** (não deixe flutuante ou em GND!)

- If WP# is at GND, the Status Register is protected and you **CANNOT** remove block protection!- Se WP# estiver em GND, o Status Register fica protegido e você **NÃO CONSEGUIRÁ** remover proteção de blocos!

- If HOLD# is at GND, SPI communication will be permanently paused!- Se HOLD# estiver em GND, a comunicação SPI ficará pausada permanentemente!



### 🔌 Complete Connection Diagram### 🔌 Diagrama de Conexão Completo



``````

ESP8266                      W25Qxx (SOIC-8)ESP8266                      W25Qxx (SOIC-8)

                                                

3.3V ────┬──────────────────► Pin 8 (VCC)3.3V ────┬──────────────────► Pin 8 (VCC)

         │         │

         ├──────────────────► Pin 3 (WP#)   ⚠️ CRITICAL!         ├──────────────────► Pin 3 (WP#)   ⚠️ CRÍTICO!

         │         │

         └──────────────────► Pin 7 (HOLD#) ⚠️ CRITICAL!         └──────────────────► Pin 7 (HOLD#) ⚠️ CRÍTICO!



GND ─────────────────────────► Pin 4 (GND)GND ─────────────────────────► Pin 4 (GND)



D8 (GPIO15) ─────────────────► Pin 1 (CS)D8 (GPIO15) ─────────────────► Pin 1 (CS)

D7 (GPIO13) ─────────────────► Pin 5 (DI/MOSI)D7 (GPIO13) ─────────────────► Pin 5 (DI/MOSI)

D6 (GPIO12) ─────────────────► Pin 2 (DO/MISO)D6 (GPIO12) ─────────────────► Pin 2 (DO/MISO)

D5 (GPIO14) ─────────────────► Pin 6 (CLK)D5 (GPIO14) ─────────────────► Pin 6 (CLK)

``````



**Pre-Programming Checklist:****Lista de Verificação antes de gravar:**

- [ ] VCC connected to 3.3V (Pin 8)- [ ] VCC conectado a 3.3V (Pin 8)

- [ ] GND connected (Pin 4)- [ ] GND conectado (Pin 4)

- [ ] **WP# connected to 3.3V (Pin 3)** ⚠️- [ ] **WP# conectado a 3.3V (Pin 3)** ⚠️

- [ ] **HOLD# connected to 3.3V (Pin 7)** ⚠️- [ ] **HOLD# conectado a 3.3V (Pin 7)** ⚠️

- [ ] CS connected to D8 (Pin 1)- [ ] CS conectado a D8 (Pin 1)

- [ ] MOSI connected to D7 (Pin 5)- [ ] MOSI conectado a D7 (Pin 5)

- [ ] MISO connected to D6 (Pin 2)- [ ] MISO conectado a D6 (Pin 2)

- [ ] CLK connected to D5 (Pin 6)- [ ] CLK conectado a D5 (Pin 6)

- [ ] **Short cables (10-15cm maximum)** 📏- [ ] **Cabos curtos (10-15cm máximo)** 📏



## 🛠️ Arduino IDE Setup## 🛠️ Configuração Arduino IDE



### 1. Install ESP8266 Core### 1. Instalar ESP8266 Core

1. File → Preferences1. File → Preferences

2. Add to "Additional Board Manager URLs":2. Em "Additional Board Manager URLs" adicione:

   ```   ```

   http://arduino.esp8266.com/stable/package_esp8266com_index.json   http://arduino.esp8266.com/stable/package_esp8266com_index.json

   ```   ```

3. Tools → Board → Boards Manager3. Tools → Board → Boards Manager

4. Search for "ESP8266" and install4. Procure "ESP8266" e instale



### 2. Configure Board### 2. Configurar Board

- **Board:** "NodeMCU 1.0 (ESP-12E Module)" or your specific model- **Board:** "NodeMCU 1.0 (ESP-12E Module)" ou seu modelo específico

- **Upload Speed:** 115200- **Upload Speed:** 115200

- **CPU Frequency:** 80 MHz- **CPU Frequency:** 80 MHz

- **Flash Size:** 4MB (FS:2MB OTA:~1019KB)- **Flash Size:** 4MB (FS:2MB OTA:~1019KB)

- **Port:** Select your ESP8266 COM port- **Port:** Selecione a porta COM do ESP8266



### 3. Configure WiFi### 3. Configurar WiFi

Edit these lines in `esp8266_w25q32_programmer.ino`:Edite as linhas no arquivo `esp8266_w25q32_programmer.ino`:

```cpp```cpp

const char* ssid = "YourWiFi";        // Your network nameconst char* ssid = "SeuWiFi";        // Nome da sua rede

const char* password = "YourPassword"; // Your network passwordconst char* password = "SuaSenha";   // Senha da sua rede

``````



### 4. Upload Firmware### 4. Upload do Firmware

1. Connect ESP8266 via USB1. Conecte o ESP8266 via USB

2. Open `esp8266_w25q32_programmer.ino`2. Abra `esp8266_w25q32_programmer.ino`

3. Click Upload (→)3. Clique em Upload (→)

4. Wait for completion4. Aguarde conclusão



## 🌐 Using the Programmer## 🌐 Utilizando o Gravador



### 1. Connect to ESP8266### 1. Conectar ao ESP8266

1. Open Serial Monitor (115200 baud)1. Abra Serial Monitor (115200 baud)

2. Find the IP shown on WiFi connection2. Encontre o IP mostrado na conexão WiFi

3. Access `http://YOUR_ESP8266_IP` in browser3. Acesse `http://SEU_IP_ESP8266` no navegador



### 2. Web Interface### 2. Interface Web

The interface provides:A interface permite:



#### 📋 Chip Information#### 📋 Informações do Chip

- **Read JEDEC ID:** - **Ler JEDEC ID:** 

  - ✅ Automatically detects: W25Q16/32/64/128/256  - ✅ Detecta automaticamente: W25Q16/32/64/128/256

  - ✅ Shows real chip capacity (2MB-32MB)  - ✅ Mostra capacidade real do chip (2MB-32MB)

  - ✅ Validates SPI communication  - ✅ Valida comunicação SPI

- **Status:** Shows if busy or write-protected- **Status:** Mostra se está ocupado ou protegido contra escrita



#### ⚙️ SPI Configuration#### ⚙️ Configuração SPI

- **Adjustable Frequency:** 0.1 MHz - 20 MHz (slider)- **Frequência Ajustável:** 0.1 MHz - 20 MHz (slider)

- **Quick Presets:** Buttons for 1/4/8/20 MHz quick testing- **Presets Rápidos:** Botões 1/4/8/20 MHz para teste rápido

- **Chunk Size Verify:** 4KB, 8KB, 16KB, 32KB, 64KB- **Chunk Size Verify:** 4KB, 8KB, 16KB, 32KB, 64KB

- **Recommendations:** - **Recomendação:** 

  - Use **1 MHz** if you encounter corruption  - Use **1 MHz** se encontrar corrupção

  - Use **4 MHz** for stability  - Use **4 MHz** para estabilidade

  - Use **8-20 MHz** for speed (short cables)  - Use **8-20 MHz** para velocidade (cabos curtos)



#### 📖 Read Operations#### 📖 Leitura

- **Read Data:** Reads specific region (address + size)- **Ler Dados:** Lê região específica (endereço + tamanho)

- **Full Dump:** - **Dump Completo:** 

  - ✅ Automatically detects chip size  - ✅ Detecta tamanho do chip automaticamente

  - ✅ Downloads 2MB-32MB according to connected chip  - ✅ Baixa 2MB-32MB conforme chip conectado

  - ✅ File saved as `bios_<timestamp>.bin`  - ✅ Arquivo salvo como `bios_<timestamp>.bin`



#### ✏️ Write Operations#### ✏️ Gravação

- **Accepted Formats:** `.bin`, `.rom`, `.bss` **← New!**- **Aceita Formatos:** `.bin`, `.rom`, `.bss` **← Novo!**

- **Maximum Size:** Up to 8MB (adjusts to detected chip)- **Tamanho Máximo:** Até 8MB (ajusta conforme chip detectado)

- **Chunk-by-Chunk System:**- **Sistema Chunk-by-Chunk:**

  - Erases sector (4KB)  - Apaga setor (4KB)

  - Writes 1KB  - Escreve 1KB

  - Verifies immediately  - Verifica imediatamente

  - Automatic retry (up to 3x per chunk)  - Retry automático (até 3x por chunk)

- **Automatic Verification:** - **Verificação Automática:** 

  - Byte-by-byte checksum  - Checksum byte-a-byte

  - Detailed error report  - Relatório detalhado de erros

  - Locates first/last error  - Localiza primeiro/último erro

  - Counts affected regions  - Conta regiões afetadas

- **Progress Bar:** Real-time updates- **Barra de Progresso:** Atualização em tempo real



#### 🗑️ Erase Operations#### 🗑️ Apagamento

- **Erase Sector:** Removes specific 4KB- **Apagar Setor:** Remove 4KB específicos

- **Erase Chip:** **⚠️ DANGER** - Removes everything (makes board unusable)- **Apagar Chip:** **⚠️ PERIGO** - Remove tudo (torna placa inutilizável)

- **Automatic Protection:** Removes protections before erasing- **Proteção Automática:** Remove proteções antes de apagar



#### 🔓 Chip Protection#### 🔓 Proteção de Chip

- **Check Protection:** Shows Status Register and BP0-BP2 bits- **Verificar Proteção:** Mostra Status Register e bits BP0-BP2

- **Remove Protection:** Clears all block protection bits- **Remover Proteção:** Limpa todos os bits de proteção de bloco

- **Auto-Remove:** System automatically removes protections before writing- **Auto-Remove:** Sistema remove proteções automaticamente antes de gravar

- **IMPORTANT:** Hardware protection (WP# pin) must be disabled (connected to VCC)- **IMPORTANTE:** Proteção de hardware (pino WP#) deve estar desabilitada (conectado a VCC)



#### 📟 Log Terminal#### 📟 Terminal de Logs

- **Verbosity Levels:** Quiet / Normal / Verbose / Debug- **Níveis de Verbosidade:** Quiet / Normal / Verbose / Debug

- **Auto-Scroll:** Enable/disable automatic scrolling- **Auto-Scroll:** Ativa/desativa rolagem automática

- **Real-Time Logs:** Updates every 1 second- **Logs em Tempo Real:** Atualização a cada 1 segundo

- **Type Filtering:** ERROR (red), WARNING (yellow), INFO (blue), DEBUG (gray)- **Filtragem por Tipo:** ERROR (vermelho), WARNING (amarelo), INFO (azul), DEBUG (cinza)



## 🔐 W25Q32BV Protection System## 🔐 Sistema de Proteção do W25Q32BV



### Hardware Protection (Pins)### Proteção por Hardware (Pinos)

The W25Q32BV has two protection levels:O W25Q32BV possui dois níveis de proteção:



**1. WP# Pin (Write Protect - Pin 3):****1. Pino WP# (Write Protect - Pin 3):**

- **GND (LOW):** Status Register **LOCKED** - won't accept modifications ❌- **GND (LOW):** Status Register **BLOQUEADO** - não aceita modificações ❌

- **VCC (HIGH):** Status Register **UNLOCKED** - accepts modifications ✅- **VCC (HIGH):** Status Register **DESBLOQUEADO** - aceita modificações ✅

- **For writing:** **MANDATORY** to connect to **3.3V**!- **Para gravação:** **OBRIGATÓRIO** conectar a **3.3V**!



**2. HOLD# Pin (Pin 7):****2. Pino HOLD# (Pin 7):**

- **GND (LOW):** SPI Communication **PAUSED** ❌- **GND (LOW):** Comunicação SPI **PAUSADA** ❌

- **VCC (HIGH):** SPI Communication **NORMAL** ✅- **VCC (HIGH):** Comunicação SPI **NORMAL** ✅

- **For writing:** **MANDATORY** to connect to **3.3V**!- **Para gravação:** **OBRIGATÓRIO** conectar a **3.3V**!



### Software Protection (Status Register)### Proteção por Software (Status Register)

BP0, BP1, BP2 bits in Status Register protect memory regions:Bits BP0, BP1, BP2 no Status Register protegem regiões de memória:



| BP2 | BP1 | BP0 | Protected Region | Addresses || BP2 | BP1 | BP0 | Região Protegida | Endereços |

|-----|-----|-----|------------------|-----------||-----|-----|-----|------------------|-----------|

| 0   | 0   | 0   | None (unlocked) ✅ | - || 0   | 0   | 0   | Nenhuma (desbloqueado) ✅ | - |

| 0   | 0   | 1   | Upper 256KB | 0x3F0000-0x3FFFFF || 0   | 0   | 1   | 256KB superior | 0x3F0000-0x3FFFFF |

| 0   | 1   | 0   | Upper 512KB | 0x3E0000-0x3FFFFF || 0   | 1   | 0   | 512KB superior | 0x3E0000-0x3FFFFF |

| 0   | 1   | 1   | Upper 1MB | 0x3C0000-0x3FFFFF || 0   | 1   | 1   | 1MB superior | 0x3C0000-0x3FFFFF |

| 1   | 0   | 0   | Upper 2MB | 0x380000-0x3FFFFF || 1   | 0   | 0   | 2MB superior | 0x380000-0x3FFFFF |

| 1   | 0   | 1   | Upper 3MB | 0x300000-0x3FFFFF || 1   | 0   | 1   | 3MB superior | 0x300000-0x3FFFFF |

| 1   | 1   | 0   | Upper Half | 0x200000-0x3FFFFF || 1   | 1   | 0   | Metade superior | 0x200000-0x3FFFFF |

| 1   | 1   | 1   | Entire Chip 🔒 | 0x000000-0x3FFFFF || 1   | 1   | 1   | Chip inteiro 🔒 | 0x000000-0x3FFFFF |



**How to use protection:****Como usar a proteção:**

1. **Before writing:** Click "🔍 Check Protection"1. **Antes de gravar:** Clique em "🔍 Verificar Proteção"

2. If protected: Click "🔓 Remove Protection"2. Se protegido: Clique em "🔓 Remover Proteção"

3. Make sure **WP# is at VCC (3.3V)**!3. Certifique-se que **WP# está em VCC (3.3V)**!

4. If WP# is at GND, protection removal **WON'T WORK**!4. Se WP# estiver em GND, a remoção de proteção **NÃO FUNCIONARÁ**!



## ⚠️ CRITICAL WARNINGS## ⚠️ AVISOS CRÍTICOS



### 🚨 BACKUP FIRST!### 🚨 BACKUP PRIMEIRO!

**ALWAYS** backup the original BIOS before any modification:**SEMPRE** faça backup do BIOS original antes de qualquer modificação:

1. Click "Full Dump"1. Clique em "Dump Completo (4MB)"

2. Save the `.bin` file to a safe location2. Salve o arquivo `.bin` em local seguro

3. Keep multiple copies3. Guarde múltiplas cópias



### 🔴 Programming Risks### 🔴 Riscos da Gravação

- **Corrupted BIOS = Unusable motherboard**- **BIOS corrompida = Placa mãe inutilizável**

- Only program BIOS compatible with your H61 board- Apenas grave BIOS compatível com sua placa H61

- Never power off during programming- Nunca desligue durante gravação

- Verify file integrity before writing- Verifique integridade do arquivo antes de gravar



### ⚡ Electrical Precautions### ⚡ Cuidados Elétricos

- **Completely power off** the motherboard (power cable + battery)- **Desligue completamente** a placa mãe (cabo força + bateria)

- Ensure no residual power- Certifique-se que não há energia residual

- Use anti-static wrist strap before touching components- Use pulso estática antes de tocar componentes

- Connect GND first, then other pins- Conecte primeiro GND, depois outros pinos



## 🔧 Troubleshooting## 🔧 Solução de Problemas



### ❌ Error: "4001 bytes corrupted" or partial corruption### ❌ Erro: "4001 bytes corrompidos" ou corrupção parcial

**Root cause:** Critical bug in Base64 decoding (FIXED in v2.0!)**Causa raiz:** Bug crítico na decodificação Base64 (CORRIGIDO na v2.0!)



**Solutions:****Soluções:**

1. **Update to latest version** (Base64 fix implemented)1. **Atualize para versão mais recente** (correção de Base64 implementada)

2. **Reduce SPI speed:**2. **Reduza velocidade SPI:**

   - Test 1 MHz first   - Teste 1 MHz primeiro

   - If OK, gradually increase to 4 MHz → 8 MHz   - Se OK, aumente gradualmente para 4 MHz → 8 MHz

3. **Improve physical connections:**3. **Melhore conexões físicas:**

   - Use shorter cables (10-15cm)   - Use cabos mais curtos (10-15cm)

   - Check for loose wires   - Verifique se há fios soltos

   - Confirm common ground   - Confirme aterramento comum

4. **Check detailed logs:**4. **Verifique logs detalhados:**

   - Browser console (F12): chunk sizes sent   - Console do navegador (F12): tamanho de chunks enviados

   - Serial Monitor: bytes received/decoded   - Serial Monitor: bytes recebidos/decodificados



### 🔍 ID not detected or incorrect JEDEC### 🔍 ID não detectado ou JEDEC incorreto

- Verify SPI connections (pin order)- Verifique conexões SPI (ordem dos pinos)

- Confirm stable 3.3V power- Confirme alimentação estável 3.3V

- Test jumper continuity- Teste continuidade dos jumpers

- Check if chip is soldered inverted- Verifique se chip não está soldado invertido

- Use shorter cables (<15cm)- Use cabos mais curtos (<15cm)



### 🚫 Write error / "Chip protected"### 🚫 Erro de gravação / "Chip protegido"

- **Chip write-protected:**- **Chip protegido contra escrita:**

  - Verify **WP# (Pin 3)** is connected to **3.3V** (not GND!)  - Verifique se **WP# (Pin 3)** está conectado a **3.3V** (não GND!)

  - Click "🔍 Check Protection" in interface  - Clique em "🔍 Verificar Proteção" na interface

  - If protected, click "🔓 Remove Protection"  - Se protegido, clique em "🔓 Remover Proteção"

  - **If WP# is at GND, removal WON'T work!**  - **Se WP# estiver em GND, a remoção NÃO funcionará!**

- **Chip not responding:**- **Chip não responde:**

  - Verify **HOLD# (Pin 7)** is connected to **3.3V** (not GND!)  - Verifique se **HOLD# (Pin 7)** está conectado a **3.3V** (não GND!)

  - If HOLD# at GND, chip is permanently paused  - Se HOLD# em GND, chip fica pausado permanentemente

- Insufficient voltage (use stable 3.3V source)- Tensão insuficiente (use fonte estável 3.3V)

- Unstable connection during operation- Conexão instável durante operação



### 📡 ESP8266 won't connect to WiFi### 📡 ESP8266 não conecta WiFi

- Verify SSID and password in code- Verifique SSID e senha no código

- 2.4GHz network (ESP8266 doesn't support 5GHz)- Rede 2.4GHz (ESP8266 não suporta 5GHz)

- If it fails, ESP creates Access Point:- Se falhar, ESP cria Access Point:

  - Name: `ESP8266-BIOS-Programmer`  - Nome: `ESP8266-BIOS-Programmer`

  - Password: `12345678`  - Senha: `12345678`

  - IP: `192.168.4.1`  - IP: `192.168.4.1`



### 💾 Motherboard won't boot after programming### 💾 Placa mãe não inicia após gravação

1. **DON'T PANIC**1. **NÃO ENTRE EM PÂNICO**

2. Re-program the original backup2. Regrave o backup original

3. If backup is corrupted, find official BIOS from manufacturer3. Se backup corrompido, procure BIOS oficial da fabricante

4. Use board's BIOS recovery tool (if available)4. Use ferramenta de recuperação de BIOS da placa (se disponível)



## 📚 W25Q32BV Technical Commands## 📚 Comandos Técnicos W25Q32BV



| Command | Code | Description || Comando | Código | Descrição |

|---------|--------|-----------||---------|--------|-----------|

| JEDEC ID | 0x9F | Read ID: EF4016 || JEDEC ID | 0x9F | Lê ID: EF4016 |

| Read Data | 0x03 | Sequential read || Read Data | 0x03 | Leitura sequencial |

| Write Enable | 0x06 | Enable writing || Write Enable | 0x06 | Habilita escrita |

| Page Program | 0x02 | Program page (256B) || Page Program | 0x02 | Grava página (256B) |

| Read Status | 0x05 | Status register || Read Status | 0x05 | Status register |

| Sector Erase | 0x20 | Erase sector (64KB) || Sector Erase | 0x20 | Apaga setor (64KB) |

| Chip Erase | 0xC7 | Erase all || Chip Erase | 0xC7 | Apaga tudo |



## 🏗️ Code Architecture## 🏗️ Arquitetura do Código



``````

esp8266_w25q32_programmer.inoesp8266_w25q32_programmer.ino

├── 🔧 Configuration├── 🔧 Configurações

│   ├── WiFi/AP dual mode│   ├── WiFi/AP dual mode

│   ├── Configurable SPI (0.1-20 MHz)│   ├── SPI configurável (0.1-20 MHz)

│   └── Automatic chip detection│   └── Detecção automática de chip

├── 📡 Low-level SPI Functions├── 📡 Funções SPI low-level

│   ├── readJedecId() → Read ID + detect capacity│   ├── readJedecId() → Lê ID + detecta capacidade

│   ├── detectFlashSize() → Adjust FLASH_SIZE dynamically│   ├── detectFlashSize() → Ajusta FLASH_SIZE dinamicamente

│   ├── readData() → Sequential read│   ├── readData() → Leitura sequencial

│   ├── programPage() → Write 256 bytes│   ├── programPage() → Grava 256 bytes

│   ├── sectorErase() → Erase 4KB│   ├── sectorErase() → Apaga 4KB

│   └── chipErase() → Erase all│   └── chipErase() → Apaga tudo

├── 🔒 Protection System├── 🔒 Sistema de Proteção

│   ├── readStatus() / readStatus2()│   ├── readStatus() / readStatus2()

│   ├── writeStatusRegister()│   ├── writeStatusRegister()

│   ├── disableAllProtection() → Remove BP0-BP2│   ├── disableAllProtection() → Remove BP0-BP2

│   └── isWriteProtected() → Check protection│   └── isWriteProtected() → Verifica proteção

├── 📤 Streaming System (large files)├── 📤 Sistema de Streaming (arquivos grandes)

│   ├── 1KB chunks for memory efficiency│   ├── Chunks de 1KB para economia de memória

│   ├── Write-Verify-Retry per chunk (up to 3x)│   ├── Write-Verify-Retry por chunk (até 3x)

│   ├── On-demand erase (erase sector when needed)│   ├── Erase on-demand (apaga setor quando necessário)

│   └── Real-time progress tracking│   └── Progress tracking em tempo real

├── 🌐 HTTP Web Server├── 🌐 Servidor Web HTTP

│   ├── handleInterface() → Embedded HTML/JS/CSS│   ├── handleInterface() → HTML/JS/CSS embutido

│   ├── handleReadId() → /id (with chip detection)│   ├── handleReadId() → /id (com detecção de chip)

│   ├── handleRead() → /read (dynamic reading)│   ├── handleRead() → /read (leitura dinâmica)

│   ├── handleWriteStream() → /writeStream (init/finish)│   ├── handleWriteStream() → /writeStream (init/finish)

│   ├── handleWriteChunk() → /writeChunk (ROBUST Base64)│   ├── handleWriteChunk() → /writeChunk (Base64 ROBUSTO)

│   ├── handleSetSpiFrequency() → /setSpiFrequency│   ├── handleSetSpiFrequency() → /setSpiFrequency

│   ├── handleCheckProtection() → /checkProtection│   ├── handleCheckProtection() → /checkProtection

│   ├── handleRemoveProtection() → /removeProtection│   ├── handleRemoveProtection() → /removeProtection

│   └── handleSystem() → /system (info + detected size)│   └── handleSystem() → /system (info + tamanho detectado)

├── 📊 Logging System├── 📊 Sistema de Logs

│   ├── 4 levels: Quiet/Normal/Verbose/Debug│   ├── 4 níveis: Quiet/Normal/Verbose/Debug

│   ├── Circular buffer (50 messages)│   ├── Buffer circular (50 mensagens)

│   └── /logs endpoint for web terminal│   └── Endpoint /logs para web terminal

└── 🖥️ HTML/JS Interface└── 🖥️ Interface HTML/JS

    ├── SPI controls with presets    ├── Controles de SPI com presets

    ├── Streaming upload with progress    ├── Upload streaming com progresso

    ├── Byte-by-byte verification with report    ├── Verificação byte-a-byte com relatório

    ├── Real-time terminal    ├── Terminal em tempo real

    └── Console logs (F12) for debugging    └── Console logs (F12) para debug

``````



## 🐛 Fixed Bugs (v2.0)## 🐛 Bugs Corrigidos (v2.0)



### 🔴 Critical Bug: Base64 Decoding (RESOLVED)### 🔴 Bug Crítico: Decodificação Base64 (RESOLVIDO)

**Symptom:** 4001 bytes corrupted in large uploads**Sintoma:** 4001 bytes corrompidos em uploads grandes



**Cause:** 3 bugs in manual Base64 decoding:**Causa:** 3 bugs na decodificação Base64 manual:

1. ❌ Loop stopped before end (`length() - 3`)1. ❌ Loop parava antes do fim (`length() - 3`)

2. ❌ Premature break on padding2. ❌ Break prematuro no padding

3. ❌ Incorrect byte extraction logic3. ❌ Lógica de extração de bytes incorreta



**Solution:** Base64 decoder completely rewritten:**Solução:** Decodificador Base64 reescrito do zero:

- ✅ Processes ALL characters- ✅ Processa TODOS os caracteres

- ✅ Accumulates bits gradually (not in blocks)- ✅ Acumula bits gradualmente (não por blocos)

- ✅ Handles padding correctly- ✅ Trata padding corretamente

- ✅ Validates each byte- ✅ Valida cada byte

- ✅ Debug logs (input/output size)- ✅ Logs de debug (tamanho entrada/saída)



**Result:** **0% corruption** in tests with 8MB files**Resultado:** **0% de corrupção** em testes com arquivos de 8MB



## 📈 Changelog## 📈 Changelog



### v2.0 (2025-10-10)### v2.0 (2025-10-10)

- ✅ **Automatic detection of W25Q16/32/64/128/256 chips**- ✅ **Detecção automática de chips W25Q16/32/64/128/256**

- ✅ **Support for 8MB+ (up to 32MB)**- ✅ **Suporte para 8MB+ (até 32MB)**

- ✅ **Acceptance of .bss files**- ✅ **Aceitação de arquivos .bss**

- ✅ **Critical Base64 decoding fix** (eliminates corruption)- ✅ **Correção crítica de decodificação Base64** (elimina corrupção)

- ✅ **Base64 debug logs** (client + server)- ✅ **Logs de debug Base64** (cliente + servidor)

- ✅ **Dynamic dump** (adjusts to chip size)- ✅ **Dump dinâmico** (ajusta ao tamanho do chip)

- ✅ **Interface shows chip name and capacity**- ✅ **Interface mostra nome e capacidade do chip**

- ✅ **/system endpoint returns detected size**- ✅ **Endpoint /system retorna tamanho detectado**



### v1.0 (Original)### v1.0 (Original)

- W25Q32 support (4MB)- Suporte W25Q32 (4MB)

- Streaming upload- Upload streaming

- Basic verification- Verificação básica

- SPI control- Controle de SPI



## 📄 License## 📄 Licença



This project is provided "as is" for educational purposes. Use at your own risk. The author is not responsible for damages caused by improper use.Este projeto é fornecido "como está" para fins educacionais. Use por sua conta e risco. O autor não se responsabiliza por danos causados por uso inadequado.



------



**⚠️ FINAL REMINDER:** BIOS programming is a high-risk operation. Always have backup and adequate technical knowledge before proceeding.**⚠️ LEMBRETE FINAL:** Gravação de BIOS é operação de alto risco. Sempre tenha backup e conhecimento técnico adequado antes de prosseguir.
