# Especificações do W25Q32BV - Winbond Serial Flash Memory

## Resumo do Chip
- **Capacidade**: 32Mbit (4MB / 4,194,304 bytes)
- **Organização**: 16,384 páginas programáveis de 256 bytes cada
- **Interface**: SPI Serial (Single, Dual, Quad)
- **Tensão**: 2.7V - 3.6V
- **Temperatura**: -40°C a +85°C (Industrial)

## Frequências SPI Suportadas
- **Standard SPI**: até 104MHz
- **Dual SPI**: até 80MHz  
- **Quad SPI**: até 80MHz

**Implementação atual**: 20MHz (balanceamento entre velocidade e estabilidade do ESP8266)

## Estrutura de Memória
- **Page Size**: 256 bytes (programação mínima)
- **Sector Size**: 4KB (4,096 bytes) - apagamento mínimo
- **Block Size 32KB**: 32,768 bytes
- **Block Size 64KB**: 65,536 bytes
- **Total**: 4,194,304 bytes (0x000000 a 0x3FFFFF)

## Comandos Principais (SPI Mode 0,3)

| Comando | Hex | Descrição | Ciclos |
|---------|-----|-----------|--------|
| JEDEC ID | 0x9F | Lê Manufacturer/Device ID | 4 bytes |
| Read Data | 0x03 | Lê dados sequencialmente | 3 addr + n data |
| Write Enable | 0x06 | Habilita escrita | 1 byte |
| Write Disable | 0x04 | Desabilita escrita | 1 byte |
| Page Program | 0x02 | Grava até 256 bytes | 3 addr + 1-256 data |
| Read Status-1 | 0x05 | Lê Status Register-1 | 2 bytes |
| Sector Erase (4KB) | 0x20 | Apaga setor 4KB | 3 addr |
| Block Erase 32KB | 0x52 | Apaga bloco 32KB | 3 addr |
| Block Erase 64KB | 0xD8 | Apaga bloco 64KB | 3 addr |
| Chip Erase | 0xC7/0x60 | Apaga chip completo | 1 byte |

## Timings Críticos (conforme Table 10 do datasheet)

### Page Program (256 bytes)
- **Típico**: 0.7ms
- **Máximo**: 3ms
- **Implementação**: timeout 5ms (margem de segurança)

### Sector Erase (4KB)
- **Típico**: 60ms
- **Máximo**: 400ms
- **Implementação**: timeout 500ms

### Block Erase 32KB
- **Típico**: 120ms
- **Máximo**: 1,600ms (1.6s)

### Block Erase 64KB
- **Típico**: 150ms
- **Máximo**: 2,000ms (2s)

### Chip Erase (Full 4MB)
- **Típico**: 40s
- **Máximo**: 200s
- **Implementação**: timeout 250s (250,000ms)

## Status Register-1 (SR1) - Endereço 0x05

| Bit | Nome | Descrição | R/W |
|-----|------|-----------|-----|
| 7 | SRP0 | Status Register Protect 0 | R/W |
| 6 | SEC | Sector/Block Protect | R/W |
| 5 | TB | Top/Bottom Protect | R/W |
| 4 | BP2 | Block Protect Bit 2 | R/W |
| 3 | BP1 | Block Protect Bit 1 | R/W |
| 2 | BP0 | Block Protect Bit 0 | R/W |
| 1 | WEL | Write Enable Latch | R |
| 0 | BUSY | Erase/Write In Progress | R |

### Interpretação dos Bits de Proteção
- **BUSY (bit 0)**: `1` = operação em andamento, `0` = chip pronto
- **WEL (bit 1)**: `1` = escrita habilitada (após Write Enable cmd)
- **BP0-BP2 (bits 2-4)**: Controlam proteção de blocos
  - `000` = Nenhuma proteção
  - `001` a `111` = Proteção progressiva de setores

## Sequência de Operações

### Leitura
1. CS = LOW
2. Enviar comando 0x03
3. Enviar endereço de 24 bits (A23-A0)
4. Receber dados sequencialmente
5. CS = HIGH

### Escrita (Page Program)
1. Enviar comando Write Enable (0x06)
2. CS = LOW
3. Enviar comando 0x02
4. Enviar endereço de 24 bits
5. Enviar 1 a 256 bytes de dados
6. CS = HIGH
7. Aguardar BUSY = 0 (polling Status Register)

### Apagamento de Setor
1. Enviar comando Write Enable (0x06)
2. CS = LOW
3. Enviar comando 0x20 (Sector Erase)
4. Enviar endereço de 24 bits (múltiplo de 4KB)
5. CS = HIGH
6. Aguardar BUSY = 0 (até 400ms máximo)

### Apagamento Completo
1. Enviar comando Write Enable (0x06)
2. CS = LOW
3. Enviar comando 0xC7 (Chip Erase)
4. CS = HIGH
5. Aguardar BUSY = 0 (até 200s máximo)

## Importantes Considerações

### 1. Proteção de Escrita
- Sempre verificar bits BP0-BP2 antes de escrever
- Chip pode estar protegido contra escrita parcial/total
- Use comando Write Status Register para desproteger se necessário

### 2. Page Boundary
- Page Program NUNCA pode cruzar limite de 256 bytes
- Se endereço inicial = 0x0080 e escrever 200 bytes:
  - Bytes 0-127 vão para página atual (0x0080-0x00FF)
  - Bytes 128-199 **sobrescrevem** o início da página (wrap-around)
  
### 3. Erase Before Write
- Flash memory pode apenas mudar bits de `1` para `0`
- Para mudar `0` para `1` é necessário apagar primeiro
- Células apagadas = 0xFF (todos bits em 1)

### 4. Endurance
- **Program/Erase Cycles**: 100,000 típico
- **Data Retention**: 20 anos mínimo

### 5. Power-Down
- Comando 0xB9 para low power mode
- Consumo < 1µA em deep power-down

## Checklist de Implementação

✅ **Frequência SPI**: 20MHz configurada (chip suporta até 104MHz)  
✅ **SPI Mode**: Mode 0 (CPOL=0, CPHA=0)  
✅ **Timeouts**: Page 5ms, Sector 500ms, Chip 250s  
✅ **Status Register**: Polling bit BUSY (bit 0)  
✅ **Proteção**: Verificação bits BP0-BP2 (bits 2-4)  
✅ **Write Enable**: Comando 0x06 antes de programar/apagar  
✅ **Page Boundary**: Controlado por PAGE_SIZE = 256  
✅ **Sector Alignment**: SECTOR_SIZE = 4KB (0x1000)  
✅ **JEDEC ID**: Verificação 0xEF4016 (Winbond W25Q32BV)  

## Referências
- **Datasheet**: W25Q32BVSSIG-Winbond.pdf
- **Manufacturer**: Winbond Electronics
- **Part Number**: W25Q32BVSSIG
- **Revision**: Rev. J (Março 2011)
