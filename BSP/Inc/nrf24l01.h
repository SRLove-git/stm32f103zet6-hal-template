/**
 ******************************************************************************
 * @file    nrf24l01.h
 * @brief   NRF24L01 2.4 GHz transceiver driver (WIRELESS interface).
 *
 *          On the Elite board the module plugs into the WIRELESS socket:
 *            CE - PG8 | CS - PG7 | IRQ - PG6
 *            SCK/MISO/MOSI - SPI2 (PB13/PB14/PB15), shared with W25Q128.
 *
 *          Defaults match the ALIENTEK examples: channel 40, 5-byte address
 *          0x34 0x43 0x10 0x10 0x01, 2 Mbps, 0 dBm, auto-ACK, 32-byte payload.
 *
 * @note  A link test needs TWO modules on TWO boards. While the NRF is active,
 *        keep the W25Q128 chip-select high (and vice versa) - they share SPI2.
 ******************************************************************************
 */

#ifndef __NRF24L01_H
#define __NRF24L01_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define NRF_CE_PORT GPIOG
#define NRF_CE_PIN GPIO_PIN_8
#define NRF_CS_PORT GPIOG
#define NRF_CS_PIN GPIO_PIN_7
#define NRF_IRQ_PORT GPIOG
#define NRF_IRQ_PIN GPIO_PIN_6

    /**
     * @brief Initialize GPIO, SPI2 and the transceiver defaults.
     * @retval 0 on success, 1 if the module is not detected (SPI read-back).
     */
    uint8_t NRF24L01_Init(void);

    /**
     * @brief Switch to transmitter mode (CE low).
     */
    void NRF24L01_SetTxMode(void);

    /**
     * @brief Switch to receiver mode (CE high).
     */
    void NRF24L01_SetRxMode(void);

    /**
     * @brief Send one packet (max 32 bytes, blocking with timeout).
     * @retval 0 sent OK (TX_DS), 1 failed (MAX_RT / timeout).
     */
    uint8_t NRF24L01_TxPacket(uint8_t* buf, uint8_t len);

    /**
     * @brief Poll for one received packet.
     * @param buf [out] payload, len [in/out] max/actual length.
     * @retval 0 packet received, 1 nothing in the RX FIFO.
     */
    uint8_t NRF24L01_RxPacket(uint8_t* buf, uint8_t* len);

#ifdef __cplusplus
}
#endif

#endif /* __NRF24L01_H */
