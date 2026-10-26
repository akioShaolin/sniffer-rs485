#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <Arduino.h>

struct ModbusConfig {
  uint8_t slaveId = 1;
  uint32_t baud = 9600;
  char parity = 'N';
  uint8_t stopBits = 1;
};

class ModbusManager {
  public:
    ModbusConfig cfg;
    uint16_t frameTimeoutMs = 4;

    void begin();
    SerialConfig serialMode();
    void applySerial();
    bool update(uint8_t id, uint32_t baud, char parity, uint8_t stopBits);
};

extern ModbusManager modbus;

#endif