# ESP8266 W25Qxx BIOS Programmer

Gravador/Depurador de EPROM para chips de BIOS da série W25Qxx (Winbond) usando ESP8266, com **detecção automática de capacidade** e suporte de **2MB até 32MB**.

## ✨ Funcionalidades Principais

- 🔍 **Detecção Automática de Chip**: Identifica W25Q16/32/64/128/256 via JEDEC ID
- 📦 **Suporte Multi-Capacidade**: 2MB, 4MB, 8MB, 16MB, 32MB
- 📤 **Upload Streaming Otimizado**: Chunks de 1KB com verify-retry automático
- 🔒 **Gerenciamento de Proteção**: Remove proteções de bloco automaticamente
- ⚡ **Controle de Velocidade SPI**: 0.1-20 MHz ajustável em runtime (presets: 1/4/8/20 MHz)
- ✅ **Verificação Automática**: Checksum byte-a-byte com relatório detalhado de erros
- 🌐 **Interface Web Responsiva**: HTML/JS embutido com terminal em tempo real
- 📊 **Logs Detalhados**: Sistema de verbosidade (Quiet/Normal/Verbose/Debug)
- 🔧 **Decodificação Base64 Robusta**: Correção de bugs críticos de transferência

## 🔧 Hardware Necessário

- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
- Chip W25Qxx Series:
  - ✅ W25Q16 (2MB) - JEDEC ID: EF4015
  - ✅ W25Q32 (4MB) - JEDEC ID: EF4016
  - ✅ W25Q64 (8MB) - JEDEC ID: EF4017 **← Novo suporte!**
  - ✅ W25Q128 (16MB) - JEDEC ID: EF4018
  - ✅ W25Q256 (32MB) - JEDEC ID: EF4019
- Jumpers para conexão (recomendado: cabos curtos 10-15cm)
- Fonte 3.3V estável (ESP8266 já fornece)

## 📌 Pinagem ESP8266 ↔ W25Qxx Series

| ESP8266 | Pino | W25Qxx | Descrição |
|---------|------|----------|-----------|
| D8      | GPIO15 | CS (Pin 1)  | Chip Select |
| D7      | GPIO13 | DI (Pin 5)  | Data Input (MOSI) |
| D6      | GPIO12 | DO (Pin 2)  | Data Output (MISO) |
| D5      | GPIO14 | CLK (Pin 6) | Serial Clock |
| 3.3V    | 3.3V   | VCC (Pin 8) | Alimentação |
| 3.3V    | 3.3V   | **WP# (Pin 3)** | **Write Protect - DEVE estar em VCC!** |
| 3.3V    | 3.3V   | **HOLD# (Pin 7)** | **Hold/Reset - DEVE estar em VCC!** |
| GND     | GND    | GND (Pin 4) | Terra |

**IMPORTANTE:** 
- W25Qxx opera APENAS em 3.3V. NÃO use 5V!
- **⚠️ CRÍTICO:** Pinos 3 (WP#) e 7 (HOLD#) **DEVEM** estar conectados a 3.3V para permitir gravação!
- Se deixar WP# em GND, o Status Register fica bloqueado e não é possível remover proteção de escrita!
- **Use cabos curtos (10-15cm)** para evitar problemas de integridade de sinal

## 🔍 Pinout W25Qxx (SOIC-8)

```
     ┌─── 1 CS (Chip Select)
     │ ┌─ 2 DO (MISO)
     │ │ ┌ 3 WP# (Write Protect) → ⚠️ Conectar a VCC (3.3V)!
     │ │ │┌ 4 GND
  ┌──▼─▼─▼▼─┐
1─┤  ●      ├─8 VCC (3.3V)
2─┤         ├─7 HOLD# → ⚠️ Conectar a VCC (3.3V)!
3─┤  W25Qxx├─6 CLK
4─┤  Series├─5 DI (MOSI)
  └─────────┘
```

**⚠️ ATENÇÃO - CONFIGURAÇÃO CRÍTICA:**
- **Pino 3 (WP#)**: **DEVE** estar conectado a **3.3V** (não deixe flutuante ou em GND!)
- **Pino 7 (HOLD#)**: **DEVE** estar conectado a **3.3V** (não deixe flutuante ou em GND!)
- Se WP# estiver em GND, o Status Register fica protegido e você **NÃO CONSEGUIRÁ** remover proteção de blocos!
- Se HOLD# estiver em GND, a comunicação SPI ficará pausada permanentemente!

### 🔌 Diagrama de Conexão Completo

```
ESP8266                      W25Qxx (SOIC-8)
                        
3.3V ────┬──────────────────► Pin 8 (VCC)
         │
         ├──────────────────► Pin 3 (WP#)   ⚠️ CRÍTICO!
         │
         └──────────────────► Pin 7 (HOLD#) ⚠️ CRÍTICO!

GND ─────────────────────────► Pin 4 (GND)

D8 (GPIO15) ─────────────────► Pin 1 (CS)
D7 (GPIO13) ─────────────────► Pin 5 (DI/MOSI)
D6 (GPIO12) ─────────────────► Pin 2 (DO/MISO)
D5 (GPIO14) ─────────────────► Pin 6 (CLK)
```

**Lista de Verificação antes de gravar:**
- [ ] VCC conectado a 3.3V (Pin 8)
- [ ] GND conectado (Pin 4)
- [ ] **WP# conectado a 3.3V (Pin 3)** ⚠️
- [ ] **HOLD# conectado a 3.3V (Pin 7)** ⚠️
- [ ] CS conectado a D8 (Pin 1)
- [ ] MOSI conectado a D7 (Pin 5)
- [ ] MISO conectado a D6 (Pin 2)
- [ ] CLK conectado a D5 (Pin 6)
- [ ] **Cabos curtos (10-15cm máximo)** 📏

## 🛠️ Configuração Arduino IDE

### 1. Instalar ESP8266 Core
1. File → Preferences
2. Em "Additional Board Manager URLs" adicione:
   ```
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
3. Tools → Board → Boards Manager
4. Procure "ESP8266" e instale

### 2. Configurar Board
- **Board:** "NodeMCU 1.0 (ESP-12E Module)" ou seu modelo específico
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
3. Clique em Upload (→)
4. Aguarde conclusão

## 🌐 Utilizando o Gravador

### 1. Conectar ao ESP8266
1. Abra Serial Monitor (115200 baud)
2. Encontre o IP mostrado na conexão WiFi
3. Acesse `http://SEU_IP_ESP8266` no navegador

### 2. Interface Web
A interface permite:

#### 📋 Informações do Chip
- **Ler JEDEC ID:** 
  - ✅ Detecta automaticamente: W25Q16/32/64/128/256
  - ✅ Mostra capacidade real do chip (2MB-32MB)
  - ✅ Valida comunicação SPI
- **Status:** Mostra se está ocupado ou protegido contra escrita

#### ⚙️ Configuração SPI
- **Frequência Ajustável:** 0.1 MHz - 20 MHz (slider)
- **Presets Rápidos:** Botões 1/4/8/20 MHz para teste rápido
- **Chunk Size Verify:** 4KB, 8KB, 16KB, 32KB, 64KB
- **Recomendação:** 
  - Use **1 MHz** se encontrar corrupção
  - Use **4 MHz** para estabilidade
  - Use **8-20 MHz** para velocidade (cabos curtos)

#### 📖 Leitura
- **Ler Dados:** Lê região específica (endereço + tamanho)
- **Dump Completo:** 
  - ✅ Detecta tamanho do chip automaticamente
  - ✅ Baixa 2MB-32MB conforme chip conectado
  - ✅ Arquivo salvo como `bios_<timestamp>.bin`

#### ✏️ Gravação
- **Aceita Formatos:** `.bin`, `.rom`, `.bss` **← Novo!**
- **Tamanho Máximo:** Até 8MB (ajusta conforme chip detectado)
- **Sistema Chunk-by-Chunk:**
  - Apaga setor (4KB)
  - Escreve 1KB
  - Verifica imediatamente
  - Retry automático (até 3x por chunk)
- **Verificação Automática:** 
  - Checksum byte-a-byte
  - Relatório detalhado de erros
  - Localiza primeiro/último erro
  - Conta regiões afetadas
- **Barra de Progresso:** Atualização em tempo real

#### 🗑️ Apagamento
- **Apagar Setor:** Remove 4KB específicos
- **Apagar Chip:** **⚠️ PERIGO** - Remove tudo (torna placa inutilizável)
- **Proteção Automática:** Remove proteções antes de apagar

#### 🔓 Proteção de Chip
- **Verificar Proteção:** Mostra Status Register e bits BP0-BP2
- **Remover Proteção:** Limpa todos os bits de proteção de bloco
- **Auto-Remove:** Sistema remove proteções automaticamente antes de gravar
- **IMPORTANTE:** Proteção de hardware (pino WP#) deve estar desabilitada (conectado a VCC)

#### 📟 Terminal de Logs
- **Níveis de Verbosidade:** Quiet / Normal / Verbose / Debug
- **Auto-Scroll:** Ativa/desativa rolagem automática
- **Logs em Tempo Real:** Atualização a cada 1 segundo
- **Filtragem por Tipo:** ERROR (vermelho), WARNING (amarelo), INFO (azul), DEBUG (cinza)

## 🔐 Sistema de Proteção do W25Q32BV

### Proteção por Hardware (Pinos)
O W25Q32BV possui dois níveis de proteção:

**1. Pino WP# (Write Protect - Pin 3):**
- **GND (LOW):** Status Register **BLOQUEADO** - não aceita modificações ❌
- **VCC (HIGH):** Status Register **DESBLOQUEADO** - aceita modificações ✅
- **Para gravação:** **OBRIGATÓRIO** conectar a **3.3V**!

**2. Pino HOLD# (Pin 7):**
- **GND (LOW):** Comunicação SPI **PAUSADA** ❌
- **VCC (HIGH):** Comunicação SPI **NORMAL** ✅
- **Para gravação:** **OBRIGATÓRIO** conectar a **3.3V**!

### Proteção por Software (Status Register)
Bits BP0, BP1, BP2 no Status Register protegem regiões de memória:

| BP2 | BP1 | BP0 | Região Protegida | Endereços |
|-----|-----|-----|------------------|-----------|
| 0   | 0   | 0   | Nenhuma (desbloqueado) ✅ | - |
| 0   | 0   | 1   | 256KB superior | 0x3F0000-0x3FFFFF |
| 0   | 1   | 0   | 512KB superior | 0x3E0000-0x3FFFFF |
| 0   | 1   | 1   | 1MB superior | 0x3C0000-0x3FFFFF |
| 1   | 0   | 0   | 2MB superior | 0x380000-0x3FFFFF |
| 1   | 0   | 1   | 3MB superior | 0x300000-0x3FFFFF |
| 1   | 1   | 0   | Metade superior | 0x200000-0x3FFFFF |
| 1   | 1   | 1   | Chip inteiro 🔒 | 0x000000-0x3FFFFF |

**Como usar a proteção:**
1. **Antes de gravar:** Clique em "🔍 Verificar Proteção"
2. Se protegido: Clique em "🔓 Remover Proteção"
3. Certifique-se que **WP# está em VCC (3.3V)**!
4. Se WP# estiver em GND, a remoção de proteção **NÃO FUNCIONARÁ**!

## ⚠️ AVISOS CRÍTICOS

### 🚨 BACKUP PRIMEIRO!
**SEMPRE** faça backup do BIOS original antes de qualquer modificação:
1. Clique em "Dump Completo (4MB)"
2. Salve o arquivo `.bin` em local seguro
3. Guarde múltiplas cópias

### 🔴 Riscos da Gravação
- **BIOS corrompida = Placa mãe inutilizável**
- Apenas grave BIOS compatível com sua placa H61
- Nunca desligue durante gravação
- Verifique integridade do arquivo antes de gravar

### ⚡ Cuidados Elétricos
- **Desligue completamente** a placa mãe (cabo força + bateria)
- Certifique-se que não há energia residual
- Use pulso estática antes de tocar componentes
- Conecte primeiro GND, depois outros pinos

## 🔧 Solução de Problemas

### ❌ Erro: "4001 bytes corrompidos" ou corrupção parcial
**Causa raiz:** Bug crítico na decodificação Base64 (CORRIGIDO na v2.0!)

**Soluções:**
1. **Atualize para versão mais recente** (correção de Base64 implementada)
2. **Reduza velocidade SPI:**
   - Teste 1 MHz primeiro
   - Se OK, aumente gradualmente para 4 MHz → 8 MHz
3. **Melhore conexões físicas:**
   - Use cabos mais curtos (10-15cm)
   - Verifique se há fios soltos
   - Confirme aterramento comum
4. **Verifique logs detalhados:**
   - Console do navegador (F12): tamanho de chunks enviados
   - Serial Monitor: bytes recebidos/decodificados

### 🔍 ID não detectado ou JEDEC incorreto
- Verifique conexões SPI (ordem dos pinos)
- Confirme alimentação estável 3.3V
- Teste continuidade dos jumpers
- Verifique se chip não está soldado invertido
- Use cabos mais curtos (<15cm)

### 🚫 Erro de gravação / "Chip protegido"
- **Chip protegido contra escrita:**
  - Verifique se **WP# (Pin 3)** está conectado a **3.3V** (não GND!)
  - Clique em "🔍 Verificar Proteção" na interface
  - Se protegido, clique em "🔓 Remover Proteção"
  - **Se WP# estiver em GND, a remoção NÃO funcionará!**
- **Chip não responde:**
  - Verifique se **HOLD# (Pin 7)** está conectado a **3.3V** (não GND!)
  - Se HOLD# em GND, chip fica pausado permanentemente
- Tensão insuficiente (use fonte estável 3.3V)
- Conexão instável durante operação

### 📡 ESP8266 não conecta WiFi
- Verifique SSID e senha no código
- Rede 2.4GHz (ESP8266 não suporta 5GHz)
- Se falhar, ESP cria Access Point:
  - Nome: `ESP8266-BIOS-Programmer`
  - Senha: `12345678`
  - IP: `192.168.4.1`

### 💾 Placa mãe não inicia após gravação
1. **NÃO ENTRE EM PÂNICO**
2. Regrave o backup original
3. Se backup corrompido, procure BIOS oficial da fabricante
4. Use ferramenta de recuperação de BIOS da placa (se disponível)

## 📚 Comandos Técnicos W25Q32BV

| Comando | Código | Descrição |
|---------|--------|-----------|
| JEDEC ID | 0x9F | Lê ID: EF4016 |
| Read Data | 0x03 | Leitura sequencial |
| Write Enable | 0x06 | Habilita escrita |
| Page Program | 0x02 | Grava página (256B) |
| Read Status | 0x05 | Status register |
| Sector Erase | 0x20 | Apaga setor (64KB) |
| Chip Erase | 0xC7 | Apaga tudo |

## 🏗️ Arquitetura do Código

```
esp8266_w25q32_programmer.ino
├── 🔧 Configurações
│   ├── WiFi/AP dual mode
│   ├── SPI configurável (0.1-20 MHz)
│   └── Detecção automática de chip
├── 📡 Funções SPI low-level
│   ├── readJedecId() → Lê ID + detecta capacidade
│   ├── detectFlashSize() → Ajusta FLASH_SIZE dinamicamente
│   ├── readData() → Leitura sequencial
│   ├── programPage() → Grava 256 bytes
│   ├── sectorErase() → Apaga 4KB
│   └── chipErase() → Apaga tudo
├── 🔒 Sistema de Proteção
│   ├── readStatus() / readStatus2()
│   ├── writeStatusRegister()
│   ├── disableAllProtection() → Remove BP0-BP2
│   └── isWriteProtected() → Verifica proteção
├── 📤 Sistema de Streaming (arquivos grandes)
│   ├── Chunks de 1KB para economia de memória
│   ├── Write-Verify-Retry por chunk (até 3x)
│   ├── Erase on-demand (apaga setor quando necessário)
│   └── Progress tracking em tempo real
├── 🌐 Servidor Web HTTP
│   ├── handleInterface() → HTML/JS/CSS embutido
│   ├── handleReadId() → /id (com detecção de chip)
│   ├── handleRead() → /read (leitura dinâmica)
│   ├── handleWriteStream() → /writeStream (init/finish)
│   ├── handleWriteChunk() → /writeChunk (Base64 ROBUSTO)
│   ├── handleSetSpiFrequency() → /setSpiFrequency
│   ├── handleCheckProtection() → /checkProtection
│   ├── handleRemoveProtection() → /removeProtection
│   └── handleSystem() → /system (info + tamanho detectado)
├── 📊 Sistema de Logs
│   ├── 4 níveis: Quiet/Normal/Verbose/Debug
│   ├── Buffer circular (50 mensagens)
│   └── Endpoint /logs para web terminal
└── 🖥️ Interface HTML/JS
    ├── Controles de SPI com presets
    ├── Upload streaming com progresso
    ├── Verificação byte-a-byte com relatório
    ├── Terminal em tempo real
    └── Console logs (F12) para debug
```

## 🐛 Bugs Corrigidos (v2.0)

### 🔴 Bug Crítico: Decodificação Base64 (RESOLVIDO)
**Sintoma:** 4001 bytes corrompidos em uploads grandes

**Causa:** 3 bugs na decodificação Base64 manual:
1. ❌ Loop parava antes do fim (`length() - 3`)
2. ❌ Break prematuro no padding
3. ❌ Lógica de extração de bytes incorreta

**Solução:** Decodificador Base64 reescrito do zero:
- ✅ Processa TODOS os caracteres
- ✅ Acumula bits gradualmente (não por blocos)
- ✅ Trata padding corretamente
- ✅ Valida cada byte
- ✅ Logs de debug (tamanho entrada/saída)

**Resultado:** **0% de corrupção** em testes com arquivos de 8MB

## 📈 Changelog

### v2.0 (2025-10-10)
- ✅ **Detecção automática de chips W25Q16/32/64/128/256**
- ✅ **Suporte para 8MB+ (até 32MB)**
- ✅ **Aceitação de arquivos .bss**
- ✅ **Correção crítica de decodificação Base64** (elimina corrupção)
- ✅ **Logs de debug Base64** (cliente + servidor)
- ✅ **Dump dinâmico** (ajusta ao tamanho do chip)
- ✅ **Interface mostra nome e capacidade do chip**
- ✅ **Endpoint /system retorna tamanho detectado**

### v1.0 (Original)
- Suporte W25Q32 (4MB)
- Upload streaming
- Verificação básica
- Controle de SPI

## 📄 Licença

Este projeto é fornecido "como está" para fins educacionais. Use por sua conta e risco. O autor não se responsabiliza por danos causados por uso inadequado.

---

**⚠️ LEMBRETE FINAL:** Gravação de BIOS é operação de alto risco. Sempre tenha backup e conhecimento técnico adequado antes de prosseguir.