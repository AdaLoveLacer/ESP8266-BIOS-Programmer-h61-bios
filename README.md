# ESP8266 W25Q32BV BIOS Programmer

Gravador/Depurador de EPROM para chip de BIOS W25Q32BV usando ESP8266, especificamente desenvolvido para placas mãe H61.

## 🔧 Hardware Necessário

- ESP8266 (NodeMCU, Wemos D1 Mini, etc.)
- Chip W25Q32BV (32Mbit = 4MB SPI Flash)
- Jumpers para conexão
- Fonte 3.3V (ESP8266 já fornece)

## 📌 Pinagem ESP8266 ↔ W25Q32BV

| ESP8266 | Pino | W25Q32BV | Descrição |
|---------|------|----------|-----------|
| D8      | GPIO15 | CS (Pin 1)  | Chip Select |
| D7      | GPIO13 | DI (Pin 5)  | Data Input (MOSI) |
| D6      | GPIO12 | DO (Pin 2)  | Data Output (MISO) |
| D5      | GPIO14 | CLK (Pin 6) | Serial Clock |
| 3.3V    | 3.3V   | VCC (Pin 8) | Alimentação |
| GND     | GND    | GND (Pin 4) | Terra |

**IMPORTANTE:** W25Q32BV opera APENAS em 3.3V. NÃO use 5V!

## 🔍 Pinout W25Q32BV (SOIC-8)

```
     ┌─── 1 CS
     │ ┌─ 2 DO (MISO)
     │ │ 
  ┌──▼─▼──┐
1─┤  ●    ├─8 VCC (3.3V)
2─┤       ├─7 HOLD
3─┤ W25Q  ├─6 CLK
4─┤ 32BV  ├─5 DI (MOSI)
  └───────┘
```

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
- **Ler JEDEC ID:** Verifica se o W25Q32BV está conectado corretamente
- **Status:** Mostra se está ocupado ou protegido contra escrita

#### 📖 Leitura
- **Ler Dados:** Lê região específica (endereço + tamanho)
- **Dump Completo:** Faz backup de todos os 4MB do BIOS

#### ✏️ Gravação
- **Gravar BIOS (Básico):** Para arquivos pequenos (até 1MB)
- **Gravar BIOS (Streaming):** Para arquivos grandes (até 5MB) usando chunks de 1KB
- **Sistema Otimizado:** Focado na estabilidade do ESP8266
- **Verificar:** Compara arquivo gravado com original
- **Barra de Progresso:** Mostra progresso em tempo real

#### 🗑️ Apagamento
- **Apagar Setor:** Remove 64KB específicos
- **Apagar Chip:** **⚠️ PERIGO** - Remove tudo (torna placa inutilizável)

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

### BIOS maior que 1MB
- **Use Streaming:** Ative "Usar Streaming" na interface (recomendado)
- **Suporte até 5MB:** Sistema aceita arquivos maiores que o chip
- **Chunks de 1KB:** Arquivo é enviado em pequenos pedaços para não sobrecarregar o ESP8266
- **Otimizado para Hardware:** Sem compressão para evitar sobrecarga do processador

### ID não detectado (diferente de EF4016)
- Verifique conexões SPI
- Confirme alimentação 3.3V
- Teste continuidade dos fios
- Verifique se chip não está soldado invertido

### Erro de gravação
- Chip pode estar protegido contra escrita
- Tensão insuficiente
- Conexão instável durante operação

### ESP8266 não conecta WiFi
- Verifique SSID e senha
- Rede 2.4GHz (ESP8266 não suporta 5GHz)
- Reinicie ESP8266 após alteração

### Placa mãe não inicia após gravação
1. **NÃO ENTRE EM PÂNICO**
2. Regrave o backup original
3. Se backup corrompido, procure BIOS oficial da fabricante
4. Use ferramenta de recuperação de BIOS da placa

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
├── Configurações WiFi/SPI
├── Funções SPI low-level
│   ├── readJedecId()
│   ├── readData()
│   ├── programPage()
│   └── sectorErase()
├── Sistema de Streaming (para arquivos grandes)
│   ├── Chunks de 1KB para economia de memória
│   └── Progress tracking em tempo real
├── Servidor Web HTTP
│   ├── handleRoot() → Interface HTML
│   ├── handleReadId() → /id
│   ├── handleRead() → /read
│   ├── handleWrite() → /write (tradicional)
│   ├── handleWriteStream() → /writeStream (iniciar/finalizar)
│   ├── handleWriteChunk() → /writeChunk (chunks)
│   └── handleErase() → /erase
└── Interface HTML embutida com streaming
```

## 📄 Licença

Este projeto é fornecido "como está" para fins educacionais. Use por sua conta e risco. O autor não se responsabiliza por danos causados por uso inadequado.

---

**⚠️ LEMBRETE FINAL:** Gravação de BIOS é operação de alto risco. Sempre tenha backup e conhecimento técnico adequado antes de prosseguir.