# LED Activity Indicators - ESP8266 W25Q32BV Programmer

## Visão Geral
O sistema agora inclui indicações visuais LED para todas as operações SPI, proporcionando feedback em tempo real sobre a atividade do programador.

## LED Utilizado
- **GPIO2** (LED interno do ESP8266 NodeMCU/Wemos D1 Mini)
- Lógica invertida: LOW = LED aceso, HIGH = LED apagado

## Padrões de LED por Operação

### Operações SPI Básicas

#### 1. **readJedecId()** - Verificação de ID do Chip
- **Padrão**: LED aceso durante operação
- **Duração**: ~100ms
- **Indica**: Comunicação SPI ativa

#### 2. **readStatus()** - Leitura de Status
- **Padrão**: LED aceso durante operação  
- **Duração**: ~50ms
- **Indica**: Verificação de status do chip

#### 3. **readData()** - Leitura de Dados
- **Padrão**: 
  - Chunks pequenos (≤1KB): LED aceso contínuo
  - Chunks grandes (>1KB): LED piscando (100ms on/off)
- **Duração**: Proporcional ao tamanho dos dados
- **Indica**: Transferência de dados do chip para ESP

#### 4. **programPage()** - Gravação de Página
- **Padrão**: LED aceso durante gravação
- **Duração**: ~5ms por página (256 bytes)
- **Indica**: Escrita ativa na memória flash

#### 5. **sectorErase()** - Apagamento de Setor
- **Padrão**: 
  1. 2 piscadas rápidas (50ms)
  2. LED aceso durante apagamento
- **Duração**: ~1-3 segundos por setor
- **Indica**: Operação de apagamento em andamento

#### 6. **chipErase()** - Apagamento Completo
- **Padrão**:
  1. 5 piscadas rápidas (50ms)
  2. LED aceso durante apagamento completo
- **Duração**: ~10-60 segundos
- **Indica**: Apagamento completo do chip

### Operações de Streaming

#### 1. **Início de Upload** (`handleWriteStream` - init)
- **Padrão**: 3 piscadas longas (100ms)
- **Indica**: Inicialização do streaming de upload

#### 2. **Recebimento de Chunk** (`handleWriteChunk`)
- **Padrão**: 
  1. 1 piscada rápida (10ms) - recebimento
  2. LED aceso durante gravação do chunk
- **Indica**: Processamento de dados recebidos

#### 3. **Finalização de Upload** (`handleWriteStream` - finish)
- **Padrão**: 5 piscadas rápidas (50ms)
- **Indica**: Upload concluído com sucesso

#### 4. **Dump/Leitura** (`handleRead`)
- **Padrão**:
  - Chunks pequenos (≤1KB): 1 piscada curta (10ms)
  - Chunks grandes (>1KB): 2 piscadas rápidas (50ms)
  - LED aceso durante leitura
- **Indica**: Operação de dump em andamento

## Funções de Controle LED

### `ledActivityOn()`
- Liga o LED (GPIO2 = LOW)
- Usado durante operações longas

### `ledActivityOff()`
- Desliga o LED (GPIO2 = HIGH)
- Usado para finalizar indicações

### `ledActivityBlink(int count, int delayMs)`
- Pisca o LED um número específico de vezes
- `count`: número de piscadas
- `delayMs`: duração de cada estado (on/off)

## Benefícios

1. **Feedback Visual**: O usuário pode ver quando o ESP está ativo
2. **Diagnóstico**: Diferentes padrões ajudam a identificar operações
3. **Depuração**: Facilita identificar travamentos ou problemas
4. **Experiência**: Interface mais profissional e responsiva

## Uso Prático

- **LED piscando**: Operação em andamento
- **LED aceso constante**: Transferência de dados ativa
- **LED apagado**: Sistema em espera
- **Padrões específicos**: Identificam tipos de operação

O sistema LED fornece feedback imediato sobre todas as operações do programador, melhorando significativamente a experiência do usuário e facilitando o diagnóstico de problemas.