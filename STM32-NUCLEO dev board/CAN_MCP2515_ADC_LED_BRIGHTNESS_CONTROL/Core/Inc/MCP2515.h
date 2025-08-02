#ifndef MCP2515_H_
#define MCP2515_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

// MCP2515 SPI Commands
#define MCP2515_RESET       0xC0 // 1100 0000
#define MCP2515_READ        0x03 // 0000 0011
#define MCP2515_WRITE       0x02 // 0000 0010
#define MCP2515_RTS_TX0     0x81 // 1000 0001
#define MCP2515_READ_STATUS 0xA0 // 1010 0000
#define MCP2515_RX_STATUS   0xB0 // 1011 0000
#define MCP2515_BIT_MODIFY  0x05 // 0000 0101

// MCP2515 Registers
#define MCP2515_RXF0SIDH    0x00 // 0000 0000
#define MCP2515_RXF0SIDL    0x01 // 0000 0001
#define MCP2515_BFPCTRL     0x0C // 0000 1100
#define MCP2515_TXRTSCTRL   0x0D // 0000 1101
#define MCP2515_CANSTAT     0x0E // 0000 1110
#define MCP2515_CANCTRL     0x0F // 0000 1111
#define MCP2515_CNF3        0x28 // 0010 1000
#define MCP2515_CNF2        0x29 // 0010 1001
#define MCP2515_CNF1        0x2A // 0010 1010
#define MCP2515_CANINTE     0x2B // 0010 1011
#define MCP2515_CANINTF     0x2C // 0010 1100
#define MCP2515_TXB0CTRL    0x30 // 0011 0000
#define MCP2515_TXB0SIDH    0x31 // 0011 0001
#define MCP2515_TXB0SIDL    0x32 // 0011 0010
#define MCP2515_TXB0DLC     0x35 // 0011 0101
#define MCP2515_TXB0D0      0x36 // 0011 0110
#define MCP2515_RXB0CTRL    0x60 // 0110 0000
#define MCP2515_RXB0SIDH    0x61 // 0110 0001
#define MCP2515_RXB0SIDL    0x62 // 0110 0010
#define MCP2515_RXB0DLC     0x65 // 0110 0101
#define MCP2515_RXB0D0      0x66 // 0110 0110

// MCP2515 Configuration
#define MCP2515_MODE_NORMAL     0x00 // 0000 0000
#define MCP2515_MODE_CONFIG     0x80 // 1000 0000
#define MCP2515_INT_RX0IF       0x01 // 0000 0001
#define MCP2515_INT_TX0IF       0x04 // 0000 0100

// CAN Message Structure
typedef struct {
    uint32_t id;        // Standard ID (11-bit)
    uint8_t dlc;        // Data Length Code (0-8)
    uint8_t data[8];    // Data bytes
} CAN_Message;

// MCP2515 Handle Structure
typedef struct {
    SPI_HandleTypeDef *hspi;    // SPI handle
    GPIO_TypeDef *cs_port;      // CS GPIO Port
    uint16_t cs_pin;            // CS GPIO Pin
} MCP2515_Handle;

// Function Prototypes
void MCP2515_Init(MCP2515_Handle *hcan, uint32_t bitrate);
void MCP2515_SetMode(MCP2515_Handle *hcan, uint8_t mode);
void MCP2515_Transmit(MCP2515_Handle *hcan, CAN_Message *msg);
uint8_t MCP2515_Receive(MCP2515_Handle *hcan, CAN_Message *msg);
void MCP2515_WriteReg(MCP2515_Handle *hcan, uint8_t reg, uint8_t value);
uint8_t MCP2515_ReadReg(MCP2515_Handle *hcan, uint8_t reg);

#endif /* MCP2515_H_ */