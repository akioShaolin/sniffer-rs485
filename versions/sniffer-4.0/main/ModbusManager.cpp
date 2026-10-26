#include "ModbusManager.h"

ModbusManager modbus;

void ModbusManager::begin() {
  applySerial();
}

SerialConfig ModbusManager::serialMode() {
  if (cfg.parity == 'E' && cfg.stopBits == 1) return SERIAL_8E1;
  if (cfg.parity == 'E' && cfg.stopBits == 2) return SERIAL_8E2;
  if (cfg.parity == 'O' && cfg.stopBits == 1) return SERIAL_8O1;
  if (cfg.parity == 'O' && cfg.stopBits == 2) return SERIAL_8O2;
  if (cfg.parity == 'N' && cfg.stopBits == 2) return SERIAL_8N2;
  return SERIAL_8N1;
}

void ModbusManager::applySerial() {
  Serial.flush();
  Serial.end();
  delay(50);

  Serial.begin(cfg.baud, serialMode());//serialMode());

  frameTimeoutMs = ((3500000UL * 11UL) / cfg.baud) / 1000UL;
  if (frameTimeoutMs < 2) frameTimeoutMs = 2;
}

bool ModbusManager::update(uint8_t id, uint32_t baud, char parity, uint8_t stopBits) {
  if (id < 1 || id > 247) return false;
  if (!(parity == 'N' || parity == 'E' || parity == 'O')) return false;
  if (!(stopBits == 1 || stopBits == 2)) return false;

  cfg.slaveId = id;
  cfg.baud = baud;
  cfg.parity = parity;
  cfg.stopBits = stopBits;

  applySerial();
  return true;
}