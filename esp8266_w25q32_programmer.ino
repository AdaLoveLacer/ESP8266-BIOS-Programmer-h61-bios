/*
 * ESP8266 W25Q32BV BIOS Flash Programmer/Debugger
 * Para H61 Motherboard BIOS chip programming
 * 
 * ⚠️ IMPORTANTE - REGRA DE EDIÇÃO DE ARQUIVOS HTML:
 * =================================================
 * O arquivo index.html é APENAS para visualização durante desenvolvimento.
 * TODO o HTML funcional está DENTRO deste arquivo .ino na função handleInterface().
 * 
 * SEMPRE que editar HTML:
 * 1. PRIMEIRO edite o HTML dentro da função handleInterface() (linhas ~400-650)
 * 2. DEPOIS sincronize as mudanças no arquivo index.html (opcional para preview)
 * 3. NUNCA edite só o index.html e esqueça de atualizar o .ino!
 * 
 * O ESP8266 só executa o que está no .ino, não usa o index.html externo!
 * =================================================
 * 
 * Pinagem sugerida (ESP8266 <-> W25Q32BV):
 * D8 (GPIO15) -> CS  (Chip Select)
 * D7 (GPIO13) -> MOSI (Serial Data Input)
 * D6 (GPIO12) -> MISO (Serial Data Output)  
 * D5 (GPIO14) -> CLK  (Serial Clock)
 * 3.3V        -> VCC
 * GND         -> GND
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>

// Configurações WiFi - ALTERE AQUI
const char* ssid = "Morpheus-Base";
const char* password = "neves@725";

// Configurações Access Point (fallback)
const char* ap_ssid = "ESP8266-BIOS-Programmer";
const char* ap_password = "12345678";  // Mínimo 8 caracteres

// Configurações SPI para W25Q32BV - Conforme Datasheet
#define CS_PIN 15  // D8 - Chip Select
#define SPI_FREQUENCY_DEFAULT 20000000  // 20MHz (ESP8266 estável, chip suporta até 104MHz)
uint32_t currentSpiFrequency = SPI_FREQUENCY_DEFAULT;  // Frequência atual (ajustável em runtime)

// Comandos W25Q32BV - Conforme Datasheet Table 3
#define CMD_JEDEC_ID      0x9F  // Manufacturer/Device ID
#define CMD_READ_DATA     0x03  // Read Data
#define CMD_WRITE_ENABLE  0x06  // Write Enable
#define CMD_PAGE_PROGRAM  0x02  // Page Program
#define CMD_READ_STATUS1  0x05  // Read Status Register-1
#define CMD_SECTOR_ERASE  0x20  // Sector Erase (4KB)
#define CMD_CHIP_ERASE    0xC7  // Chip Erase (ou 0x60)
#define CMD_WRITE_DISABLE 0x04  // Write Disable

// Constantes do chip - Conforme Datasheet
// Suporte estendido: permitir até 8MB (W25Q64 compatível em muitos módulos)
#define FLASH_SIZE_DEFAULT 0x800000  // 8MB default (64Mbit / 8,388,608 bytes)
// Máximo de upload permitido (usar limite um pouco maior que o flash para validação)
#define MAX_FILE_SIZE     0x800000  // 8MB - máximo permitido para upload
#define PAGE_SIZE         256       // 256 bytes por página (Table 1)
#define SECTOR_SIZE       0x1000    // 4KB por setor (4,096 bytes - Table 1)
#define BLOCK_SIZE_32K    0x8000    // 32KB block
#define BLOCK_SIZE_64K    0x10000   // 64KB block
#define CHUNK_SIZE        1024      // 1KB chunks para streaming
#define LED_BUILTIN       2         // LED built-in ESP8266 (invertido)

// Variável global para tamanho do chip detectado (ajustado em runtime via JEDEC)
uint32_t FLASH_SIZE = FLASH_SIZE_DEFAULT;

// Timings do chip - Conforme Datasheet Table 10 (valores máximos)
#define TIMEOUT_PAGE_PROGRAM  5     // 3ms típico, 5ms margem
#define TIMEOUT_SECTOR_ERASE  500   // 400ms máximo + margem
#define TIMEOUT_CHIP_ERASE    250000 // 200s máximo (em ms)

ESP8266WebServer server(80);

// Variáveis para upload streaming
uint32_t uploadAddress = 0;
uint32_t uploadSize = 0;
uint32_t uploadReceived = 0;
bool uploadInProgress = false;

// Buffer para acumular dados CHUNK por CHUNK (1KB) com Write-Verify-Retry
uint8_t* sectorBuffer = NULL;
uint32_t sectorBufferIndex = 0;
uint32_t currentSectorAddress = 0;
bool currentSectorErased = false;  // Flag: setor atual já foi apagado?

// Configurações de sistema
unsigned long ledLastToggle = 0;
bool ledState = false;
unsigned long lastVerboseLog = 0;
const unsigned long VERBOSE_INTERVAL = 100;  // Mínimo 100ms entre logs para evitar overflow

// Sistema de logs para web terminal (apenas erros e avisos)
#define MAX_LOG_MESSAGES 50
String logMessages[MAX_LOG_MESSAGES];
int logIndex = 0;
int logCount = 0;

// Níveis de verbosidade
enum VerbosityLevel {
  VERB_QUIET = 0,    // Apenas ERRORs críticos
  VERB_NORMAL = 1,   // ERRORs + WARNINGs
  VERB_VERBOSE = 2,  // ERRORs + WARNINGs + INFOs
  VERB_DEBUG = 3     // Tudo incluindo mensagens de debug
};
VerbosityLevel currentVerbosity = VERB_VERBOSE;  // Padrão: mostrar tudo

// Configurações de Write-Verify-Retry
uint32_t verifyChunkSize = SECTOR_SIZE;  // Padrão: 4KB (1 setor)

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("===========================================");
  Serial.println("ESP8266 W25Q32BV BIOS Programmer v1.0");
  Serial.println("===========================================");
  
  // Configurar SPI
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // LED off inicialmente (HIGH = off no built-in)
  
  SPI.begin();
  SPI.setFrequency(currentSpiFrequency);
  SPI.setDataMode(SPI_MODE0);
  Serial.printf("✅ SPI configurado: %d MHz\n", currentSpiFrequency / 1000000);
  
  // Tentar conectar ao WiFi
  Serial.println();
  Serial.println("🌐 Tentando conectar ao WiFi...");
  Serial.printf("SSID: %s\n", ssid);
  
  WiFi.begin(ssid, password);
  
  // Aguardar conexão por 15 segundos
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    // Conectado ao roteador
    Serial.println();
    Serial.println("✅ WiFi conectado com sucesso!");
    Serial.println("===========================================");
    Serial.printf("🌐 IP do Servidor: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("🔗 Acesse: http://%s\n", WiFi.localIP().toString().c_str());
    Serial.println("===========================================");
  } else {
    // Falha na conexão - iniciar modo AP
    Serial.println();
    Serial.println("❌ Falha ao conectar WiFi!");
    Serial.println("🔄 Iniciando modo Access Point...");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);
    
    IPAddress AP_IP = WiFi.softAPIP();
    Serial.println();
    Serial.println("✅ Access Point ativo!");
    Serial.println("===========================================");
    Serial.printf("📡 Nome da Rede: %s\n", ap_ssid);
    Serial.printf("🔐 Senha: %s\n", ap_password);
    Serial.printf("🌐 IP do Servidor: %s\n", AP_IP.toString().c_str());
    Serial.printf("🔗 Acesse: http://%s\n", AP_IP.toString().c_str());
    Serial.println("===========================================");
    Serial.println("📱 INSTRUÇÕES:");
    Serial.println("1. Conecte seu celular/PC na rede WiFi acima");
    Serial.println("2. Abra o navegador e acesse o IP mostrado");
    Serial.println("3. Use a interface para programar o BIOS");
    Serial.println("===========================================");
  }
  
  // Configurar rotas HTTP
  server.on("/", HTTP_GET, handleRoot);
  server.on("/interface", HTTP_GET, handleInterface);  // Interface completa
  server.on("/id", HTTP_GET, handleReadId);
  server.on("/read", HTTP_GET, handleRead);
  server.on("/write", HTTP_POST, handleWrite);
  server.on("/writeStream", HTTP_POST, handleWriteStream);
  server.on("/writeChunk", HTTP_POST, handleWriteChunk);
  server.on("/erase", HTTP_POST, handleErase);
  server.on("/eraseSector", HTTP_POST, handleEraseSector);
  server.on("/eraseChip", HTTP_POST, handleEraseChip);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/system", HTTP_GET, handleSystem);
  server.on("/logs", HTTP_GET, handleGetLogs);  // Endpoint para logs
  server.on("/setVerbosity", HTTP_POST, handleSetVerbosity);  // Alterar verbosidade
  server.on("/setSpiFrequency", HTTP_POST, handleSetSpiFrequency);  // Alterar frequência SPI
  server.on("/getSpiFrequency", HTTP_GET, handleGetSpiFrequency);  // Obter frequência atual
  server.on("/setVerifyChunkSize", HTTP_POST, handleSetVerifyChunkSize);  // Alterar tamanho do chunk de verificação
  server.on("/getVerifyChunkSize", HTTP_GET, handleGetVerifyChunkSize);  // Obter tamanho atual do chunk
  server.on("/checkProtection", HTTP_GET, handleCheckProtection);  // Verificar proteção do chip
  server.on("/removeProtection", HTTP_POST, handleRemoveProtection);  // Remover proteção do chip
  
  server.begin();
  Serial.println("✅ Servidor HTTP iniciado");
  
  // Verificar conexão com o chip
  Serial.println();
  Serial.println("🔍 Verificando chip W25Q32BV...");
  uint32_t id = readJedecId();
  
  // Detectar tamanho do chip baseado no JEDEC ID
  detectFlashSize();
  
  if ((id & 0xFFFF00) == 0xEF4000) {  // Manufacturer EF = Winbond
    Serial.println("✅ Chip Winbond detectado com sucesso!");
    logInfo("Chip Winbond detectado (ID: " + String(id, HEX) + ")");
  } else {
    Serial.printf("⚠️ AVISO: ID inesperado: 0x%06X\n", id);
    Serial.println("   Verifique as conexões SPI");
    logWarning("ID inesperado: 0x" + String(id, HEX) + " - Verifique conexões SPI");
  }
  
  Serial.println();
  Serial.println("🚀 Sistema pronto para uso!");
  Serial.println("===========================================");
  logInfo("Sistema iniciado e pronto");
}

// Variáveis para informações periódicas
unsigned long lastInfoPrint = 0;
const unsigned long INFO_INTERVAL = 30000;  // 30 segundos

void loop() {
  server.handleClient();
  
  // Mostrar informações periódicas
  if (millis() - lastInfoPrint > INFO_INTERVAL) {
    lastInfoPrint = millis();
    
    Serial.println();
    Serial.println("📊 STATUS DO SISTEMA:");
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("🌐 WiFi: Conectado (%s)\n", WiFi.localIP().toString().c_str());
    } else if (WiFi.getMode() == WIFI_AP) {
      Serial.printf("📡 AP Mode: %s (%s)\n", ap_ssid, WiFi.softAPIP().toString().c_str());
      Serial.printf("👥 Clientes conectados: %d\n", WiFi.softAPgetStationNum());
    }
    
    Serial.printf("🧠 RAM Livre: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("⏱️ Uptime: %lu segundos\n", millis() / 1000);
    
    if (uploadInProgress) {
      float progress = (float)uploadReceived / uploadSize * 100;
      Serial.printf("📤 Upload: %.1f%% (%d/%d bytes)\n", progress, uploadReceived, uploadSize);
    }
    
    Serial.println("===========================================");
  }
}

// === Funções SPI para W25Q32BV ===

void toggleLED() {
  if (millis() - ledLastToggle > 50) {  // Throttle LED a cada 50ms
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);  // LOW = LED on, HIGH = LED off (built-in invertido)
    ledLastToggle = millis();
  }
}

void verboseLog(String message) {
  if (millis() - lastVerboseLog > VERBOSE_INTERVAL) {  // Rate limit para evitar overflow
    Serial.print("[VERBOSE] ");
    Serial.println(message);
    lastVerboseLog = millis();
  }
}

// Adiciona log ao buffer circular (apenas erros e avisos)
void addWebLog(String type, String message) {
  String timestamp = String(millis() / 1000) + "s";
  String logEntry = "[" + timestamp + "] " + type + ": " + message;
  
  logMessages[logIndex] = logEntry;
  logIndex = (logIndex + 1) % MAX_LOG_MESSAGES;
  if (logCount < MAX_LOG_MESSAGES) logCount++;
  
  // Também envia para Serial
  Serial.println(logEntry);
}

void logError(String message) {
  if (currentVerbosity >= VERB_QUIET) {
    addWebLog("ERROR", message);
  }
}

void logWarning(String message) {
  if (currentVerbosity >= VERB_NORMAL) {
    addWebLog("WARNING", message);
  }
}

void logInfo(String message) {
  if (currentVerbosity >= VERB_VERBOSE) {
    addWebLog("INFO", message);
  }
}

void logDebug(String message) {
  if (currentVerbosity >= VERB_DEBUG) {
    addWebLog("DEBUG", message);
  }
}

uint32_t readJedecId() {
  toggleLED();
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_JEDEC_ID);
  uint32_t id = 0;
  id |= SPI.transfer(0) << 16;  // Manufacturer
  id |= SPI.transfer(0) << 8;   // Memory Type
  id |= SPI.transfer(0);        // Capacity
  digitalWrite(CS_PIN, HIGH);
  toggleLED();
  return id;
}

// Detecta tamanho do chip baseado no JEDEC ID e ajusta FLASH_SIZE
void detectFlashSize() {
  uint32_t id = readJedecId();
  
  // Byte de capacidade (último byte do JEDEC ID)
  uint8_t capacityByte = id & 0xFF;
  
  // Mapear capacity byte para tamanho real
  // Conforme datasheet Winbond: 0x15=2MB, 0x16=4MB, 0x17=8MB, 0x18=16MB, 0x19=32MB
  switch(capacityByte) {
    case 0x15:  // W25Q16 (2MB / 16Mbit)
      FLASH_SIZE = 0x200000;
      Serial.println("✅ Chip detectado: W25Q16 (2MB)");
      logInfo("Chip: W25Q16 - 2MB");
      break;
    case 0x16:  // W25Q32 (4MB / 32Mbit)
      FLASH_SIZE = 0x400000;
      Serial.println("✅ Chip detectado: W25Q32 (4MB)");
      logInfo("Chip: W25Q32 - 4MB");
      break;
    case 0x17:  // W25Q64 (8MB / 64Mbit)
      FLASH_SIZE = 0x800000;
      Serial.println("✅ Chip detectado: W25Q64 (8MB)");
      logInfo("Chip: W25Q64 - 8MB");
      break;
    case 0x18:  // W25Q128 (16MB / 128Mbit)
      FLASH_SIZE = 0x1000000;
      Serial.println("✅ Chip detectado: W25Q128 (16MB)");
      logInfo("Chip: W25Q128 - 16MB");
      break;
    case 0x19:  // W25Q256 (32MB / 256Mbit)
      FLASH_SIZE = 0x2000000;
      Serial.println("✅ Chip detectado: W25Q256 (32MB)");
      logInfo("Chip: W25Q256 - 32MB");
      break;
    default:
      // Se não reconhecer, manter default (8MB) e avisar
      FLASH_SIZE = FLASH_SIZE_DEFAULT;
      Serial.printf("⚠️ Capacity byte desconhecido: 0x%02X - Usando padrão 8MB\n", capacityByte);
      logWarning("Capacity byte desconhecido (0x" + String(capacityByte, HEX) + ") - Usando 8MB default");
      break;
  }
  
  Serial.printf("📦 Tamanho do flash configurado: %d MB (%d bytes)\n", FLASH_SIZE / 1048576, FLASH_SIZE);
}

uint8_t readStatus() {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_READ_STATUS1);
  uint8_t status = SPI.transfer(0);
  digitalWrite(CS_PIN, HIGH);
  return status;
}

// Lê Status Register 2 (contém bits de Quad Enable, etc)
uint8_t readStatus2() {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x35);  // CMD_READ_STATUS2
  uint8_t status = SPI.transfer(0);
  digitalWrite(CS_PIN, HIGH);
  return status;
}

void writeEnable() {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_WRITE_ENABLE);
  digitalWrite(CS_PIN, HIGH);
  delayMicroseconds(1);  // tSHSL timing conforme datasheet
  
  // Aguardar WEL (Write Enable Latch) ser setado
  delay(1);  // 1ms para chip processar comando
  
  // Verificar se WEL foi realmente setado (bit 1 do Status Register)
  uint8_t status = readStatus();
  if (!(status & 0x02)) {  // Bit 1 = WEL
    logWarning("Write Enable Latch não foi setado! Status: 0x" + String(status, HEX));
  }
}

void writeDisable() {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_WRITE_DISABLE);
  digitalWrite(CS_PIN, HIGH);
}

// Aguarda chip ficar pronto - Conforme Datasheet Table 10 timings
void waitForReady(uint32_t timeoutMs = 10000) {
  uint32_t start = millis();
  while (readStatus() & 0x01) {  // Bit 0 = BUSY
    delay(1);
    if (millis() - start > timeoutMs) {
      Serial.println("ERRO: Timeout waiting for chip ready!");
      logError("Timeout aguardando chip (>" + String(timeoutMs) + "ms) - Chip travado ou desconectado");
      return;  // Sair do loop para evitar travamento
    }
    yield();  // Evita watchdog ESP8266
  }
}

// ========== FUNÇÕES DE PROTEÇÃO DE CHIP ==========

// Escreve Status Register (remove TODA proteção)
void writeStatusRegister(uint8_t status1, uint8_t status2) {
  writeEnable();
  
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x01);  // CMD_WRITE_STATUS
  SPI.transfer(status1);
  SPI.transfer(status2);
  digitalWrite(CS_PIN, HIGH);
  
  waitForReady(1000);  // Aguardar conclusão da escrita
}

// Remove TODA proteção do chip (BP0-BP2 = 000)
void disableAllProtection() {
  uint8_t status1 = readStatus();
  uint8_t status2 = readStatus2();
  
  // Limpar bits BP0, BP1, BP2 (bits 2-4) e SRP0 (bit 7)
  status1 &= 0x03;  // Mantém apenas WEL e BUSY, zera proteções
  
  logInfo("🔓 Removendo proteção de escrita (Status antes: 0x" + String(status1, HEX) + ")");
  
  writeStatusRegister(status1, status2);
  
  // Verificar se realmente desprotegeu
  uint8_t newStatus = readStatus();
  if ((newStatus & 0x1C) == 0) {
    logInfo("✅ Chip completamente desprotegido (Status: 0x" + String(newStatus, HEX) + ")");
  } else {
    logError("⚠️ FALHA ao desproteger! Status ainda: 0x" + String(newStatus, HEX));
  }
}

// Verifica bits de proteção - Status Register bits 2,3,4 (BP0,BP1,BP2)
bool isWriteProtected() {
  uint8_t status = readStatus();
  return (status & 0x1C) != 0;  // Bits BP0-BP2 (bits 2-4)
}

// ========== FIM FUNÇÕES DE PROTEÇÃO ==========

void readData(uint32_t address, uint8_t* buffer, uint16_t length) {
  toggleLED();
  verboseLog("Reading data from flash");
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_READ_DATA);
  SPI.transfer((address >> 16) & 0xFF);
  SPI.transfer((address >> 8) & 0xFF);
  SPI.transfer(address & 0xFF);
  
  for (uint16_t i = 0; i < length; i++) {
    buffer[i] = SPI.transfer(0);
  }
  digitalWrite(CS_PIN, HIGH);
  toggleLED();
}

void programPage(uint32_t address, uint8_t* buffer, uint16_t length) {
  toggleLED();
  verboseLog("Programming page");
  
  // Verificar proteção de escrita
  if (isWriteProtected()) {
    logWarning("Chip possui proteção de escrita ativada");
  }
  
  writeEnable();
  // writeEnable() já inclui delay de 1ms e verificação de WEL
  
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_PAGE_PROGRAM);
  SPI.transfer((address >> 16) & 0xFF);
  SPI.transfer((address >> 8) & 0xFF);
  SPI.transfer(address & 0xFF);
  
  for (uint16_t i = 0; i < length; i++) {
    SPI.transfer(buffer[i]);
  }
  digitalWrite(CS_PIN, HIGH);
  
  // Aguardar conclusão com timeout adequado (3ms típico, 5ms máximo)
  waitForReady(TIMEOUT_PAGE_PROGRAM);
  toggleLED();
}

void sectorErase(uint32_t address) {
  writeEnable();
  delay(1);  // Aguardar WEL confirmar antes de apagar
  
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_SECTOR_ERASE);
  SPI.transfer((address >> 16) & 0xFF);
  SPI.transfer((address >> 8) & 0xFF);
  SPI.transfer(address & 0xFF);
  digitalWrite(CS_PIN, HIGH);
  
  // Aguardar conclusão com timeout adequado (60ms típico, 400ms máximo)
  waitForReady(TIMEOUT_SECTOR_ERASE);
}

void chipErase() {
  // 🔓 CRÍTICO: Remover proteção antes de apagar
  disableAllProtection();
  
  writeEnable();
  delay(1);  // Aguardar WEL confirmar antes de apagar
  
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_CHIP_ERASE);
  digitalWrite(CS_PIN, HIGH);
  
  // Aguardar conclusão com timeout adequado (40s típico, 200s máximo)
  waitForReady(TIMEOUT_CHIP_ERASE);
}

// === Sistema de Streaming para Arquivos Grandes ===
// O ESP8266 trabalha com chunks de 1KB para não sobrecarregar a memória
// 
// NOTA: Funcionalidade de compressão RLE foi removida porque:
// - ESP8266 tem apenas 80MHz de CPU e ~80KB de RAM livre
// - Compressão em tempo real é muito pesada para este hardware
// - Sistema de streaming com chunks pequenos é mais estável e confiável
// - Foco na estabilidade ao invés de features computacionalmente intensivas

// === Write-Verify-Retry por CHUNK (1KB) ===
// LÓGICA CORRETA:
// 1. Apaga setor inteiro (4KB) UMA VEZ
// 2. Escreve chunk de 1KB
// 3. Lê de volta e verifica
// 4. Se OK → próximo chunk
// 5. Se falhou → reescreve esse chunk (até 3x)
// 6. Repete até setor completo (4 chunks de 1KB)
bool writeChunkWithRetry(uint32_t address, uint8_t* data, uint32_t dataSize) {
  const uint8_t MAX_RETRIES = 3;
  uint8_t retryCount = 0;
  bool writeSuccess = false;
  
  // Alocar buffer para verificação
  uint8_t* verifyBuffer = (uint8_t*)malloc(dataSize);
  if (!verifyBuffer) {
    logError("Falha ao alocar buffer de verificação (1KB)");
    return false;
  }
  
  while (!writeSuccess && retryCount < MAX_RETRIES) {
    retryCount++;
    
    if (retryCount > 1) {
      logWarning("🔄 Tentativa " + String(retryCount) + "/" + String(MAX_RETRIES) + " - Chunk 0x" + String(address, HEX));
    }
    
    // Gravar dados página por página (1KB = 4 páginas de 256 bytes)
    uint32_t currentAddr = address;
    for (uint32_t i = 0; i < dataSize; i += PAGE_SIZE) {
      uint16_t pageSize = min((uint32_t)PAGE_SIZE, dataSize - i);
      if (currentAddr + pageSize <= FLASH_SIZE) {
        programPage(currentAddr, data + i, pageSize);
        currentAddr += pageSize;
      }
    }
    
    yield();
    
    // Ler de volta para verificar
    readData(address, verifyBuffer, dataSize);
    
    // Comparar byte a byte
    writeSuccess = true;
    uint32_t errorCount = 0;
    for (uint32_t i = 0; i < dataSize; i++) {
      if (data[i] != verifyBuffer[i]) {
        if (writeSuccess) { // Log apenas primeiro erro
          logError("❌ Verificação chunk falhou no offset 0x" + String(i, HEX) + 
                   " (esperado=0x" + String(data[i], HEX) + 
                   ", lido=0x" + String(verifyBuffer[i], HEX) + ")");
        }
        writeSuccess = false;
        errorCount++;
      }
    }
    
    if (!writeSuccess) {
      logWarning("❌ " + String(errorCount) + " bytes incorretos no chunk (total: " + String(dataSize) + ")");
    } else {
      if (retryCount > 1) {
        logInfo("✅ Chunk OK após " + String(retryCount) + " tentativas");
      }
    }
  }
  
  free(verifyBuffer);
  
  if (!writeSuccess) {
    logError("💥 FALHA CRÍTICA após " + String(MAX_RETRIES) + " tentativas no chunk 0x" + String(address, HEX));
  }
  
  return writeSuccess;
}

// === Handlers HTTP ===

void handleRoot() {
  // Página inicial leve que redireciona para interface completa
  String html = "<!DOCTYPE html><html><head><title>ESP8266 BIOS Programmer</title>";
  html += "<meta http-equiv='refresh' content='2;url=/interface'></head><body>";
  html += "<h1>🔧 ESP8266 W25Q32BV BIOS Programmer</h1>";
  html += "<p>✅ Servidor ativo! Redirecionando para interface...</p>";
  html += "<p>Se não redirecionar, <a href='/interface'>clique aqui</a></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleInterface() {
  // Interface completa otimizada para ESP8266
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  
  // HTML Header
  server.sendContent("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
  server.sendContent("<title>ESP8266 W25Q32BV Programmer</title>");
  
  // CSS Minificado
  server.sendContent("<style>");
  server.sendContent("body{font-family:Arial;margin:20px;background:#f0f0f0}");
  server.sendContent(".container{max-width:800px;margin:0 auto;background:white;padding:20px;border-radius:10px}");
  server.sendContent(".section{margin:20px 0;padding:15px;border:1px solid #ccc;border-radius:5px}");
  server.sendContent("button{padding:10px 15px;margin:5px;background:#007cba;color:white;border:none;border-radius:3px;cursor:pointer}");
  server.sendContent("button:hover{background:#005a87}");
  server.sendContent(".result{background:#f9f9f9;padding:10px;margin:10px 0;border-left:4px solid #007cba;font-family:monospace}");
  server.sendContent(".warning{background:#fff3cd;border-left:4px solid #ffc107}");
  server.sendContent(".error{background:#f8d7da;border-left:4px solid #dc3545}");
  server.sendContent(".success{background:#d4edda;border-left:4px solid #28a745}");
  server.sendContent("</style></head><body>");
  
  // HTML Body Principal
  server.sendContent("<div class='container'>");
  server.sendContent("<h1>🔧 ESP8266 W25Q32BV BIOS Programmer</h1>");
  
  // Seção de Informações do Chip
  server.sendContent("<div class='section'><h3>📋 Chip Info</h3>");
  server.sendContent("<button onclick='readChipId()'>JEDEC ID</button>");
  server.sendContent("<button onclick='readStatus()'>Status</button>");
  server.sendContent("<div id='chipInfo' class='result'></div></div>");
  
  // Seção de Configuração SPI
  server.sendContent("<div class='section'><h3>⚙️ SPI Configuration</h3>");
  server.sendContent("<label>Frequência SPI: </label>");
  server.sendContent("<input type='range' id='spiFreqSlider' min='0.1' max='20' step='0.1' value='20' style='width:200px'>");
  server.sendContent("<span id='spiFreqValue' style='margin-left:10px;font-weight:bold'>20.0 MHz</span><br>");
  server.sendContent("<button onclick='applySpiFrequency()' style='background:#28a745;margin-top:10px'>Aplicar Frequência</button>");
  server.sendContent("<div id='spiResult' class='result' style='margin-top:10px'></div>");
  server.sendContent("<hr style='margin:15px 0'>");
  server.sendContent("<label>Tamanho Chunk Verify: </label>");
  server.sendContent("<select id='chunkSizeSelect' style='padding:5px;margin-left:5px'>");
  server.sendContent("<option value='4096'>4 KB (1 setor)</option>");
  server.sendContent("<option value='8192'>8 KB (2 setores)</option>");
  server.sendContent("<option value='16384'>16 KB (4 setores)</option>");
  server.sendContent("<option value='32768'>32 KB (8 setores)</option>");
  server.sendContent("<option value='65536'>64 KB (16 setores)</option>");
  server.sendContent("</select><br>");
  server.sendContent("<button onclick='applyChunkSize()' style='background:#17a2b8;margin-top:10px'>Aplicar Chunk Size</button>");
  server.sendContent("<div id='chunkResult' class='result' style='margin-top:10px'></div></div>");
  
  // Seção de Leitura
  server.sendContent("<div class='section'><h3>📖 Read</h3>");
  server.sendContent("Addr: <input type='text' id='readAddr' value='0x000000' style='width:100px'>");
  server.sendContent("Size: <input type='text' id='readSize' value='256' style='width:80px'><br>");
  server.sendContent("<button onclick='readFlash()'>Read Data</button>");
  server.sendContent("<button onclick='dumpFullBios()'>Full Dump (4MB)</button>");
  server.sendContent("<div id='readResult' class='result'></div></div>");
  
  // Seção de Gravação
  server.sendContent("<div class='section'><h3>✏️ Write</h3>");
  server.sendContent("<input type='file' id='biosFile' accept='.bin,.rom,.bss'><br>");
  server.sendContent("<button onclick='writeBiosStream()'>Write BIOS (Stream)</button>");
  server.sendContent("<div id='writeResult' class='result'></div>");
  server.sendContent("<div id='uploadProgress' style='display:none'>");
  server.sendContent("<div style='background:#ddd;border-radius:3px;overflow:hidden'>");
  server.sendContent("<div id='progressBar' style='background:#007cba;height:20px;width:0%'></div>");
  server.sendContent("</div><span id='progressText'>0%</span></div></div>");
  
  // Seção de Apagamento
  server.sendContent("<div class='section'><h3>🗑️ Erase</h3>");
  server.sendContent("<div class='warning result'>⚠️ CUIDADO: Apagar tornará placa inutilizável!</div>");
  server.sendContent("Setor (hex): <input type='text' id='sectorAddr' value='0x000000' style='width:100px'><br>");
  server.sendContent("<button onclick='eraseSector()' style='background:#ffc107'>Erase Sector (4KB)</button>");
  server.sendContent("<button onclick='eraseChip()' style='background:#dc3545'>Erase Chip Complete</button><br>");
  server.sendContent("<button onclick='checkProtection()' style='background:#17a2b8;margin-top:10px'>🔍 Verificar Proteção</button>");
  server.sendContent("<button onclick='removeProtection()' style='background:#28a745;margin-top:10px'>🔓 Remover Proteção</button>");
  server.sendContent("<div id='eraseResult' class='result'></div></div>");
  
  // Seção de Terminal/Logs
  server.sendContent("<div class='section'><h3>📟 Terminal (Erros e Avisos)</h3>");
  server.sendContent("<button onclick='clearTerminal()'>Limpar</button>");
  server.sendContent("<button onclick='toggleAutoScroll()' id='autoScrollBtn'>Auto-Scroll: ON</button>");
  server.sendContent("<button onclick='decreaseVerbosity()' style='background:#ff6b6b'>🔇 -</button>");
  server.sendContent("<span id='verbosityLevel' style='margin:0 10px;font-weight:bold'>Verbosidade: Verbose</span>");
  server.sendContent("<button onclick='increaseVerbosity()' style='background:#51cf66'>🔊 +</button>");
  server.sendContent("<div id='terminal' style='background:#1e1e1e;color:#00ff00;padding:10px;height:300px;overflow-y:auto;font-family:Consolas,monospace;font-size:12px;border-radius:5px'></div>");
  server.sendContent("</div>");
  
  server.sendContent("</div>");  // Fechar container
  
  yield();  // Dar tempo ao sistema
  
  // JavaScript Minificado
  server.sendContent("<script>");
  
  // Variáveis globais para checksum
  server.sendContent("let originalFileBuffer=null;");
  
  // Controle de frequência SPI
  server.sendContent("const spiSlider=document.getElementById('spiFreqSlider');");
  server.sendContent("const spiValue=document.getElementById('spiFreqValue');");
  server.sendContent("spiSlider.addEventListener('input',function(){");
  server.sendContent("const val=parseFloat(this.value);");
  server.sendContent("spiValue.textContent=(val>=1?val.toFixed(1)+' MHz':(val*1000).toFixed(0)+' kHz');");
  server.sendContent("});");
  server.sendContent("function applySpiFrequency(){");
  server.sendContent("const freq=parseFloat(document.getElementById('spiFreqSlider').value);");
  server.sendContent("fetch('/setSpiFrequency',{method:'POST',body:JSON.stringify({frequency:freq})})");
  server.sendContent(".then(r=>r.json()).then(d=>{");
  server.sendContent("if(d.success){");
  server.sendContent("const f=d.frequency>=1?d.frequency.toFixed(1)+' MHz':(d.frequency*1000).toFixed(0)+' kHz';");
  server.sendContent("document.getElementById('spiResult').innerHTML='<div class=\"success\">✅ Frequência alterada para '+f+'</div>';");
  server.sendContent("}else{");
  server.sendContent("document.getElementById('spiResult').innerHTML='<div class=\"error\">❌ '+d.error+'</div>';");
  server.sendContent("}}).catch(e=>document.getElementById('spiResult').innerHTML='<div class=\"error\">❌ '+e.message+'</div>');}");
  server.sendContent("function loadSpiFrequency(){");
  server.sendContent("fetch('/getSpiFrequency').then(r=>r.json()).then(d=>{");
  server.sendContent("const freqMHz=d.frequency/1000000;");
  server.sendContent("document.getElementById('spiFreqSlider').value=freqMHz.toFixed(1);");
  server.sendContent("const display=freqMHz>=1?freqMHz.toFixed(1)+' MHz':(freqMHz*1000).toFixed(0)+' kHz';");
  server.sendContent("document.getElementById('spiFreqValue').textContent=display;");
  server.sendContent("}).catch(e=>console.error('Erro ao carregar frequência:',e));}");
  server.sendContent("function applyChunkSize(){");
  server.sendContent("const size=parseInt(document.getElementById('chunkSizeSelect').value);");
  server.sendContent("fetch('/setVerifyChunkSize',{method:'POST',body:JSON.stringify({size:size})})");
  server.sendContent(".then(r=>r.json()).then(d=>{");
  server.sendContent("if(d.success){");
  server.sendContent("document.getElementById('chunkResult').innerHTML='<div class=\"success\">✅ Chunk size: '+d.sizeKB+' KB</div>';");
  server.sendContent("}else{");
  server.sendContent("document.getElementById('chunkResult').innerHTML='<div class=\"error\">❌ '+d.error+'</div>';");
  server.sendContent("}}).catch(e=>document.getElementById('chunkResult').innerHTML='<div class=\"error\">❌ '+e.message+'</div>');}");
  server.sendContent("function loadChunkSize(){");
  server.sendContent("fetch('/getVerifyChunkSize').then(r=>r.json()).then(d=>{");
  server.sendContent("document.getElementById('chunkSizeSelect').value=d.size;");
  server.sendContent("}).catch(e=>console.error('Erro ao carregar chunk size:',e));}");
  
  // Função readChipId
  server.sendContent("function readChipId(){");
  server.sendContent("fetch('/id').then(r=>r.json()).then(d=>{");
  server.sendContent("const chip=d.chipName||'Unknown';");
  server.sendContent("const sizeMB=d.sizeMB||'?';");
  server.sendContent("const isWinbond=chip.startsWith('W25Q');");
  server.sendContent("const info=isWinbond?'<div class=\"success\">✅ '+chip+' ('+d.id+') - '+sizeMB+' MB</div>':'<div class=\"error\">❌ ID: '+d.id+'</div>';");
  server.sendContent("document.getElementById('chipInfo').innerHTML=info;");
  server.sendContent("}).catch(e=>document.getElementById('chipInfo').innerHTML='<div class=\"error\">'+e+'</div>');}");
  
  // Função readStatus
  server.sendContent("function readStatus(){");
  server.sendContent("fetch('/status').then(r=>r.json()).then(d=>{");
  server.sendContent("const status=d.busy?'🔄 Ocupado':'✅ Pronto';");
  server.sendContent("const wp=d.writeProtected?'🔒 Protegido':'🔓 Gravável';");
  server.sendContent("document.getElementById('chipInfo').innerHTML='<div>'+status+'<br>'+wp+'</div>';");
  server.sendContent("}).catch(e=>document.getElementById('chipInfo').innerHTML='<div class=\"error\">'+e+'</div>');}");
  
  yield();
  
  // Função de dump streaming otimizada
  server.sendContent("async function dumpFullBios(){");
  server.sendContent("document.getElementById('readResult').innerHTML='🔄 Dump...';");
  server.sendContent("document.getElementById('uploadProgress').style.display='block';");
  server.sendContent("try{");
  server.sendContent("const totalSize=4194304,chunkSize=4096,totalChunks=1024;");
  server.sendContent("const biosData=new Uint8Array(totalSize);let offset=0;");
  server.sendContent("for(let i=0;i<totalChunks;i++){");
  server.sendContent("const hexAddr='0x'+offset.toString(16).padStart(6,'0');");
  server.sendContent("const r=await fetch('/read?addr='+hexAddr+'&size='+chunkSize);");
  server.sendContent("if(!r.ok)throw new Error('Chunk '+(i+1)+' failed');");
  server.sendContent("biosData.set(new Uint8Array(await r.arrayBuffer()),offset);");
  server.sendContent("offset+=chunkSize;");
  server.sendContent("const p=((i+1)/totalChunks)*100;");
  server.sendContent("document.getElementById('progressBar').style.width=p+'%';");
  server.sendContent("document.getElementById('progressText').textContent=Math.round(p)+'%';");
  server.sendContent("if(i%10===0)await new Promise(r=>setTimeout(r,10));}");
  server.sendContent("const blob=new Blob([biosData]);");
  server.sendContent("const url=URL.createObjectURL(blob);");
  server.sendContent("const a=document.createElement('a');a.href=url;");
  server.sendContent("a.download='bios_'+Date.now()+'.bin';a.click();URL.revokeObjectURL(url);");
  server.sendContent("document.getElementById('readResult').innerHTML='<div class=\"success\">✅ OK!</div>';");
  server.sendContent("}catch(e){document.getElementById('readResult').innerHTML='<div class=\"error\">❌ '+e.message+'</div>';");
  server.sendContent("}finally{document.getElementById('uploadProgress').style.display='none';}}");
  
  yield();
  
  // Função readFlash simples
  server.sendContent("function readFlash(){");
  server.sendContent("const addr=document.getElementById('readAddr').value;");
  server.sendContent("const size=document.getElementById('readSize').value;");
  server.sendContent("fetch('/read?addr='+addr+'&size='+size).then(r=>r.text()).then(d=>{");
  server.sendContent("document.getElementById('readResult').innerHTML=d.substring(0,500)+'...';");
  server.sendContent("}).catch(e=>document.getElementById('readResult').innerHTML='<div class=\"error\">'+e+'</div>');}");
  
  // Função de gravação streaming com checksum
  server.sendContent("async function writeBiosStream(){");
  server.sendContent("const file=document.getElementById('biosFile').files[0];");
  server.sendContent("if(!file){alert('Selecione arquivo');return;}");
  server.sendContent("if(file.size>5242880){alert('Arquivo muito grande (max 5MB)');return;}");
  server.sendContent("document.getElementById('writeResult').innerHTML='🔄 Preparando...';");
  server.sendContent("document.getElementById('uploadProgress').style.display='block';");
  server.sendContent("const reader=new FileReader();");
  server.sendContent("originalFileBuffer=await new Promise(r=>{reader.onload=e=>r(new Uint8Array(e.target.result));reader.readAsArrayBuffer(file);});");
  server.sendContent("try{");
  server.sendContent("if(file.size > " + String(FLASH_SIZE) + ") { alert('Arquivo muito grande para o chip (max " + String(FLASH_SIZE) + " bytes)'); return; }");
  server.sendContent("await fetch('/writeStream',{method:'POST',body:JSON.stringify({action:'init',size:file.size})});");
  server.sendContent("const chunkSize=1024;let offset=0;");
  server.sendContent("while(offset<file.size){");
  server.sendContent("const chunk=file.slice(offset,offset+chunkSize);");
  server.sendContent("const reader2=new FileReader();");
  server.sendContent("const data=await new Promise(r=>{reader2.onload=e=>r(e.target.result.split(',')[1]);reader2.readAsDataURL(chunk);});");
  server.sendContent("console.log('Chunk @'+offset+': '+chunk.size+' bytes -> base64 '+data.length+' chars');");
  server.sendContent("await fetch('/writeChunk',{method:'POST',body:JSON.stringify({data:data})});");
  server.sendContent("offset+=chunkSize;");
  server.sendContent("const p=(offset/file.size)*100;");
  server.sendContent("document.getElementById('progressBar').style.width=p+'%';");
  server.sendContent("document.getElementById('progressText').textContent=Math.round(p)+'%';}");
  server.sendContent("await fetch('/writeStream',{method:'POST',body:JSON.stringify({action:'finish'})});");
  server.sendContent("document.getElementById('writeResult').innerHTML='<div class=\"success\">✅ Gravado! Verificando...</div>';");
  server.sendContent("await verifyChecksum();");
  server.sendContent("}catch(e){document.getElementById('writeResult').innerHTML='<div class=\"error\">❌ '+e.message+'</div>';");
  server.sendContent("}finally{document.getElementById('uploadProgress').style.display='none';}}");;
  
  yield();
  
  // Função de verificação de checksum com logs detalhados
  server.sendContent("async function verifyChecksum(){");
  server.sendContent("if(!originalFileBuffer){alert('Arquivo original não disponível');return;}");
  server.sendContent("document.getElementById('writeResult').innerHTML='🔄 Verificando integridade...';");
  server.sendContent("document.getElementById('uploadProgress').style.display='block';");
  server.sendContent("try{");
  server.sendContent("const fileSize=originalFileBuffer.length;");
  server.sendContent("const chunkSize=4096;let offset=0,errors=0;");
  server.sendContent("let firstError=null,lastError=null,errorRanges=[];");
  server.sendContent("while(offset<fileSize){");
  server.sendContent("const currentChunk=Math.min(chunkSize,fileSize-offset);");
  server.sendContent("const hexAddr='0x'+offset.toString(16).padStart(6,'0');");
  server.sendContent("const r=await fetch('/read?addr='+hexAddr+'&size='+currentChunk);");
  server.sendContent("const chipData=new Uint8Array(await r.arrayBuffer());");
  server.sendContent("const fileData=originalFileBuffer.slice(offset,offset+currentChunk);");
  server.sendContent("let chunkErrors=0;");
  server.sendContent("for(let i=0;i<chipData.length;i++){");
  server.sendContent("if(chipData[i]!==fileData[i]){");
  server.sendContent("errors++;chunkErrors++;");
  server.sendContent("const errorAddr=offset+i;");
  server.sendContent("if(!firstError)firstError={addr:errorAddr,expected:fileData[i],got:chipData[i]};");
  server.sendContent("lastError={addr:errorAddr,expected:fileData[i],got:chipData[i]};");
  server.sendContent("}}");
  server.sendContent("if(chunkErrors>0)errorRanges.push({start:offset,count:chunkErrors});");
  server.sendContent("offset+=currentChunk;");
  server.sendContent("const p=(offset/fileSize)*100;");
  server.sendContent("document.getElementById('progressBar').style.width=p+'%';");
  server.sendContent("document.getElementById('progressText').textContent=Math.round(p)+'%';");
  server.sendContent("if(offset%40960===0)await new Promise(r=>setTimeout(r,10));}");
  server.sendContent("if(errors===0){");
  server.sendContent("document.getElementById('writeResult').innerHTML='<div class=\"success\">✅ Verificação OK! BIOS íntegra ('+fileSize+' bytes)</div>';");
  server.sendContent("}else{");
  server.sendContent("let errorMsg='<div class=\"error\">❌ ERRO: '+errors+' bytes corrompidos!<br>';");
  server.sendContent("errorMsg+='📍 Primeiro erro: 0x'+firstError.addr.toString(16).toUpperCase()+' (esperado=0x'+firstError.expected.toString(16)+', lido=0x'+firstError.got.toString(16)+')<br>';");
  server.sendContent("errorMsg+='📍 Último erro: 0x'+lastError.addr.toString(16).toUpperCase()+'<br>';");
  server.sendContent("errorMsg+='📦 Regiões afetadas: '+errorRanges.length+' chunks<br>';");
  server.sendContent("errorMsg+='💡 Dica: Reduza velocidade SPI ou troque cabos</div>';");
  server.sendContent("document.getElementById('writeResult').innerHTML=errorMsg;");
  server.sendContent("}");
  server.sendContent("}catch(e){document.getElementById('writeResult').innerHTML='<div class=\"error\">❌ Erro na verificação: '+e.message+'</div>';");
  server.sendContent("}finally{document.getElementById('uploadProgress').style.display='none';}}");  
  yield();
  
  // Função de apagar setor
  server.sendContent("function eraseSector(){");
  server.sendContent("const addr=document.getElementById('sectorAddr').value;");
  server.sendContent("if(!confirm('Apagar setor em '+addr+'?'))return;");
  server.sendContent("fetch('/eraseSector',{method:'POST',body:JSON.stringify({address:addr})}).then(r=>r.json()).then(d=>{");
  server.sendContent("document.getElementById('eraseResult').innerHTML=d.success?'<div class=\"success\">✅ Setor apagado</div>':'<div class=\"error\">❌ '+d.error+'</div>';");
  server.sendContent("}).catch(e=>document.getElementById('eraseResult').innerHTML='<div class=\"error\">'+e+'</div>');}");
  
  // Função de apagar chip completo  
  server.sendContent("function eraseChip(){");
  server.sendContent("if(!confirm('APAGAR COMPLETAMENTE O CHIP?\\n\\nIsto tornará a placa inutilizável!'))return;");
  server.sendContent("if(!confirm('Tem certeza absoluta? Esta ação é IRREVERSÍVEL!'))return;");
  server.sendContent("fetch('/eraseChip',{method:'POST'}).then(r=>r.json()).then(d=>{");
  server.sendContent("document.getElementById('eraseResult').innerHTML=d.success?'<div class=\"success\">✅ Chip apagado</div>':'<div class=\"error\">❌ '+d.error+'</div>';");
  server.sendContent("}).catch(e=>document.getElementById('eraseResult').innerHTML='<div class=\"error\">'+e+'</div>');}");
  
  // Funções de proteção
  server.sendContent("function checkProtection(){");
  server.sendContent("fetch('/checkProtection').then(r=>r.json()).then(d=>{");
  server.sendContent("let msg='<div class=\"'+(d.protected?'error':'success')+'\">';");
  server.sendContent("msg+='Status Register: 0x'+d.status.toString(16).toUpperCase()+'<br>';");
  server.sendContent("msg+=d.protected?'🔒 CHIP PROTEGIDO! BP bits: '+d.bpBits:'🔓 Chip desprotegido';");
  server.sendContent("msg+='</div>';");
  server.sendContent("document.getElementById('eraseResult').innerHTML=msg;");
  server.sendContent("}).catch(e=>document.getElementById('eraseResult').innerHTML='<div class=\"error\">'+e+'</div>');}");
  
  server.sendContent("function removeProtection(){");
  server.sendContent("if(!confirm('Remover toda proteção do chip?'))return;");
  server.sendContent("fetch('/removeProtection',{method:'POST'}).then(r=>r.json()).then(d=>{");
  server.sendContent("document.getElementById('eraseResult').innerHTML=d.success?'<div class=\"success\">✅ Proteção removida</div>':'<div class=\"error\">❌ '+d.error+'</div>';");
  server.sendContent("}).catch(e=>document.getElementById('eraseResult').innerHTML='<div class=\"error\">'+e+'</div>');}");
  
  yield();
  
  // Funções do Terminal
  server.sendContent("let autoScroll=true;");
  server.sendContent("let verbosity=2;");  // 0=Quiet, 1=Normal, 2=Verbose, 3=Debug
  server.sendContent("const verbLevels=['Quiet','Normal','Verbose','Debug'];");
  server.sendContent("function clearTerminal(){document.getElementById('terminal').innerHTML='';}");
  server.sendContent("function toggleAutoScroll(){autoScroll=!autoScroll;document.getElementById('autoScrollBtn').textContent='Auto-Scroll: '+(autoScroll?'ON':'OFF');}");
  server.sendContent("function increaseVerbosity(){verbosity=Math.min(3,verbosity+1);updateVerbosityDisplay();fetch('/setVerbosity',{method:'POST',body:JSON.stringify({level:verbosity})});}");
  server.sendContent("function decreaseVerbosity(){verbosity=Math.max(0,verbosity-1);updateVerbosityDisplay();fetch('/setVerbosity',{method:'POST',body:JSON.stringify({level:verbosity})});}");
  server.sendContent("function updateVerbosityDisplay(){document.getElementById('verbosityLevel').textContent='Verbosidade: '+verbLevels[verbosity];}");
  server.sendContent("function addTerminalLine(line){");
  server.sendContent("const term=document.getElementById('terminal');");
  server.sendContent("let color='#00ff00';");
  server.sendContent("if(line.includes('ERROR'))color='#ff4444';");
  server.sendContent("else if(line.includes('WARNING'))color='#ffaa00';");
  server.sendContent("else if(line.includes('INFO'))color='#00aaff';");
  server.sendContent("else if(line.includes('DEBUG'))color='#999999';");
  server.sendContent("term.innerHTML+='<div style=\"color:'+color+'\">'+line+'</div>';");
  server.sendContent("if(autoScroll)term.scrollTop=term.scrollHeight;}");
  server.sendContent("function updateTerminal(){");
  server.sendContent("fetch('/logs').then(r=>r.json()).then(d=>{");
  server.sendContent("const term=document.getElementById('terminal');");
  server.sendContent("term.innerHTML='';");
  server.sendContent("d.logs.forEach(log=>addTerminalLine(log));");
  server.sendContent("}).catch(e=>console.error('Log fetch error:',e));}");
  server.sendContent("setInterval(updateTerminal,1000);");
  
  // Auto-load
  server.sendContent("window.onload=()=>{readChipId();updateTerminal();loadSpiFrequency();loadChunkSize();};");
  server.sendContent("</script></body></html>");
  
  server.sendContent("");  // Finalizar stream
}

void handleReadId() {
  uint32_t id = readJedecId();
  
  // Verificar se conseguiu ler o ID
  if (id == 0x000000 || id == 0xFFFFFF) {
    server.send(500, "application/json", "{\"error\":\"Falha na comunicação SPI - Verifique conexões\"}");
    return;
  }
  
  // Identificar chip específico
  String chipName = "UNKNOWN";
  if ((id & 0xFFFF00) == 0xEF4000) {  // Winbond
    uint8_t capacityByte = id & 0xFF;
    switch(capacityByte) {
      case 0x15: chipName = "W25Q16"; break;
      case 0x16: chipName = "W25Q32"; break;
      case 0x17: chipName = "W25Q64"; break;
      case 0x18: chipName = "W25Q128"; break;
      case 0x19: chipName = "W25Q256"; break;
    }
  }
  
  String response = "{\"id\":\"" + String(id, HEX) + "\",\"chipName\":\"" + chipName + "\",\"sizeMB\":" + String(FLASH_SIZE / 1048576) + "}";
  response.toUpperCase();
  server.send(200, "application/json", response);
}

void handleRead() {
  String addrStr = server.arg("addr");
  String sizeStr = server.arg("size");
  
  // Validar parâmetros
  if (addrStr.length() == 0 || sizeStr.length() == 0) {
    server.send(400, "text/plain", "Parâmetros addr e size são obrigatórios");
    return;
  }
  
  uint32_t address = strtoul(addrStr.c_str(), NULL, 0);
  uint16_t size = sizeStr.toInt();
  
  if (size == 0) {
    server.send(400, "text/plain", "Tamanho deve ser maior que zero");
    return;
  }
  
  // Limitar chunks para não sobrecarregar ESP8266
  if (size > 8192) size = 8192;  // Máximo 8KB por request
  if (address + size > FLASH_SIZE) {
    server.send(400, "text/plain", "Endereço fora do range da memória flash");
    return;
  }
  
  uint8_t* buffer = (uint8_t*)malloc(size);
  if (!buffer) {
    server.send(500, "text/plain", "Erro de memória - RAM insuficiente");
    return;
  }
  
  // Verificar se chip está acessível antes da leitura
  uint32_t testId = readJedecId();
  if (testId == 0x000000 || testId == 0xFFFFFF) {
    free(buffer);
    server.send(500, "text/plain", "Erro na comunicação SPI - Chip não responde");
    return;
  }
  
  readData(address, buffer, size);
  
  // Se tamanho grande, enviar como binário
  if (size > 512) {
    server.send(200, "application/octet-stream", (char*)buffer, size);
  } else {
    // Pequeno, enviar como hex
    String hex = "";
    for (uint16_t i = 0; i < size; i++) {
      if (i % 16 == 0) hex += "\n";
      hex += String(buffer[i], HEX);
      if (buffer[i] < 16) hex = hex.substring(0, hex.length()-1) + "0" + hex.substring(hex.length()-1);
      hex += " ";
    }
    server.send(200, "text/plain", hex);
  }
  
  free(buffer);
}

void handleWrite() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Dados não encontrados\"}");
    return;
  }
  
  String data = server.arg("plain");
  uint32_t fileSize = data.length();
  
  if (fileSize == 0) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Arquivo vazio\"}");
    return;
  }
  
  if (fileSize > FLASH_SIZE) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Arquivo muito grande\"}");
    return;
  }
  
  // Verificar se chip está acessível
  uint32_t testId = readJedecId();
  if (testId == 0x000000 || testId == 0xFFFFFF) {
    server.send(500, "application/json", "{\"success\":false,\"error\":\"Chip não responde - Verifique conexões SPI\"}");
    return;
  }
  
  unsigned long startTime = millis();
  
  // Apagar setores necessários primeiro
  uint32_t sectorsNeeded = (fileSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
  for (uint32_t sector = 0; sector < sectorsNeeded; sector++) {
    uint32_t sectorAddr = sector * SECTOR_SIZE;
    sectorErase(sectorAddr);
    
    // Verificar se setor foi apagado corretamente
    uint8_t testByte;
    readData(sectorAddr, &testByte, 1);
    if (testByte != 0xFF) {
      server.send(500, "application/json", "{\"success\":false,\"error\":\"Falha ao apagar setor " + String(sector) + "\"}");
      return;
    }
    
    // Dar um tempo para não sobrecarregar
    yield();
  }
  
  // Programar por páginas
  uint32_t address = 0;
  for (uint32_t i = 0; i < fileSize; i += PAGE_SIZE) {
    uint16_t pageSize = min((uint32_t)PAGE_SIZE, fileSize - i);
    uint8_t* pageData = (uint8_t*)data.c_str() + i;
    
    programPage(address, pageData, pageSize);
    
    // Verificar se página foi gravada corretamente
    uint8_t verify[PAGE_SIZE];
    readData(address, verify, pageSize);
    if (memcmp(pageData, verify, pageSize) != 0) {
      server.send(500, "application/json", "{\"success\":false,\"error\":\"Falha na verificação da página em 0x" + String(address, HEX) + "\"}");
      return;
    }
    
    address += pageSize;
    yield();  // Dar tempo ao watchdog
  }
  
  unsigned long totalTime = (millis() - startTime) / 1000;
  
  String response = "{\"success\":true,\"time\":" + String(totalTime) + "}";
  server.send(200, "application/json", response);
}

void handleErase() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Parâmetros inválidos\"}");
    return;
  }
  
  String body = server.arg("plain");
  
  if (body.indexOf("\"type\":\"chip\"") >= 0) {
    chipErase();
    server.send(200, "application/json", "{\"success\":true}");
  } else if (body.indexOf("\"type\":\"sector\"") >= 0) {
    // Extrair endereço (implementação simplificada)
    uint32_t address = 0;
    if (body.indexOf("0x") >= 0) {
      String addrStr = body.substring(body.indexOf("0x"), body.indexOf("0x") + 8);
      address = strtoul(addrStr.c_str(), NULL, 0);
    }
    
    if (address < FLASH_SIZE) {
      sectorErase(address);
      server.send(200, "application/json", "{\"success\":true}");
    } else {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Endereço inválido\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Tipo de apagamento inválido\"}");
  }
}

void handleStatus() {
  uint8_t status = readStatus();
  bool busy = status & 0x01;  // Bit 0 = BUSY
  bool writeProtected = (status & 0x1C) != 0;  // Bits 2-4 = BP0, BP1, BP2
  
  String response = "{\"busy\":" + String(busy ? "true" : "false") + 
                   ",\"writeProtected\":" + String(writeProtected ? "true" : "false") +
                   ",\"statusReg\":\"0x" + String(status, HEX) + "\"" +
                   "}";
  server.send(200, "application/json", response);
}

// === Handlers para Streaming ===

void handleWriteStream() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Parâmetros inválidos\"}");
    logError("WriteStream: Parâmetros inválidos");
    return;
  }
  
  String body = server.arg("plain");
  
  if (body.indexOf("\"action\":\"init\"") >= 0) {
    // Verificar se chip está acessível antes de iniciar
    uint32_t testId = readJedecId();
    if (testId == 0x000000 || testId == 0xFFFFFF) {
      server.send(500, "application/json", "{\"success\":false,\"error\":\"Chip não responde - Verifique conexões SPI\"}");
      logError("Chip não responde - Verifique conexões SPI");
      return;
    }
    
    // Inicializar upload streaming
    uploadAddress = 0;
    uploadReceived = 0;
    uploadInProgress = true;
    
    // Alocar buffer para chunk (1KB fixo)
    if (sectorBuffer != NULL) {
      free(sectorBuffer);
    }
    sectorBuffer = (uint8_t*)malloc(1024);  // Buffer de 1KB
    if (!sectorBuffer) {
      uploadInProgress = false;
      server.send(500, "application/json", "{\"success\":false,\"error\":\"Falha ao alocar buffer de 1KB\"}");
      logError("Falha ao alocar buffer de 1KB");
      return;
    }
    sectorBufferIndex = 0;
    currentSectorAddress = 0;
    currentSectorErased = false;  // Marca que o primeiro setor ainda não foi apagado
    
    // 🔓 CRÍTICO: Remover TODA proteção antes de gravar
    disableAllProtection();
    
    // Extrair e validar tamanho do arquivo
    int sizePos = body.indexOf("\"size\":");
    if (sizePos >= 0) {
      String sizeStr = body.substring(sizePos + 7);
      sizeStr = sizeStr.substring(0, sizeStr.indexOf(","));
      uploadSize = sizeStr.toInt();
    }
    
    if (uploadSize == 0) {
      uploadInProgress = false;
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Tamanho de arquivo inválido\"}");
      logError("Tamanho de arquivo inválido");
      return;
    }
    
    if (uploadSize > FLASH_SIZE) {  // Máximo 4MB
      uploadInProgress = false;
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Arquivo maior que 4MB\"}");
      logError("Arquivo maior que 4MB: " + String(uploadSize) + " bytes");
      return;
    }
    
    uint32_t sectorsNeeded = (uploadSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
    logInfo("🚀 Iniciando gravação: " + String(uploadSize) + " bytes (" + String(sectorsNeeded) + " setores)");
    logInfo("⚡ Modo CHUNK-BY-CHUNK: apaga setor → escreve 1KB → verifica → próximo 1KB");
    
    server.send(200, "application/json", "{\"success\":true}");
    
  } else if (body.indexOf("\"action\":\"finish\"") >= 0) {
    // Gravar último chunk parcial se houver
    if (sectorBufferIndex > 0) {
      logInfo("📦 Gravando último chunk parcial: " + String(sectorBufferIndex) + " bytes no endereço 0x" + String(currentSectorAddress, HEX));
      
      // Se ainda não apagou o setor atual, apagar agora
      if (!currentSectorErased) {
        uint32_t sectorAddr = (currentSectorAddress / SECTOR_SIZE) * SECTOR_SIZE;
        logInfo("🗑️ Apagando setor 0x" + String(sectorAddr, HEX));
        sectorErase(sectorAddr);
        currentSectorErased = true;
      }
      
      bool success = writeChunkWithRetry(currentSectorAddress, sectorBuffer, sectorBufferIndex);
      if (!success) {
        free(sectorBuffer);
        sectorBuffer = NULL;
        uploadInProgress = false;
        server.send(500, "application/json", "{\"success\":false,\"error\":\"Falha ao gravar último chunk parcial\"}");
        return;
      }
      logInfo("✅ Último chunk parcial gravado com sucesso");
    }
    
    // CRÍTICO: Aguardar chip finalizar operações internas
    logInfo("⏳ Aguardando chip estabilizar...");
    delay(100);  // 100ms para flash finalizar writes internos e caches
    waitForReady(1000);  // Garantir que chip não está mais ocupado
    
    // Liberar buffer
    if (sectorBuffer != NULL) {
      free(sectorBuffer);
      sectorBuffer = NULL;
    }
    
    // Finalizar upload
    uploadInProgress = false;
    
    logInfo("✅ Gravação concluída: " + String(uploadReceived) + " bytes");
    logInfo("💾 Dados estabilizados na memória flash");
    verboseLog("Upload finalizado: " + String(uploadReceived) + "/" + String(uploadSize) + " bytes");
    
    String response = "{\"success\":true,\"time\":0,\"received\":" + String(uploadReceived) + "}";
    server.send(200, "application/json", response);
    
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Ação inválida\"}");
    logError("WriteStream: Ação inválida");
  }
}

void handleWriteChunk() {
  if (!uploadInProgress) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Stream não iniciado\"}");
    return;
  }
  
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Dados inválidos\"}");
    return;
  }
  
  String body = server.arg("plain");
  
  // Extrair dados base64
  int dataPos = body.indexOf("\"data\":\"");
  if (dataPos < 0) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Dados não encontrados\"}");
    return;
  }
  
  // Verificar se ainda há espaço na memória
  if (ESP.getFreeHeap() < 2048) {  // Menos de 2KB livre
    uploadInProgress = false;
    server.send(500, "application/json", "{\"success\":false,\"error\":\"Memória insuficiente no ESP8266\"}");
    return;
  }
  
  String base64Data = body.substring(dataPos + 8);
  base64Data = base64Data.substring(0, base64Data.indexOf("\""));
  
  // Log do tamanho base64 recebido (debug)
  logDebug("📥 Base64 recebido: " + String(base64Data.length()) + " chars");
  
  // Dados em base64 - decodificar usando algoritmo ROBUSTO
  // Alocar buffer com tamanho exato (base64: 4 chars -> 3 bytes, ajustado para padding)
  uint16_t estimatedSize = (base64Data.length() * 3) / 4 + 4;  // +4 margem de segurança
  uint8_t* chunkData = (uint8_t*)malloc(estimatedSize);
  
  if (!chunkData) {
    uploadInProgress = false;
    server.send(500, "application/json", "{\"success\":false,\"error\":\"Falha ao alocar memória\"}");
    return;
  }
  
  // === DECODIFICADOR BASE64 ROBUSTO ===
  // Tabela de lookup para decodificação rápida
  const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  uint16_t chunkSize = 0;
  uint32_t buffer = 0;
  int bitsCollected = 0;
  
  for (int i = 0; i < base64Data.length(); i++) {
    char c = base64Data[i];
    
    // Ignorar caracteres inválidos (whitespace, newlines, etc)
    if (c == '=' || c == '\r' || c == '\n' || c == ' ') {
      if (c == '=') break;  // Padding = fim dos dados
      continue;
    }
    
    // Encontrar valor do caractere base64
    int value = -1;
    if (c >= 'A' && c <= 'Z') value = c - 'A';
    else if (c >= 'a' && c <= 'z') value = c - 'a' + 26;
    else if (c >= '0' && c <= '9') value = c - '0' + 52;
    else if (c == '+') value = 62;
    else if (c == '/') value = 63;
    
    if (value < 0) continue;  // Caractere inválido, pular
    
    // Acumular bits
    buffer = (buffer << 6) | value;
    bitsCollected += 6;
    
    // Quando temos 8 bits (1 byte completo), extrair
    if (bitsCollected >= 8) {
      bitsCollected -= 8;
      chunkData[chunkSize++] = (buffer >> bitsCollected) & 0xFF;
    }
  }
  
  // Log do tamanho decodificado (debug)
  logDebug("📦 Bytes decodificados: " + String(chunkSize) + 
           " (primeiro: 0x" + String(chunkData[0], HEX) + 
           ", último: 0x" + String(chunkData[chunkSize-1], HEX) + ")");
  
  // === NOVA LÓGICA CHUNK-BY-CHUNK (1KB) ===
  // 1. Acumula dados até completar 1KB
  // 2. Se mudou de setor → apaga setor inteiro ANTES
  // 3. Escreve 1KB com writeChunkWithRetry (que verifica automaticamente)
  // 4. Próximo 1KB
  
  for (uint16_t i = 0; i < chunkSize; i++) {
    sectorBuffer[sectorBufferIndex++] = chunkData[i];
    
    // Se completou 1KB, gravar com verify-retry
    if (sectorBufferIndex >= 1024) {
      // Verificar se mudou de setor (a cada 4KB)
      uint32_t currentSector = (currentSectorAddress / SECTOR_SIZE) * SECTOR_SIZE;
      uint32_t nextChunkSector = ((currentSectorAddress + 1024) / SECTOR_SIZE) * SECTOR_SIZE;
      
      // Se vai escrever em um setor novo, precisa apagar primeiro
      if (!currentSectorErased) {
        logInfo("�️ Apagando setor 0x" + String(currentSector, HEX));
        sectorErase(currentSector);
        currentSectorErased = true;
      }
      
      // Se o próximo chunk vai cair em outro setor, marcar para apagar depois
      if (currentSector != nextChunkSector) {
        currentSectorErased = false;  // Próximo chunk está em outro setor
      }
      
      logDebug("📝 Gravando chunk 1KB no endereço 0x" + String(currentSectorAddress, HEX));
      
      bool success = writeChunkWithRetry(currentSectorAddress, sectorBuffer, 1024);
      
      if (!success) {
        free(chunkData);
        free(sectorBuffer);
        sectorBuffer = NULL;
        uploadInProgress = false;
        
        server.send(500, "application/json", "{\"success\":false,\"error\":\"Falha ao gravar chunk 0x" + String(currentSectorAddress, HEX) + " após 3 tentativas\"}");
        return;
      }
      
      // Resetar buffer para próximo chunk
      sectorBufferIndex = 0;
      currentSectorAddress += 1024;
      
      // Checkpoint a cada 256KB
      if (currentSectorAddress % 262144 == 0 && currentSectorAddress > 0) {
        logInfo("🔍 Checkpoint: " + String(currentSectorAddress / 1024) + " KB gravados");
      }
    }
  }
  
  uploadReceived += chunkSize;
  
  // Log de progresso
  if (uploadReceived % 10240 == 0 || uploadReceived >= uploadSize) {
    float progress = (float)uploadReceived / uploadSize * 100;
    logInfo("📊 Progresso: " + String(uploadReceived) + "/" + String(uploadSize) + " bytes (" + String((int)progress) + "%)");
  }
  
  free(chunkData);
  
  server.send(200, "application/json", "{\"success\":true}");
}

void handleSystem() {
  uint32_t jedecId = readJedecId();
  uint8_t status = readStatus();
  
  String response = "{";
  response += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  response += "\"chipId\":\"" + String(ESP.getChipId(), HEX) + "\",";
  response += "\"flashSize\":" + String(ESP.getFlashChipSize()) + ",";
  response += "\"cpuFreq\":" + String(ESP.getCpuFreqMHz()) + ",";
  response += "\"uptime\":" + String(millis()) + ",";
  response += "\"jedecId\":\"" + String(jedecId, HEX) + "\",";
  response += "\"detectedFlashSize\":" + String(FLASH_SIZE) + ",";  // Tamanho detectado do chip externo
  response += "\"detectedFlashSizeMB\":" + String(FLASH_SIZE / 1048576) + ",";
  response += "\"flashStatus\":\"";
  
  // Verificar família Winbond (EF40xx)
  if ((jedecId & 0xFFFF00) == 0xEF4000) {
    uint8_t capacityByte = jedecId & 0xFF;
    switch(capacityByte) {
      case 0x15: response += "W25Q16_2MB"; break;
      case 0x16: response += "W25Q32_4MB"; break;
      case 0x17: response += "W25Q64_8MB"; break;
      case 0x18: response += "W25Q128_16MB"; break;
      case 0x19: response += "W25Q256_32MB"; break;
      default: response += "WINBOND_UNKNOWN"; break;
    }
  } else if (jedecId == 0x000000 || jedecId == 0xFFFFFF) {
    response += "NO_COMMUNICATION";
  } else {
    response += "UNKNOWN_ID";
  }
  
  if (status & 0x01) response += "_BUSY";
  if (status & 0x02) response += "_WEL";
  if (status & 0x1C) response += "_PROTECTED";
  
  response += "\"";
  response += "}";
  server.send(200, "application/json", response);
}

// Handler para alterar nível de verbosidade
void handleSetVerbosity() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Body vazio\"}");
    return;
  }
  
  String body = server.arg("plain");
  int levelPos = body.indexOf("\"level\":");
  if (levelPos < 0) {
    server.send(400, "application/json", "{\"error\":\"Parametro level não encontrado\"}");
    return;
  }
  
  // Extrair número do nível
  String levelStr = body.substring(levelPos + 8);
  levelStr = levelStr.substring(0, levelStr.indexOf("}"));
  int level = levelStr.toInt();
  
  if (level < 0 || level > 3) {
    server.send(400, "application/json", "{\"error\":\"Level deve ser 0-3\"}");
    return;
  }
  
  currentVerbosity = (VerbosityLevel)level;
  
  String levelName[] = {"Quiet", "Normal", "Verbose", "Debug"};
  logInfo("Verbosidade alterada para: " + levelName[level]);
  
  server.send(200, "application/json", "{\"success\":true,\"level\":" + String(level) + "}");
}

// Handler para alterar frequência SPI
void handleSetSpiFrequency() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Body vazio\"}");
    return;
  }
  
  String body = server.arg("plain");
  int freqPos = body.indexOf("\"frequency\":");
  if (freqPos < 0) {
    server.send(400, "application/json", "{\"error\":\"Parametro frequency não encontrado\"}");
    return;
  }
  
  // Extrair valor da frequência em MHz (pode ser decimal)
  String freqStr = body.substring(freqPos + 12);
  freqStr = freqStr.substring(0, freqStr.indexOf("}"));
  float freqMHz = freqStr.toFloat();
  
  // Validar range (0.1 MHz = 100kHz até 20MHz)
  if (freqMHz < 0.1 || freqMHz > 20.0) {
    server.send(400, "application/json", "{\"error\":\"Frequência deve ser entre 0.1-20 MHz (100kHz-20MHz)\"}");
    return;
  }
  
  // Converter para Hz e aplicar
  currentSpiFrequency = (uint32_t)(freqMHz * 1000000);
  SPI.setFrequency(currentSpiFrequency);
  
  String freqDisplay = freqMHz >= 1.0 ? String(freqMHz, 1) + " MHz" : String((int)(freqMHz * 1000)) + " kHz";
  logInfo("Frequência SPI alterada para: " + freqDisplay);
  Serial.printf("⚙️ SPI frequência ajustada: %s\n", freqDisplay.c_str());
  // Sugestão de diagnóstico quando ocorrer corrupção de gravação
  logInfo("Dica: Se você ainda observar corrupção (ex: bytes corrompidos), reduza a velocidade SPI ou troque os cabos/fios de conexão.");
  
  server.send(200, "application/json", "{\"success\":true,\"frequency\":" + String(freqMHz, 1) + "}");
}

// Handler para obter frequência SPI atual
void handleGetSpiFrequency() {
  String response = "{\"frequency\":" + String(currentSpiFrequency) + ",\"frequencyMHz\":" + String(currentSpiFrequency / 1000000) + "}";
  server.send(200, "application/json", response);
}

// Handler para alterar tamanho do chunk de verificação
void handleSetVerifyChunkSize() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Body vazio\"}");
    return;
  }
  
  String body = server.arg("plain");
  int sizePos = body.indexOf("\"size\":");
  if (sizePos < 0) {
    server.send(400, "application/json", "{\"error\":\"Parametro size não encontrado\"}");
    return;
  }
  
  // Extrair valor do tamanho
  String sizeStr = body.substring(sizePos + 7);
  sizeStr = sizeStr.substring(0, sizeStr.indexOf("}"));
  uint32_t size = sizeStr.toInt();
  
  // Validar tamanhos permitidos (múltiplos de 4KB)
  if (size < SECTOR_SIZE || size > BLOCK_SIZE_64K || (size % SECTOR_SIZE) != 0) {
    server.send(400, "application/json", "{\"error\":\"Size deve ser múltiplo de 4KB entre 4KB-64KB\"}");
    return;
  }
  
  verifyChunkSize = size;
  
  logInfo("Chunk size alterado para: " + String(size / 1024) + " KB");
  Serial.printf("⚙️ Verify chunk size ajustado: %d KB\n", size / 1024);
  
  server.send(200, "application/json", "{\"success\":true,\"sizeKB\":" + String(size / 1024) + "}");
}

// Handler para obter tamanho do chunk de verificação atual
void handleGetVerifyChunkSize() {
  String response = "{\"size\":" + String(verifyChunkSize) + ",\"sizeKB\":" + String(verifyChunkSize / 1024) + "}";
  server.send(200, "application/json", response);
}

// Handler para retornar logs (apenas erros e avisos)
void handleGetLogs() {
  String response = "{\"logs\":[";
  
  int start = (logCount < MAX_LOG_MESSAGES) ? 0 : logIndex;
  int count = (logCount < MAX_LOG_MESSAGES) ? logCount : MAX_LOG_MESSAGES;
  
  for (int i = 0; i < count; i++) {
    int idx = (start + i) % MAX_LOG_MESSAGES;
    if (i > 0) response += ",";
    response += "\"" + logMessages[idx] + "\"";
  }
  
  response += "]}";
  server.send(200, "application/json", response);
}

// Handler para apagar setor específico
void handleEraseSector() {
  String body = server.arg("plain");
  int addrStart = body.indexOf("\"address\":\"") + 11;
  int addrEnd = body.indexOf("\"", addrStart);
  String addrStr = body.substring(addrStart, addrEnd);
  
  uint32_t address = 0;
  if (addrStr.startsWith("0x")) {
    address = strtoul(addrStr.c_str(), NULL, 16);
  } else {
    address = addrStr.toInt();
  }
  
  // Validar endereço do setor (deve ser múltiplo de 4KB)
  if (address % SECTOR_SIZE != 0) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Endereço deve ser múltiplo de 4KB\"}");
    return;
  }
  
  if (address >= FLASH_SIZE) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Endereço fora do chip\"}");
    return;
  }
  
  verboseLog("Apagando setor em 0x" + String(address, HEX));
  
  digitalWrite(LED_BUILTIN, LOW); // LED aceso durante operação
  sectorErase(address);
  digitalWrite(LED_BUILTIN, HIGH); // LED apagado
  
  // Verificar se apagou corretamente
  uint8_t testByte;
  readData(address, &testByte, 1);
  
  if (testByte == 0xFF) {
    verboseLog("Setor apagado com sucesso");
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    verboseLog("Falha ao apagar setor");
    server.send(500, "application/json", "{\"success\":false,\"error\":\"Falha na verificação\"}");
  }
}

// Handler para apagar chip completo
void handleEraseChip() {
  logWarning("Iniciando apagamento COMPLETO do chip");
  verboseLog("Iniciando apagamento completo do chip");
  
  digitalWrite(LED_BUILTIN, LOW); // LED aceso durante operação
  chipErase();
  digitalWrite(LED_BUILTIN, HIGH); // LED apagado
  
  verboseLog("Verificando setores apagados");
  
  // Verificar alguns setores para confirmar
  bool success = true;
  for (uint32_t addr = 0; addr < FLASH_SIZE; addr += SECTOR_SIZE) {
    uint8_t testByte;
    readData(addr, &testByte, 1);
    if (testByte != 0xFF) {
      success = false;
      break;
    }
    
    // Verificar apenas alguns setores para não demorar muito
    if (addr > SECTOR_SIZE * 10) break;
    yield();
  }
  
  if (success) {
    logInfo("Chip apagado completamente");
    verboseLog("Chip apagado com sucesso");
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    logError("Falha ao apagar chip - Verificação falhou");
    verboseLog("Falha ao apagar chip");
    server.send(500, "application/json", "{\"success\":false,\"error\":\"Falha na verificação\"}");
  }
}

// Handler para verificar proteção
void handleCheckProtection() {
  uint8_t status = readStatus();
  bool isProtected = (status & 0x1C) != 0;  // BP0-BP2 bits
  uint8_t bpBits = (status >> 2) & 0x07;  // Extrair bits 2-4
  
  String response = "{\"success\":true,\"status\":" + String(status) + 
                   ",\"protected\":" + String(isProtected ? "true" : "false") +
                   ",\"bpBits\":" + String(bpBits) + "}";
  
  logInfo("Status Register: 0x" + String(status, HEX) + (isProtected ? " 🔒 PROTEGIDO" : " 🔓 Desprotegido"));
  server.send(200, "application/json", response);
}

// Handler para remover proteção
void handleRemoveProtection() {
  logInfo("Removendo proteção do chip...");
  disableAllProtection();
  
  // Verificar se funcionou
  uint8_t newStatus = readStatus();
  bool stillProtected = (newStatus & 0x1C) != 0;
  
  if (!stillProtected) {
    logInfo("✅ Proteção removida com sucesso");
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    logError("❌ Falha ao remover proteção! Status: 0x" + String(newStatus, HEX));
    server.send(500, "application/json", "{\"success\":false,\"error\":\"Chip ainda protegido\"}");
  }
}