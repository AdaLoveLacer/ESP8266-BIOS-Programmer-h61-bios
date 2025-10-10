# ESP8266 W25Qxx BIOS Programmer

Gravador/Depurador de EPROM para chips de BIOS da s├®rie W25Qxx (Winbond) usando ESP8266, com **detec├º├úo autom├ítica de capacidade** e suporte de **2MB at├® 32MB**.

## Ô£¿ Funcionalidades Principais

- ­ƒöì **Detec├º├úo Autom├ítica de Chip**: Identifica W25Q16/32/64/128/256 via JEDEC ID
- ­ƒôª **Suporte Multi-Capacidade**: 2MB, 4MB, 8MB, 16MB, 32MB
- ­ƒôñ **Upload Streaming Otimizado**: Chunks de 1KB com verify-retry autom├ítico
- ­ƒöÆ **Gerenciamento de Prote├º├úo**: Remove prote├º├Áes de bloco automaticamente
- ÔÜí **Controle de Velocidade SPI**: 0.1-20 MHz ajust├ível em runtime (presets: 1/4/8/20 MHz)
- Ô£à **Verifica├º├úo Autom├ítica**: Checksum byte-a-byte com relat├│rio detalhado de erros
- ­ƒîÉ **Interface Web Responsiva**: HTML/JS embutido com terminal em tempo real
- ­ƒôè **Logs Detalhados**: Sistema de verbosidade (Quiet/Normal/Verbose/Debug)
- ­ƒöº **Decodifica├º├úo Base64 Robusta**: Corre├º├úo de bugs cr├¡ticos de transfer├¬ncia

## ­ƒöº Hardware Necess├írio

- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
- Chip W25Qxx Series:
  - Ô£à W25Q16 (2MB) - JEDEC ID: EF4015
  - Ô£à W25Q32 (4MB) - JEDEC ID: EF4016
  - Ô£à W25Q64 (8MB) - JEDEC ID: EF4017 **ÔåÉ Novo suporte!**
  - Ô£à W25Q128 (16MB) - JEDEC ID: EF4018
  - Ô£à W25Q256 (32MB) - JEDEC ID: EF4019
- Jumpers para conex├úo (recomendado: cabos curtos 10-15cm)
- Fonte 3.3V est├ível (ESP8266 j├í fornece)

## ­ƒôî Pinagem ESP8266 Ôåö W25Qxx Series

| ESP8266 | Pino | W25Qxx | Descri├º├úo |
|---------|------|----------|-----------|
| D8      | GPIO15 | CS (Pin 1)  | Chip Select |
| D7      | GPIO13 | DI (Pin 5)  | Data Input (MOSI) |
| D6      | GPIO12 | DO (Pin 2)  | Data Output (MISO) |
| D5      | GPIO14 | CLK (Pin 6) | Serial Clock |
| 3.3V    | 3.3V   | VCC (Pin 8) | Alimenta├º├úo |
| 3.3V    | 3.3V   | **WP# (Pin 3)** | **Write Protect - DEVE estar em VCC!** |
| 3.3V    | 3.3V   | **HOLD# (Pin 7)** | **Hold/Reset - DEVE estar em VCC!** |
| GND     | GND    | GND (Pin 4) | Terra |

**IMPORTANTE:** 
- W25Qxx opera APENAS em 3.3V. N├âO use 5V!
- **ÔÜá´©Å CR├ìTICO:** Pinos 3 (WP#) e 7 (HOLD#) **DEVEM** estar conectados a 3.3V para permitir grava├º├úo!
- Se deixar WP# em GND, o Status Register fica bloqueado e n├úo ├® poss├¡vel remover prote├º├úo de escrita!
- **Use cabos curtos (10-15cm)** para evitar problemas de integridade de sinal

## ­ƒöì Pinout W25Qxx (SOIC-8)

```
     ÔöîÔöÇÔöÇÔöÇ 1 CS (Chip Select)
     Ôöé ÔöîÔöÇ 2 DO (MISO)
     Ôöé Ôöé Ôöî 3 WP# (Write Protect) ÔåÆ ÔÜá´©Å Conectar a VCC (3.3V)!
     Ôöé Ôöé ÔöéÔöî 4 GND
  ÔöîÔöÇÔöÇÔû╝ÔöÇÔû╝ÔöÇÔû╝Ôû╝ÔöÇÔöÉ
1ÔöÇÔöñ  ÔùÅ      Ôö£ÔöÇ8 VCC (3.3V)
2ÔöÇÔöñ         Ôö£ÔöÇ7 HOLD# ÔåÆ ÔÜá´©Å Conectar a VCC (3.3V)!
3ÔöÇÔöñ  W25QxxÔö£ÔöÇ6 CLK
4ÔöÇÔöñ  SeriesÔö£ÔöÇ5 DI (MOSI)
  ÔööÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÿ
```

**ÔÜá´©Å ATEN├ç├âO - CONFIGURA├ç├âO CR├ìTICA:**
- **Pino 3 (WP#)**: **DEVE** estar conectado a **3.3V** (n├úo deixe flutuante ou em GND!)
- **Pino 7 (HOLD#)**: **DEVE** estar conectado a **3.3V** (n├úo deixe flutuante ou em GND!)
- Se WP# estiver em GND, o Status Register fica protegido e voc├¬ **N├âO CONSEGUIR├ü** remover prote├º├úo de blocos!
- Se HOLD# estiver em GND, a comunica├º├úo SPI ficar├í pausada permanentemente!

### ­ƒöî Diagrama de Conex├úo Completo

```
ESP8266                      W25Qxx (SOIC-8)
                        
3.3V ÔöÇÔöÇÔöÇÔöÇÔö¼ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔû║ Pin 8 (VCC)
         Ôöé
         Ôö£ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔû║ Pin 3 (WP#)   ÔÜá´©Å CR├ìTICO!
         Ôöé
         ÔööÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔû║ Pin 7 (HOLD#) ÔÜá´©Å CR├ìTICO!

GND ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔû║ Pin 4 (GND)

D8 (GPIO15) ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔû║ Pin 1 (CS)
D7 (GPIO13) ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔû║ Pin 5 (DI/MOSI)
D6 (GPIO12) ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔû║ Pin 2 (DO/MISO)
D5 (GPIO14) ÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔöÇÔû║ Pin 6 (CLK)
```

**Lista de Verifica├º├úo antes de gravar:**
- [ ] VCC conectado a 3.3V (Pin 8)
- [ ] GND conectado (Pin 4)
- [ ] **WP# conectado a 3.3V (Pin 3)** ÔÜá´©Å
- [ ] **HOLD# conectado a 3.3V (Pin 7)** ÔÜá´©Å
- [ ] CS conectado a D8 (Pin 1)
- [ ] MOSI conectado a D7 (Pin 5)
- [ ] MISO conectado a D6 (Pin 2)
- [ ] CLK conectado a D5 (Pin 6)
- [ ] **Cabos curtos (10-15cm m├íximo)** ­ƒôÅ

## ­ƒøá´©Å Configura├º├úo Arduino IDE

### 1. Instalar ESP8266 Core
1. File ÔåÆ Preferences
2. Em "Additional Board Manager URLs" adicione:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Tools ÔåÆ Board ÔåÆ Boards Manager
4. Procure "ESP8266" e instale

### 2. Configurar Board
- **Board:** "NodeMCU 1.0 (ESP-12E Module)" ou seu modelo espec├¡fico
- **Upload Speed:** 115200
- **CPU Frequency:** 80 MHz
- **Flash Size:** 4MB (FS:2MB OTA:~1019KB)
- **Port:** Selecione a porta COM do ESP8266

### 3. Configurar WiFi
Edite as linhas no arquivo `esp8266_w25q32_programmer.ino`:
```cpp
const char* ssid = "SeuWiFi";        // Nome da sua rede
const char* password = "SuaSenha";   // Senha da sua rede
```

### 4. Upload do Firmware
1. Conecte o ESP8266 via USB
2. Abra `esp8266_w25q32_programmer.ino`
3. Clique em Upload (ÔåÆ)
4. Aguarde conclus├úo

## ­ƒîÉ Utilizando o Gravador

### 1. Conectar ao ESP8266
1. Abra Serial Monitor (115200 baud)
2. Encontre o IP mostrado na conex├úo WiFi
3. Acesse `http://SEU_IP_ESP8266` no navegador

### 2. Interface Web
A interface permite:

#### ­ƒôï Informa├º├Áes do Chip
- **Ler JEDEC ID:** 
  - Ô£à Detecta automaticamente: W25Q16/32/64/128/256
  - Ô£à Mostra capacidade real do chip (2MB-32MB)
  - Ô£à Valida comunica├º├úo SPI
- **Status:** Mostra se est├í ocupado ou protegido contra escrita

#### ÔÜÖ´©Å Configura├º├úo SPI
- **Frequ├¬ncia Ajust├ível:** 0.1 MHz - 20 MHz (slider)
- **Presets R├ípidos:** Bot├Áes 1/4/8/20 MHz para teste r├ípido
- **Chunk Size Verify:** 4KB, 8KB, 16KB, 32KB, 64KB
- **Recomenda├º├úo:** 
  - Use **1 MHz** se encontrar corrup├º├úo
  - Use **4 MHz** para estabilidade
  - Use **8-20 MHz** para velocidade (cabos curtos)

#### ­ƒôû Leitura
- **Ler Dados:** L├¬ regi├úo espec├¡fica (endere├ºo + tamanho)
- **Dump Completo:** 
  - Ô£à Detecta tamanho do chip automaticamente
  - Ô£à Baixa 2MB-32MB conforme chip conectado
  - Ô£à Arquivo salvo como `bios_<timestamp>.bin`

#### Ô£Å´©Å Grava├º├úo
- **Aceita Formatos:** `.bin`, `.rom`, `.bss` **ÔåÉ Novo!**
- **Tamanho M├íximo:** At├® 8MB (ajusta conforme chip detectado)
- **Sistema Chunk-by-Chunk:**
  - Apaga setor (4KB)
  - Escreve 1KB
  - Verifica imediatamente
  - Retry autom├ítico (at├® 3x por chunk)
- **Verifica├º├úo Autom├ítica:** 
  - Checksum byte-a-byte
  - Relat├│rio detalhado de erros
  - Localiza primeiro/├║ltimo erro
  - Conta regi├Áes afetadas
- **Barra de Progresso:** Atualiza├º├úo em tempo real

#### ­ƒùæ´©Å Apagamento
- **Apagar Setor:** Remove 4KB espec├¡ficos
- **Apagar Chip:** **ÔÜá´©Å PERIGO** - Remove tudo (torna placa inutiliz├ível)
- **Prote├º├úo Autom├ítica:** Remove prote├º├Áes antes de apagar

#### ­ƒöô Prote├º├úo de Chip
- **Verificar Prote├º├úo:** Mostra Status Register e bits BP0-BP2
- **Remover Prote├º├úo:** Limpa todos os bits de prote├º├úo de bloco
- **Auto-Remove:** Sistema remove prote├º├Áes automaticamente antes de gravar
- **IMPORTANTE:** Prote├º├úo de hardware (pino WP#) deve estar desabilitada (conectado a VCC)

#### ­ƒôƒ Terminal de Logs
- **N├¡veis de Verbosidade:** Quiet / Normal / Verbose / Debug
- **Auto-Scroll:** Ativa/desativa rolagem autom├ítica
- **Logs em Tempo Real:** Atualiza├º├úo a cada 1 segundo
- **Filtragem por Tipo:** ERROR (vermelho), WARNING (amarelo), INFO (azul), DEBUG (cinza)

## ­ƒöÉ Sistema de Prote├º├úo do W25Q32BV

### Prote├º├úo por Hardware (Pinos)
O W25Q32BV possui dois n├¡veis de prote├º├úo:

**1. Pino WP# (Write Protect - Pin 3):**
- **GND (LOW):** Status Register **BLOQUEADO** - n├úo aceita modifica├º├Áes ÔØî
- **VCC (HIGH):** Status Register **DESBLOQUEADO** - aceita modifica├º├Áes Ô£à
- **Para grava├º├úo:** **OBRIGAT├ôRIO** conectar a **3.3V**!

**2. Pino HOLD# (Pin 7):**
- **GND (LOW):** Comunica├º├úo SPI **PAUSADA** ÔØî
- **VCC (HIGH):** Comunica├º├úo SPI **NORMAL** Ô£à
- **Para grava├º├úo:** **OBRIGAT├ôRIO** conectar a **3.3V**!

### Prote├º├úo por Software (Status Register)
Bits BP0, BP1, BP2 no Status Register protegem regi├Áes de mem├│ria:

| BP2 | BP1 | BP0 | Regi├úo Protegida | Endere├ºos |
|-----|-----|-----|------------------|-----------|
| 0   | 0   | 0   | Nenhuma (desbloqueado) Ô£à | - |
| 0   | 0   | 1   | 256KB superior | 0x3F0000-0x3FFFFF |
| 0   | 1   | 0   | 512KB superior | 0x3E0000-0x3FFFFF |
| 0   | 1   | 1   | 1MB superior | 0x3C0000-0x3FFFFF |
| 1   | 0   | 0   | 2MB superior | 0x380000-0x3FFFFF |
| 1   | 0   | 1   | 3MB superior | 0x300000-0x3FFFFF |
| 1   | 1   | 0   | Metade superior | 0x200000-0x3FFFFF |
| 1   | 1   | 1   | Chip inteiro ­ƒöÆ | 0x000000-0x3FFFFF |

**Como usar a prote├º├úo:**
1. **Antes de gravar:** Clique em "­ƒöì Verificar Prote├º├úo"
2. Se protegido: Clique em "­ƒöô Remover Prote├º├úo"
3. Certifique-se que **WP# est├í em VCC (3.3V)**!
4. Se WP# estiver em GND, a remo├º├úo de prote├º├úo **N├âO FUNCIONAR├ü**!

## ÔÜá´©Å AVISOS CR├ìTICOS

### ­ƒÜ¿ BACKUP PRIMEIRO!
**SEMPRE** fa├ºa backup do BIOS original antes de qualquer modifica├º├úo:
1. Clique em "Dump Completo (4MB)"
2. Salve o arquivo `.bin` em local seguro
3. Guarde m├║ltiplas c├│pias

### ­ƒö┤ Riscos da Grava├º├úo
- **BIOS corrompida = Placa m├úe inutiliz├ível**
- Apenas grave BIOS compat├¡vel com sua placa H61
- Nunca desligue durante grava├º├úo
- Verifique integridade do arquivo antes de gravar

### ÔÜí Cuidados El├®tricos
- **Desligue completamente** a placa m├úe (cabo for├ºa + bateria)
- Certifique-se que n├úo h├í energia residual
- Use pulso est├ítica antes de tocar componentes
- Conecte primeiro GND, depois outros pinos

## ­ƒöº Solu├º├úo de Problemas

### ÔØî Erro: "4001 bytes corrompidos" ou corrup├º├úo parcial
**Causa raiz:** Bug cr├¡tico na decodifica├º├úo Base64 (CORRIGIDO na v2.0!)

**Solu├º├Áes:**
1. **Atualize para vers├úo mais recente** (corre├º├úo de Base64 implementada)
2. **Reduza velocidade SPI:**
   - Teste 1 MHz primeiro
   - Se OK, aumente gradualmente para 4 MHz ÔåÆ 8 MHz
3. **Melhore conex├Áes f├¡sicas:**
   - Use cabos mais curtos (10-15cm)
   - Verifique se h├í fios soltos
   - Confirme aterramento comum
4. **Verifique logs detalhados:**
   - Console do navegador (F12): tamanho de chunks enviados
   - Serial Monitor: bytes recebidos/decodificados

### ­ƒöì ID n├úo detectado ou JEDEC incorreto
- Verifique conex├Áes SPI (ordem dos pinos)
- Confirme alimenta├º├úo est├ível 3.3V
- Teste continuidade dos jumpers
- Verifique se chip n├úo est├í soldado invertido
- Use cabos mais curtos (<15cm)

### ­ƒÜ½ Erro de grava├º├úo / "Chip protegido"
- **Chip protegido contra escrita:**
  - Verifique se **WP# (Pin 3)** est├í conectado a **3.3V** (n├úo GND!)
  - Clique em "­ƒöì Verificar Prote├º├úo" na interface
  - Se protegido, clique em "­ƒöô Remover Prote├º├úo"
  - **Se WP# estiver em GND, a remo├º├úo N├âO funcionar├í!**
- **Chip n├úo responde:**
  - Verifique se **HOLD# (Pin 7)** est├í conectado a **3.3V** (n├úo GND!)
  - Se HOLD# em GND, chip fica pausado permanentemente
- Tens├úo insuficiente (use fonte est├ível 3.3V)
- Conex├úo inst├ível durante opera├º├úo

### ­ƒôí ESP8266 n├úo conecta WiFi
- Verifique SSID e senha no c├│digo
- Rede 2.4GHz (ESP8266 n├úo suporta 5GHz)
- Se falhar, ESP cria Access Point:
  - Nome: `ESP8266-BIOS-Programmer`
  - Senha: `12345678`
  - IP: `192.168.4.1`

### ­ƒÆ¥ Placa m├úe n├úo inicia ap├│s grava├º├úo
1. **N├âO ENTRE EM P├éNICO**
2. Regrave o backup original
3. Se backup corrompido, procure BIOS oficial da fabricante
4. Use ferramenta de recupera├º├úo de BIOS da placa (se dispon├¡vel)

## ­ƒôÜ Comandos T├®cnicos W25Q32BV

| Comando | C├│digo | Descri├º├úo |
|---------|--------|-----------|
| JEDEC ID | 0x9F | L├¬ ID: EF4016 |
| Read Data | 0x03 | Leitura sequencial |
| Write Enable | 0x06 | Habilita escrita |
| Page Program | 0x02 | Grava p├ígina (256B) |
| Read Status | 0x05 | Status register |
| Sector Erase | 0x20 | Apaga setor (64KB) |
| Chip Erase | 0xC7 | Apaga tudo |

## ­ƒÅù´©Å Arquitetura do C├│digo

```
esp8266_w25q32_programmer.ino
Ôö£ÔöÇÔöÇ ­ƒöº Configura├º├Áes
Ôöé   Ôö£ÔöÇÔöÇ WiFi/AP dual mode
Ôöé   Ôö£ÔöÇÔöÇ SPI configur├ível (0.1-20 MHz)
Ôöé   ÔööÔöÇÔöÇ Detec├º├úo autom├ítica de chip
Ôö£ÔöÇÔöÇ ­ƒôí Fun├º├Áes SPI low-level
Ôöé   Ôö£ÔöÇÔöÇ readJedecId() ÔåÆ L├¬ ID + detecta capacidade
Ôöé   Ôö£ÔöÇÔöÇ detectFlashSize() ÔåÆ Ajusta FLASH_SIZE dinamicamente
Ôöé   Ôö£ÔöÇÔöÇ readData() ÔåÆ Leitura sequencial
Ôöé   Ôö£ÔöÇÔöÇ programPage() ÔåÆ Grava 256 bytes
Ôöé   Ôö£ÔöÇÔöÇ sectorErase() ÔåÆ Apaga 4KB
Ôöé   ÔööÔöÇÔöÇ chipErase() ÔåÆ Apaga tudo
Ôö£ÔöÇÔöÇ ­ƒöÆ Sistema de Prote├º├úo
Ôöé   Ôö£ÔöÇÔöÇ readStatus() / readStatus2()
Ôöé   Ôö£ÔöÇÔöÇ writeStatusRegister()
Ôöé   Ôö£ÔöÇÔöÇ disableAllProtection() ÔåÆ Remove BP0-BP2
Ôöé   ÔööÔöÇÔöÇ isWriteProtected() ÔåÆ Verifica prote├º├úo
Ôö£ÔöÇÔöÇ ­ƒôñ Sistema de Streaming (arquivos grandes)
Ôöé   Ôö£ÔöÇÔöÇ Chunks de 1KB para economia de mem├│ria
Ôöé   Ôö£ÔöÇÔöÇ Write-Verify-Retry por chunk (at├® 3x)
Ôöé   Ôö£ÔöÇÔöÇ Erase on-demand (apaga setor quando necess├írio)
Ôöé   ÔööÔöÇÔöÇ Progress tracking em tempo real
Ôö£ÔöÇÔöÇ ­ƒîÉ Servidor Web HTTP
Ôöé   Ôö£ÔöÇÔöÇ handleInterface() ÔåÆ HTML/JS/CSS embutido
Ôöé   Ôö£ÔöÇÔöÇ handleReadId() ÔåÆ /id (com detec├º├úo de chip)
Ôöé   Ôö£ÔöÇÔöÇ handleRead() ÔåÆ /read (leitura din├ómica)
Ôöé   Ôö£ÔöÇÔöÇ handleWriteStream() ÔåÆ /writeStream (init/finish)
Ôöé   Ôö£ÔöÇÔöÇ handleWriteChunk() ÔåÆ /writeChunk (Base64 ROBUSTO)
Ôöé   Ôö£ÔöÇÔöÇ handleSetSpiFrequency() ÔåÆ /setSpiFrequency
Ôöé   Ôö£ÔöÇÔöÇ handleCheckProtection() ÔåÆ /checkProtection
Ôöé   Ôö£ÔöÇÔöÇ handleRemoveProtection() ÔåÆ /removeProtection
Ôöé   ÔööÔöÇÔöÇ handleSystem() ÔåÆ /system (info + tamanho detectado)
Ôö£ÔöÇÔöÇ ­ƒôè Sistema de Logs
Ôöé   Ôö£ÔöÇÔöÇ 4 n├¡veis: Quiet/Normal/Verbose/Debug
Ôöé   Ôö£ÔöÇÔöÇ Buffer circular (50 mensagens)
Ôöé   ÔööÔöÇÔöÇ Endpoint /logs para web terminal
ÔööÔöÇÔöÇ ­ƒûÑ´©Å Interface HTML/JS
    Ôö£ÔöÇÔöÇ Controles de SPI com presets
    Ôö£ÔöÇÔöÇ Upload streaming com progresso
    Ôö£ÔöÇÔöÇ Verifica├º├úo byte-a-byte com relat├│rio
    Ôö£ÔöÇÔöÇ Terminal em tempo real
    ÔööÔöÇÔöÇ Console logs (F12) para debug
```

## ­ƒÉø Bugs Corrigidos (v2.0)

### ­ƒö┤ Bug Cr├¡tico: Decodifica├º├úo Base64 (RESOLVIDO)
**Sintoma:** 4001 bytes corrompidos em uploads grandes

**Causa:** 3 bugs na decodifica├º├úo Base64 manual:
1. ÔØî Loop parava antes do fim (`length() - 3`)
2. ÔØî Break prematuro no padding
3. ÔØî L├│gica de extra├º├úo de bytes incorreta

**Solu├º├úo:** Decodificador Base64 reescrito do zero:
- Ô£à Processa TODOS os caracteres
- Ô£à Acumula bits gradualmente (n├úo por blocos)
- Ô£à Trata padding corretamente
- Ô£à Valida cada byte
- Ô£à Logs de debug (tamanho entrada/sa├¡da)

**Resultado:** **0% de corrup├º├úo** em testes com arquivos de 8MB

## ­ƒôê Changelog

### v2.0 (2025-10-10)
- Ô£à **Detec├º├úo autom├ítica de chips W25Q16/32/64/128/256**
- Ô£à **Suporte para 8MB+ (at├® 32MB)**
- Ô£à **Aceita├º├úo de arquivos .bss**
- Ô£à **Corre├º├úo cr├¡tica de decodifica├º├úo Base64** (elimina corrup├º├úo)
- Ô£à **Logs de debug Base64** (cliente + servidor)
- Ô£à **Dump din├ómico** (ajusta ao tamanho do chip)
- Ô£à **Interface mostra nome e capacidade do chip**
- Ô£à **Endpoint /system retorna tamanho detectado**

### v1.0 (Original)
- Suporte W25Q32 (4MB)
- Upload streaming
- Verifica├º├úo b├ísica
- Controle de SPI

## ­ƒôä Licen├ºa

Este projeto ├® fornecido "como est├í" para fins educacionais. Use por sua conta e risco. O autor n├úo se responsabiliza por danos causados por uso inadequado.

---

**ÔÜá´©Å LEMBRETE FINAL:** Grava├º├úo de BIOS ├® opera├º├úo de alto risco. Sempre tenha backup e conhecimento t├®cnico adequado antes de prosseguir.
