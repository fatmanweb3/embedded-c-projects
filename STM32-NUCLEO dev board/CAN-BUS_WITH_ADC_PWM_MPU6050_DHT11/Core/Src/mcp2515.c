#include "mcp2515.h"
#include <string.h>

// Helper function to control CS pin
static void MCP2515_Select(MCP2515_Handle *hcan) {
    HAL_GPIO_WritePin(hcan->cs_port, hcan->cs_pin, GPIO_PIN_RESET);
}

static void MCP2515_Deselect(MCP2515_Handle *hcan) {
    HAL_GPIO_WritePin(hcan->cs_port, hcan->cs_pin, GPIO_PIN_SET);
}

// Write to MCP2515 register
void MCP2515_WriteReg(MCP2515_Handle *hcan, uint8_t reg, uint8_t value) {
    uint8_t tx_buf[3] = {MCP2515_WRITE, reg, value};
    MCP2515_Select(hcan);
    HAL_SPI_Transmit(hcan->hspi, tx_buf, 3, HAL_MAX_DELAY);
    MCP2515_Deselect(hcan);
}

// Read from MCP2515 register
uint8_t MCP2515_ReadReg(MCP2515_Handle *hcan, uint8_t reg) {
    uint8_t tx_buf[2] = {MCP2515_READ, reg};
    uint8_t rx_buf = 0;
    MCP2515_Select(hcan);
    HAL_SPI_Transmit(hcan->hspi, tx_buf, 2, HAL_MAX_DELAY);
    HAL_SPI_Receive(hcan->hspi, &rx_buf, 1, HAL_MAX_DELAY);
    MCP2515_Deselect(hcan);
    return rx_buf;
}

// Reset MCP2515
static void MCP2515_Reset(MCP2515_Handle *hcan) {
    uint8_t tx_buf = MCP2515_RESET;
    MCP2515_Select(hcan);
    HAL_SPI_Transmit(hcan->hspi, &tx_buf, 1, HAL_MAX_DELAY);
    MCP2515_Deselect(hcan);
    HAL_Delay(10); // Wait for reset
}

// Set MCP2515 mode
void MCP2515_SetMode(MCP2515_Handle *hcan, uint8_t mode) {
    MCP2515_WriteReg(hcan, MCP2515_CANCTRL, mode);
    HAL_Delay(1);
    while ((MCP2515_ReadReg(hcan, MCP2515_CANSTAT) & 0xE0) != mode) {
        HAL_Delay(1); // Wait for mode change
    }
}

// Initialize MCP2515
void MCP2515_Init(MCP2515_Handle *hcan, uint32_t bitrate) {
    // Reset MCP2515
    MCP2515_Reset(hcan);

    // Enter configuration mode
    MCP2515_SetMode(hcan, MCP2515_MODE_CONFIG);

    // Configure bitrate (500 kbps @ 16 MHz MCP2515 clock)
    if (bitrate == 500000) {
        MCP2515_WriteReg(hcan, MCP2515_CNF1, 0x01); // SJW=1, BRP=1
        MCP2515_WriteReg(hcan, MCP2515_CNF2, 0xB1); // BTLMODE=1, PHSEG1=6, PRSEG=1
        MCP2515_WriteReg(hcan, MCP2515_CNF3, 0x05); // PHSEG2=6
    }

    // Configure RX buffer (receive all messages)
    MCP2515_WriteReg(hcan, MCP2515_RXB0CTRL, 0x60); // Receive all
    MCP2515_WriteReg(hcan, MCP2515_RXB0SIDH, 0x00); // Filter ID 0
    MCP2515_WriteReg(hcan, MCP2515_RXB0SIDL, 0x00);

    // Enable interrupts (optional, for polling we disable)
    MCP2515_WriteReg(hcan, MCP2515_CANINTE, 0x00); // No interrupts

    // Enter normal mode
    MCP2515_SetMode(hcan, MCP2515_MODE_NORMAL);
}

// Transmit CAN message
void MCP2515_Transmit(MCP2515_Handle *hcan, CAN_Message *msg) {
    // Write message to TXB0
    MCP2515_WriteReg(hcan, MCP2515_TXB0SIDH, (msg->id >> 3));           // SIDH
    MCP2515_WriteReg(hcan, MCP2515_TXB0SIDL, ((msg->id & 0x07) << 5));  // SIDL
    MCP2515_WriteReg(hcan, MCP2515_TXB0DLC, msg->dlc & 0x0F);           // DLC
    for (uint8_t i = 0; i < msg->dlc; i++) {
        MCP2515_WriteReg(hcan, MCP2515_TXB0D0 + i, msg->data[i]);       // Data
    }

    // Request transmission
    MCP2515_WriteReg(hcan, MCP2515_TXB0CTRL, 0x08); // Set TXREQ
}

// Receive CAN message (polling)
uint8_t MCP2515_Receive(MCP2515_Handle *hcan, CAN_Message *msg) {
    // Check RXB0 status
    uint8_t status = MCP2515_ReadReg(hcan, MCP2515_CANINTF);
    if (!(status & MCP2515_INT_RX0IF)) {
        return 0; // No message
    }

    // Read message
    msg->id = (MCP2515_ReadReg(hcan, MCP2515_RXB0SIDH) << 3) |
              (MCP2515_ReadReg(hcan, MCP2515_RXB0SIDL) >> 5);
    msg->dlc = MCP2515_ReadReg(hcan, MCP2515_RXB0DLC) & 0x0F;
    for (uint8_t i = 0; i < msg->dlc; i++) {
        msg->data[i] = MCP2515_ReadReg(hcan, MCP2515_RXB0D0 + i);
    }

    // Clear RX interrupt flag
    MCP2515_WriteReg(hcan, MCP2515_CANINTF, status & ~MCP2515_INT_RX0IF);
    return 1; // Message received
}