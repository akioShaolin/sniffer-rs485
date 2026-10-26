#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "ModbusManager.h"
#include "web.h"

// Configuração de Hardware
#define DE_RE 12 // Controle do transceptor RS45 (DE/RE)
#define HALF_OR_FULL_PIN  13            // Half-Duplex:LOW  Full-Duplex-HIGH
#define BTN_PIN           0             // Botão da placa com pull up

// ========== WiFi ==========

const char* ssid = "Sniffer WiFi";
const char* password = "1234567890";

// ========== OBJETOS ==========
ESP8266WebServer server(80);
WebSocketsServer ws = WebSocketsServer(81);

// Buffer de frame
String frameBuffer;
bool inFrame = false;

// ========== WEBSOCKET ==========

uint8_t wsClient = 255;

void wsEvent (uint8_t num, WStype_t type, uint8_t * payload, size_t len) {
  
  switch (type) {

    case WStype_CONNECTED:
      wsClient = num;
      break;

    case WStype_DISCONNECTED:
      if (wsClient == num)  { 
        wsClient = 255;
        frameBuffer = "";
        inFrame = false;
      }
      break; 
  }
}

// ========== ROTAS ==========

void handleRoot() {
  server.send_P(200, "text/html", HTML_PAGE);
}

void handleConfigPage() {
  server.send_P(200, "text/html", CONFIG_PAGE);
}

void handleGetModbusConfig() {
  String json = "{";
  json += "\"id\":" + String(modbus.cfg.slaveId) + ",";
  json += "\"baud\":" + String(modbus.cfg.baud) + ",";
  json += "\"parity\":\"" + String(modbus.cfg.parity) + "\",";
  json += "\"stopBits\":" + String(modbus.cfg.stopBits);
  json += "}";

  server.send(200, "application/json", json);
}

void handleSetModbusConfig() {
  if (!server.hasArg("id") || !server.hasArg("baud") || !server.hasArg("parity") || !server.hasArg("stopBits")) {
    server.send(400, "text/plain", "Parametros incompletos");
    return;
  }

  uint8_t id = server.arg("id").toInt();
  uint32_t baud = server.arg("baud").toInt();
  char parity = server.arg("parity")[0];
  uint8_t stopBits = server.arg("stopBits").toInt();

  if (!modbus.update(id, baud, parity, stopBits)) {
    server.send(400, "text/plain", "Configuracao invalida");
    return;
  }

  server.send(200, "text/plain", "Configuracao Modbus salva");
}

// ========== MODBUS ==========
void sniff() {
  static uint32_t lastByteTime = 0;
  static bool inFrame = false;
  float time = 0;
  char buffer [20];
  
  // 1. Processamento de Bytes (Não-Bloqueante)
  while (Serial.available()) {
    uint8_t b = Serial.read();
    
    // Se a trama anterior terminou, ou se estamos começando uma nova
    if (!inFrame) {
      time = (float)millis() / 1000;
      frameBuffer = "";
      sprintf(buffer, "%5.3f [RX] ", time);
      frameBuffer = buffer;
      inFrame = true;
    }
    
    char hex[4];
    sprintf(hex, "%02X ", b);
    frameBuffer += hex;

    lastByteTime = millis();
    
    // O yield() é importante para o ESP8266, mas deve ser usado com moderação
    // para não introduzir atrasos no loop de leitura.
    // yield(); 
  }

  // 2. Detecção de Fim de Trama (Timeout)
  // Verifica se a trama estava ativa e se o tempo de silêncio Modbus (4ms) foi excedido
  if (inFrame && (millis() - lastByteTime > modbus.frameTimeoutMs)) {
    if (wsClient != 255) {
      ws.broadcastTXT(frameBuffer);
    }
    inFrame = false;
  }
}

void setup() {
  modbus.begin();
  
  // WIFI AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // WebSocketsServer
  ws.begin();
  ws.onEvent(wsEvent);

  // SERVER
  server.on("/", handleRoot);
  server.on("/config", handleConfigPage);
  server.on("/api/modbus", HTTP_GET, handleGetModbusConfig);
  server.on("/api/modbus", HTTP_POST, handleSetModbusConfig);
  server.begin();

  // Configuração dos pinos
  // O pino HALF_OR_FULL_PIN se refere a Half ou Full Duplex, devido a um HC4066 selecionando entre 2 transceivers de RS485.
  pinMode(HALF_OR_FULL_PIN, OUTPUT);
  pinMode(DE_RE, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); // Pino 2 é o LED_BUILTIN em alguns ESPs, mas você o usou para RX do Debug. Vou usar o LED_BUILTIN para o pino 13.
  pinMode(BTN_PIN, INPUT_PULLUP); 
  
  // Configura o transceptor para modo de recepção (LOW)
  digitalWrite(DE_RE, LOW);
  digitalWrite(HALF_OR_FULL_PIN, LOW);  //Define a Rede RS485 como Half Duplex
  digitalWrite(LED_BUILTIN, LOW); // LED ligado
}

void loop() {
  sniff();
  // O loop roda o mais rápido possível, maximizando a chance de ler o buffer serial.
  ws.loop();
  server.handleClient();
}