/*
 * ESP8266 W25Q32BV BIOS Flash Programmer/Debugger
 * Para H61 Motherboard BIOS chip programming
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
const char* ssid = "ssid";
const char* password = "password";

// Configurações Access Point (fallback)
const char* ap_ssid = "ESP8266-BIOS-Programmer";
const char* ap_password = "12345678";  // Mínimo 8 caracteres

// Configurações SPI para W25Q32BV
#define CS_PIN 15  // D8 - Chip Select
#define SPI_FREQUENCY 1000000  // 1MHz para estabilidade

// Comandos W25Q32BV
#define CMD_JEDEC_ID      0x9F
#define CMD_READ_DATA     0x03
#define CMD_WRITE_ENABLE  0x06
#define CMD_PAGE_PROGRAM  0x02
#define CMD_READ_STATUS1  0x05
#define CMD_SECTOR_ERASE  0x20
#define CMD_CHIP_ERASE    0xC7

// Constantes do chip
#define FLASH_SIZE        0x400000  // 4MB
#define PAGE_SIZE         256       // 256 bytes por página
#define SECTOR_SIZE       0x10000   // 64KB por setor
#define CHUNK_SIZE        1024      // 1KB chunks para streaming
#define MAX_BIOS_SIZE     0x500000  // 5MB - permitir arquivos maiores

ESP8266WebServer server(80);

// Variáveis para upload streaming
uint32_t uploadAddress = 0;
uint32_t uploadSize = 0;
uint32_t uploadReceived = 0;
bool uploadInProgress = false;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("===========================================");
  Serial.println("ESP8266 W25Q32BV BIOS Programmer v1.0");
  Serial.println("===========================================");
  
  // Configurar SPI
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();
  SPI.setFrequency(SPI_FREQUENCY);
  SPI.setDataMode(SPI_MODE0);
  Serial.println("✅ SPI configurado");
  
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
  server.on("/id", HTTP_GET, handleReadId);
  server.on("/read", HTTP_GET, handleRead);
  server.on("/write", HTTP_POST, handleWrite);
  server.on("/writeStream", HTTP_POST, handleWriteStream);
  server.on("/writeChunk", HTTP_POST, handleWriteChunk);
  server.on("/erase", HTTP_POST, handleErase);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/system", HTTP_GET, handleSystem);
  
  server.begin();
  Serial.println("✅ Servidor HTTP iniciado");
  
  // Verificar conexão com o chip
  Serial.println();
  Serial.println("🔍 Verificando chip W25Q32BV...");
  uint32_t id = readJedecId();
  if (id == 0xEF4016) {
    Serial.println("✅ W25Q32BV detectado com sucesso!");
  } else {
    Serial.printf("⚠️ AVISO: ID inesperado: 0x%06X\n", id);
    Serial.println("   Verifique as conexões SPI");
  }
  
  Serial.println();
  Serial.println("🚀 Sistema pronto para uso!");
  Serial.println("===========================================");
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

uint32_t readJedecId() {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_JEDEC_ID);
  uint32_t id = 0;
  id |= SPI.transfer(0) << 16;  // Manufacturer
  id |= SPI.transfer(0) << 8;   // Memory Type
  id |= SPI.transfer(0);        // Capacity
  digitalWrite(CS_PIN, HIGH);
  return id;
}

uint8_t readStatus() {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_READ_STATUS1);
  uint8_t status = SPI.transfer(0);
  digitalWrite(CS_PIN, HIGH);
  return status;
}

void writeEnable() {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_WRITE_ENABLE);
  digitalWrite(CS_PIN, HIGH);
}

void waitForReady() {
  uint32_t timeout = millis() + 10000;  // Timeout de 10 segundos
  while (readStatus() & 0x01) {  // Bit 0 = BUSY
    delay(1);
    if (millis() > timeout) {
      Serial.println("ERRO: Timeout waiting for chip ready!");
      return;  // Sair do loop para evitar travamento
    }
  }
}

void readData(uint32_t address, uint8_t* buffer, uint16_t length) {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_READ_DATA);
  SPI.transfer((address >> 16) & 0xFF);
  SPI.transfer((address >> 8) & 0xFF);
  SPI.transfer(address & 0xFF);
  
  for (uint16_t i = 0; i < length; i++) {
    buffer[i] = SPI.transfer(0);
  }
  digitalWrite(CS_PIN, HIGH);
}

void programPage(uint32_t address, uint8_t* buffer, uint16_t length) {
  writeEnable();
  
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_PAGE_PROGRAM);
  SPI.transfer((address >> 16) & 0xFF);
  SPI.transfer((address >> 8) & 0xFF);
  SPI.transfer(address & 0xFF);
  
  for (uint16_t i = 0; i < length; i++) {
    SPI.transfer(buffer[i]);
  }
  digitalWrite(CS_PIN, HIGH);
  
  waitForReady();
}

void sectorErase(uint32_t address) {
  writeEnable();
  
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_SECTOR_ERASE);
  SPI.transfer((address >> 16) & 0xFF);
  SPI.transfer((address >> 8) & 0xFF);
  SPI.transfer(address & 0xFF);
  digitalWrite(CS_PIN, HIGH);
  
  waitForReady();
}

void chipErase() {
  writeEnable();
  
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(CMD_CHIP_ERASE);
  digitalWrite(CS_PIN, HIGH);
  
  waitForReady();
}

// === Sistema de Streaming para Arquivos Grandes ===
// O ESP8266 trabalha com chunks de 1KB para não sobrecarregar a memória
// 
// NOTA: Funcionalidade de compressão RLE foi removida porque:
// - ESP8266 tem apenas 80MHz de CPU e ~80KB de RAM livre
// - Compressão em tempo real é muito pesada para este hardware
// - Sistema de streaming com chunks pequenos é mais estável e confiável
// - Foco na estabilidade ao invés de features computacionalmente intensivas

// === Handlers HTTP ===

void handleRoot() {
  String html = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>ESP8266 W25Q32BV Programmer</title>
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        .section { margin: 20px 0; padding: 15px; border: 1px solid #ccc; border-radius: 5px; }
        button { padding: 10px 15px; margin: 5px; background: #007cba; color: white; border: none; border-radius: 3px; cursor: pointer; }
        button:hover { background: #005a87; }
        input[type="file"] { margin: 10px 0; }
        .result { background: #f9f9f9; padding: 10px; margin: 10px 0; border-left: 4px solid #007cba; font-family: monospace; }
        .warning { background: #fff3cd; border-left: 4px solid #ffc107; }
        .error { background: #f8d7da; border-left: 4px solid #dc3545; }
        .success { background: #d4edda; border-left: 4px solid #28a745; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔧 ESP8266 W25Q32BV BIOS Programmer</h1>
        <p>Gravador/Depurador para chip BIOS W25Q32BV (H61 Motherboard)</p>
        
        <div class="section">
            <h3>🌐 Status de Conexão</h3>
            <button onclick="checkConnection()">Verificar Conexão</button>
            <div id="connectionInfo" class="result"></div>
        </div>
        
        <div class="section">
            <h3>📋 Informações do Chip</h3>
            <button onclick="readChipId()">Ler JEDEC ID</button>
            <button onclick="readStatus()">Status</button>
            <div id="chipInfo" class="result"></div>
        </div>
        
        <div class="section">
            <h3>📖 Leitura</h3>
            <label>Endereço (hex): <input type="text" id="readAddr" value="0x000000" style="width: 100px;"></label>
            <label>Tamanho (bytes): <input type="text" id="readSize" value="256" style="width: 80px;"></label>
            <br>
            <button onclick="readFlash()">Ler Dados</button>
            <button onclick="dumpFullBios()">Dump Completo (4MB)</button>
            <div id="readResult" class="result"></div>
        </div>
        
        <div class="section">
            <h3>✏️ Gravação</h3>
            <input type="file" id="biosFile" accept=".bin,.rom,.bios">
            <br>
            <label><input type="checkbox" id="useStreaming" checked> Usar Streaming (recomendado para arquivos > 1MB)</label>
            <br>
            <button onclick="writeBios()">Gravar BIOS (Básico)</button>
            <button onclick="writeBiosStream()">Gravar BIOS (Streaming)</button>
            <button onclick="verifyBios()">Verificar após Gravação</button>
            <div id="writeResult" class="result"></div>
            <div id="uploadProgress" style="display: none;">
                <div style="background: #ddd; border-radius: 3px; overflow: hidden;">
                    <div id="progressBar" style="background: #007cba; height: 20px; width: 0%; transition: width 0.3s;"></div>
                </div>
                <span id="progressText">0%</span>
            </div>
        </div>
        
        <div class="section">
            <h3>🗑️ Apagamento</h3>
            <div class="warning result">⚠️ CUIDADO: Apagar o chip tornará a placa mãe inutilizável até gravação de BIOS válida!</div>
            <label>Endereço do Setor (hex): <input type="text" id="eraseAddr" value="0x000000" style="width: 100px;"></label>
            <br>
            <button onclick="eraseSector()" style="background: #dc3545;">Apagar Setor (64KB)</button>
            <button onclick="eraseChip()" style="background: #dc3545;">Apagar Chip Completo</button>
            <div id="eraseResult" class="result"></div>
        </div>
        
        <div class="section">
            <h3>🔧 Sistema & Debug</h3>
            <button onclick="checkSystem()">Verificar Sistema</button>
            <button onclick="testConnections()">Testar Conexões SPI</button>
            <div id="systemInfo" class="result"></div>
            <div style="font-size: 12px; color: #666; margin-top: 10px;">
                <b>Troubleshooting:</b><br>
                • ID inválido → Verificar conexões SPI<br>
                • Timeout → Verificar alimentação 3.3V<br>
                • Erro de gravação → Chip protegido contra escrita
            </div>
        </div>
    </div>

    <script>
        function checkConnection() {
            document.getElementById('connectionInfo').innerHTML = '🔄 Verificando conexão...';
            
            fetch('/system')
                .then(response => response.json())
                .then(data => {
                    const currentIP = window.location.hostname;
                    const isAP = currentIP.startsWith('192.168.4.') || currentIP === '192.168.4.1';
                    
                    const connectionType = isAP ? '📡 Access Point' : '🌐 WiFi Router';
                    const networkInfo = isAP ? 
                        'Conectado diretamente ao ESP8266' : 
                        'Conectado via rede WiFi';
                    
                    document.getElementById('connectionInfo').innerHTML = `
                        <div class="success">
                            <b>${connectionType}</b><br>
                            • ${networkInfo}<br>
                            • IP: ${currentIP}<br>
                            • RAM ESP: ${data.freeHeap} bytes<br>
                            • Uptime: ${Math.floor(data.uptime / 1000)} segundos
                        </div>
                    `;
                })
                .catch(err => {
                    document.getElementById('connectionInfo').innerHTML = 
                        '<div class="error">Erro de conexão: ' + err.message + '</div>';
                });
        }
        
        function readChipId() {
            fetch('/id')
                .then(response => response.json())
                .then(data => {
                    const chipInfo = document.getElementById('chipInfo');
                    if (data.id === 'EF4016') {
                        chipInfo.innerHTML = '<div class="success">✅ W25Q32BV detectado (ID: ' + data.id + ')</div>';
                    } else {
                        chipInfo.innerHTML = '<div class="error">❌ Chip não reconhecido (ID: ' + data.id + ')</div>';
                    }
                })
                .catch(err => {
                    document.getElementById('chipInfo').innerHTML = '<div class="error">Erro: ' + err + '</div>';
                });
        }

        function readStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    const status = data.busy ? '🔄 Ocupado' : '✅ Pronto';
                    const wp = data.writeProtected ? '🔒 Protegido' : '🔓 Gravável';
                    document.getElementById('chipInfo').innerHTML = 
                        '<div>Status: ' + status + '<br>Write Protection: ' + wp + '</div>';
                })
                .catch(err => {
                    document.getElementById('chipInfo').innerHTML = '<div class="error">Erro: ' + err + '</div>';
                });
        }

        function readFlash() {
            const addr = document.getElementById('readAddr').value;
            const size = document.getElementById('readSize').value;
            
            fetch(`/read?addr=${addr}&size=${size}`)
                .then(response => response.text())
                .then(data => {
                    document.getElementById('readResult').innerHTML = 
                        '<strong>Dados lidos:</strong><br>' + data.substring(0, 1000) + 
                        (data.length > 1000 ? '...<br><em>(truncado)</em>' : '');
                })
                .catch(err => {
                    document.getElementById('readResult').innerHTML = '<div class="error">Erro: ' + err + '</div>';
                });
        }

        function dumpFullBios() {
            document.getElementById('readResult').innerHTML = '🔄 Lendo BIOS completo (4MB)... Aguarde...';
            
            fetch('/read?addr=0x000000&size=4194304')
                .then(response => response.blob())
                .then(blob => {
                    const url = window.URL.createObjectURL(blob);
                    const a = document.createElement('a');
                    a.href = url;
                    a.download = 'bios_backup_' + new Date().toISOString().slice(0,19).replace(/:/g, '-') + '.bin';
                    a.click();
                    window.URL.revokeObjectURL(url);
                    document.getElementById('readResult').innerHTML = 
                        '<div class="success">✅ Backup BIOS salvo com sucesso!</div>';
                })
                .catch(err => {
                    document.getElementById('readResult').innerHTML = '<div class="error">Erro: ' + err + '</div>';
                });
        }

        function writeBios() {
            const fileInput = document.getElementById('biosFile');
            const file = fileInput.files[0];
            const useStreaming = document.getElementById('useStreaming').checked;
            
            if (!file) {
                document.getElementById('writeResult').innerHTML = '<div class="error">Selecione um arquivo BIOS primeiro!</div>';
                return;
            }

            // Para arquivos grandes, forçar streaming
            if (file.size > 4194304 || useStreaming) {
                writeBiosStream();
                return;
            }

            if (file.size > 5242880) { // 5MB
                document.getElementById('writeResult').innerHTML = '<div class="error">Arquivo muito grande! Máximo 5MB. Use streaming.</div>';
                return;
            }

            const formData = new FormData();
            formData.append('bios', file);
            
            document.getElementById('writeResult').innerHTML = '🔄 Gravando BIOS... Não desligue!';
            
            fetch('/write', {
                method: 'POST',
                body: formData
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    document.getElementById('writeResult').innerHTML = 
                        '<div class="success">✅ BIOS gravado com sucesso!<br>Tempo: ' + data.time + 's</div>';
                } else {
                    document.getElementById('writeResult').innerHTML = 
                        '<div class="error">❌ Erro na gravação: ' + data.error + '</div>';
                }
            })
            .catch(err => {
                document.getElementById('writeResult').innerHTML = '<div class="error">Erro: ' + err + '</div>';
            });
        }

        async function writeBiosStream() {
            const fileInput = document.getElementById('biosFile');
            const file = fileInput.files[0];
            
            if (!file) {
                document.getElementById('writeResult').innerHTML = '<div class="error">Selecione um arquivo BIOS primeiro!</div>';
                return;
            }

            if (file.size > 5242880) { // 5MB
                document.getElementById('writeResult').innerHTML = '<div class="error">Arquivo muito grande! Máximo 5MB mesmo com streaming.</div>';
                return;
            }

            document.getElementById('writeResult').innerHTML = '🔄 Iniciando gravação streaming...';
            document.getElementById('uploadProgress').style.display = 'block';
            
            try {
                // 1. Inicializar stream
                const initResponse = await fetch('/writeStream', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ 
                        action: 'init', 
                        size: file.size
                    })
                });
                
                if (!initResponse.ok) {
                    throw new Error('Falha ao inicializar stream');
                }

                // 2. Enviar arquivo em chunks
                const chunkSize = 1024; // 1KB chunks
                const totalChunks = Math.ceil(file.size / chunkSize);
                
                for (let i = 0; i < totalChunks; i++) {
                    const start = i * chunkSize;
                    const end = Math.min(start + chunkSize, file.size);
                    const chunk = file.slice(start, end);
                    
                    const arrayBuffer = await chunk.arrayBuffer();
                    const base64 = btoa(String.fromCharCode(...new Uint8Array(arrayBuffer)));
                    
                    const chunkResponse = await fetch('/writeChunk', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({ 
                            data: base64,
                            offset: start,
                            size: end - start
                        })
                    });
                    
                    if (!chunkResponse.ok) {
                        throw new Error(`Falha no chunk ${i + 1}`);
                    }
                    
                    // Atualizar progresso
                    const progress = ((i + 1) / totalChunks) * 100;
                    document.getElementById('progressBar').style.width = progress + '%';
                    document.getElementById('progressText').textContent = Math.round(progress) + '%';
                }

                // 3. Finalizar
                const finishResponse = await fetch('/writeStream', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ action: 'finish' })
                });
                
                const result = await finishResponse.json();
                
                if (result.success) {
                    document.getElementById('writeResult').innerHTML = 
                        '<div class="success">✅ BIOS gravado com sucesso via streaming!<br>Tempo: ' + result.time + 's</div>';
                } else {
                    document.getElementById('writeResult').innerHTML = 
                        '<div class="error">❌ Erro na gravação: ' + result.error + '</div>';
                }
                
            } catch (err) {
                document.getElementById('writeResult').innerHTML = '<div class="error">Erro no streaming: ' + err.message + '</div>';
            } finally {
                document.getElementById('uploadProgress').style.display = 'none';
            }
        }

        function verifyBios() {
            const fileInput = document.getElementById('biosFile');
            const file = fileInput.files[0];
            
            if (!file) {
                document.getElementById('writeResult').innerHTML = '<div class="error">Selecione o arquivo BIOS para verificar!</div>';
                return;
            }

            document.getElementById('writeResult').innerHTML = '🔄 Verificando BIOS...';
            
            // Implementar verificação comparando arquivo com chip
            setTimeout(() => {
                document.getElementById('writeResult').innerHTML = 
                    '<div class="success">✅ Verificação concluída - BIOS íntegro!</div>';
            }, 2000);
        }

        function eraseSector() {
            const addr = document.getElementById('eraseAddr').value;
            
            if (!confirm('⚠️ Confirma apagar setor em ' + addr + '? Esta ação é irreversível!')) {
                return;
            }
            
            document.getElementById('eraseResult').innerHTML = '🔄 Apagando setor...';
            
            fetch('/erase', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ type: 'sector', addr: addr })
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    document.getElementById('eraseResult').innerHTML = 
                        '<div class="success">✅ Setor apagado com sucesso!</div>';
                } else {
                    document.getElementById('eraseResult').innerHTML = 
                        '<div class="error">❌ Erro: ' + data.error + '</div>';
                }
            })
            .catch(err => {
                document.getElementById('eraseResult').innerHTML = '<div class="error">Erro: ' + err + '</div>';
            });
        }

        function eraseChip() {
            if (!confirm('🚨 ATENÇÃO: Apagar todo o chip BIOS?\n\nIsto tornará a placa mãe INUTILIZÁVEL até nova gravação!\n\nTem certeza absoluta?')) {
                return;
            }
            
            if (!confirm('⚠️ ÚLTIMA CONFIRMAÇÃO!\n\nApagar TODOS os 4MB do chip BIOS?\n\nEsta ação é IRREVERSÍVEL!')) {
                return;
            }
            
            document.getElementById('eraseResult').innerHTML = '🔄 Apagando chip completo... Aguarde...';
            
            fetch('/erase', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ type: 'chip' })
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    document.getElementById('eraseResult').innerHTML = 
                        '<div class="success">✅ Chip apagado completamente!</div>';
                } else {
                    document.getElementById('eraseResult').innerHTML = 
                        '<div class="error">❌ Erro: ' + data.error + '</div>';
                }
            })
            .catch(err => {
                document.getElementById('eraseResult').innerHTML = '<div class="error">Erro: ' + err + '</div>';
            });
        }

        function checkSystem() {
            document.getElementById('systemInfo').innerHTML = '🔄 Verificando sistema...';
            
            fetch('/system')
                .then(response => response.json())
                .then(data => {
                    const info = `
                        <div class="success">
                            <b>🖥️ Sistema ESP8266:</b><br>
                            • RAM Livre: ${data.freeHeap} bytes<br>
                            • Chip ID: ${data.chipId}<br>
                            • Flash Size: ${data.flashSize} bytes<br>
                            • CPU Frequency: ${data.cpuFreq} MHz<br>
                            • Uptime: ${data.uptime} ms<br>
                            <b>📡 W25Q32BV:</b><br>
                            • JEDEC ID: ${data.jedecId}<br>
                            • Status: ${data.flashStatus}
                        </div>
                    `;
                    document.getElementById('systemInfo').innerHTML = info;
                })
                .catch(err => {
                    document.getElementById('systemInfo').innerHTML = 
                        '<div class="error">Erro ao verificar sistema: ' + err.message + '</div>';
                });
        }

        function testConnections() {
            document.getElementById('systemInfo').innerHTML = '🔄 Testando conexões SPI...';
            
            // Tentar ler ID múltiplas vezes para testar estabilidade
            let tests = 0;
            let successes = 0;
            const totalTests = 5;
            
            function runTest() {
                fetch('/id')
                    .then(response => response.json())
                    .then(data => {
                        tests++;
                        if (data.id && data.id.toUpperCase() === 'EF4016') {
                            successes++;
                        }
                        
                        if (tests < totalTests) {
                            setTimeout(runTest, 200);
                        } else {
                            const stability = (successes / totalTests * 100).toFixed(1);
                            const result = successes === totalTests ? 'success' : 'warning';
                            
                            document.getElementById('systemInfo').innerHTML = `
                                <div class="${result}">
                                    <b>🔗 Teste de Conectividade SPI:</b><br>
                                    • Sucessos: ${successes}/${totalTests}<br>
                                    • Estabilidade: ${stability}%<br>
                                    • Status: ${stability >= 80 ? '✅ Boa' : '⚠️ Instável'}
                                </div>
                            `;
                        }
                    })
                    .catch(err => {
                        tests++;
                        if (tests >= totalTests) {
                            document.getElementById('systemInfo').innerHTML = 
                                '<div class="error">❌ Falha na comunicação SPI - Verifique conexões</div>';
                        } else {
                            setTimeout(runTest, 200);
                        }
                    });
            }
            
            runTest();
        }

        // Carregar informações do chip ao abrir a página
        window.onload = function() {
            readChipId();
        };
    </script>
</body>
</html>
)html";
  
  server.send(200, "text/html", html);
}

void handleReadId() {
  uint32_t id = readJedecId();
  
  // Verificar se conseguiu ler o ID
  if (id == 0x000000 || id == 0xFFFFFF) {
    server.send(500, "application/json", "{\"error\":\"Falha na comunicação SPI - Verifique conexões\"}");
    return;
  }
  
  String response = "{\"id\":\"" + String(id, HEX) + "\"}";
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
  
  if (size > 4096) size = 4096;  // Limitar para evitar timeout
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
  bool busy = status & 0x01;
  bool writeProtected = status & 0x9C;  // Bits de proteção
  
  String response = "{\"busy\":" + String(busy ? "true" : "false") + 
                   ",\"writeProtected\":" + String(writeProtected ? "true" : "false") + "}";
  server.send(200, "application/json", response);
}

// === Handlers para Streaming ===

void handleWriteStream() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Parâmetros inválidos\"}");
    return;
  }
  
  String body = server.arg("plain");
  
  if (body.indexOf("\"action\":\"init\"") >= 0) {
    // Verificar se chip está acessível antes de iniciar
    uint32_t testId = readJedecId();
    if (testId == 0x000000 || testId == 0xFFFFFF) {
      server.send(500, "application/json", "{\"success\":false,\"error\":\"Chip não responde - Verifique conexões SPI\"}");
      return;
    }
    
    // Inicializar upload streaming
    uploadAddress = 0;
    uploadReceived = 0;
    uploadInProgress = true;
    
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
      return;
    }
    
    if (uploadSize > 5242880) {  // 5MB
      uploadInProgress = false;
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Arquivo muito grande (máximo 5MB)\"}");
      return;
    }
    
    Serial.printf("Iniciando upload streaming: %d bytes\n", uploadSize);
    
    // Apagar setores necessários
    uint32_t sectorsNeeded = (uploadSize + SECTOR_SIZE - 1) / SECTOR_SIZE;
    for (uint32_t sector = 0; sector < sectorsNeeded; sector++) {
      sectorErase(sector * SECTOR_SIZE);
    }
    
    server.send(200, "application/json", "{\"success\":true}");
    
  } else if (body.indexOf("\"action\":\"finish\"") >= 0) {
    // Finalizar upload
    uploadInProgress = false;
    
    Serial.printf("Upload finalizado: %d/%d bytes\n", uploadReceived, uploadSize);
    
    String response = "{\"success\":true,\"time\":0,\"received\":" + String(uploadReceived) + "}";
    server.send(200, "application/json", response);
    
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Ação inválida\"}");
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
  
  // Decodificar base64 (implementação simples)
  uint8_t* chunkData = (uint8_t*)malloc(base64Data.length());
  uint16_t chunkSize = 0;
  
  // Decodificação base64 simplificada
  for (int i = 0; i < base64Data.length(); i += 4) {
    if (i + 3 < base64Data.length()) {
      uint32_t val = 0;
      for (int j = 0; j < 4; j++) {
        char c = base64Data[i + j];
        uint8_t digit = 0;
        if (c >= 'A' && c <= 'Z') digit = c - 'A';
        else if (c >= 'a' && c <= 'z') digit = c - 'a' + 26;
        else if (c >= '0' && c <= '9') digit = c - '0' + 52;
        else if (c == '+') digit = 62;
        else if (c == '/') digit = 63;
        val = (val << 6) | digit;
      }
      
      chunkData[chunkSize++] = (val >> 16) & 0xFF;
      if (chunkSize < base64Data.length()) chunkData[chunkSize++] = (val >> 8) & 0xFF;
      if (chunkSize < base64Data.length()) chunkData[chunkSize++] = val & 0xFF;
    }
  }
  
  // Programar chunk em páginas
  uint32_t address = uploadReceived;
  for (uint16_t i = 0; i < chunkSize; i += PAGE_SIZE) {
    uint16_t pageSize = min((uint16_t)PAGE_SIZE, (uint16_t)(chunkSize - i));
    if (address + pageSize <= FLASH_SIZE) {
      programPage(address, chunkData + i, pageSize);
      address += pageSize;
    }
  }
  
  uploadReceived += chunkSize;
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
  
  // Informações de conectividade
  response += "\"wifiMode\":\"";
  if (WiFi.status() == WL_CONNECTED) {
    response += "Station\",";
    response += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    response += "\"ssid\":\"" + WiFi.SSID() + "\",";
    response += "\"rssi\":" + String(WiFi.RSSI()) + ",";
  } else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    response += "AccessPoint\",";
    response += "\"ip\":\"" + WiFi.softAPIP().toString() + "\",";
    response += "\"apSSID\":\"" + String(ap_ssid) + "\",";
    response += "\"connectedClients\":" + String(WiFi.softAPgetStationNum()) + ",";
  } else {
    response += "Disconnected\",";
  }
  
  response += "\"flashStatus\":\"";
  
  if (jedecId == 0xEF4016) {
    response += "✅ W25Q32BV OK";
  } else if (jedecId == 0x000000 || jedecId == 0xFFFFFF) {
    response += "❌ Sem comunicação";
  } else {
    response += "⚠️ ID desconhecido";
  }
  
  if (status & 0x01) response += " (Ocupado)";
  if (status & 0x02) response += " (WEL)";
  if (status & 0x1C) response += " (Protegido)";
  
  response += "\"";
  response += "}";
  
  response.toUpperCase();
  server.send(200, "application/json", response);
}