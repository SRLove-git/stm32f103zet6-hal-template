/**
 ******************************************************************************
 * @file    nrf24l01.c
 * @brief   NRF24L01 transceiver driver (SPI2, shared with W25Q128).
 ******************************************************************************
 */

#include "nrf24l01.h"
#include "bsp_dwt.h"

extern SPI_HandleTypeDef hspi2; /* shared SPI2 handle (see w25qxx.c) */

/* Registers */
#define NRF_CONFIG 0x00U
#define NRF_EN_AA 0x01U
#define NRF_EN_RXADDR 0x02U
#define NRF_SETUP_AW 0x03U
#define NRF_SETUP_RETR 0x04U
#define NRF_RF_CH 0x05U
#define NRF_RF_SETUP 0x06U
#define NRF_STATUS 0x07U
#define NRF_RX_ADDR_P0 0x0AU
#define NRF_TX_ADDR 0x10U
#define NRF_RX_PW_P0 0x11U

/* Commands */
#define NRF_CMD_R_REGISTER 0x00U
#define NRF_CMD_W_REGISTER 0x20U
#define NRF_CMD_R_RX_PAYLOAD 0x61U
#define NRF_CMD_W_TX_PAYLOAD 0xA0U
#define NRF_CMD_FLUSH_TX 0xE1U
#define NRF_CMD_FLUSH_RX 0xE2U
#define NRF_CMD_NOP 0xFFU

/* STATUS bits */
#define NRF_STATUS_RX_DR 0x40U
#define NRF_STATUS_TX_DS 0x20U
#define NRF_STATUS_MAX_RT 0x10U

#define NRF_MAX_PAYLOAD 32U
#define NRF_TIMEOUT_MS 500U

static const uint8_t nrf_addr[5] = {0x34U, 0x43U, 0x10U, 0x10U, 0x01U};

static void NRF_CE_Write(uint8_t level)
{
    HAL_GPIO_WritePin(NRF_CE_PORT, NRF_CE_PIN, (level != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void NRF_CS_Write(uint8_t level)
{
    HAL_GPIO_WritePin(NRF_CS_PORT, NRF_CS_PIN, (level != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t NRF_SpiByte(uint8_t byte)
{
    uint8_t rx = 0U;
    if (HAL_SPI_TransmitReceive(&hspi2, &byte, &rx, 1U, 100U) != HAL_OK)
    {
        Error_Handler();
    }
    return rx;
}

static uint8_t NRF_ReadReg(uint8_t reg)
{
    uint8_t value;

    NRF_CS_Write(0U);
    (void)NRF_SpiByte(NRF_CMD_R_REGISTER | reg);
    value = NRF_SpiByte(NRF_CMD_NOP);
    NRF_CS_Write(1U);
    return value;
}

static void NRF_WriteReg(uint8_t reg, uint8_t value)
{
    NRF_CS_Write(0U);
    (void)NRF_SpiByte(NRF_CMD_W_REGISTER | reg);
    (void)NRF_SpiByte(value);
    NRF_CS_Write(1U);
}

static void NRF_ReadBuf(uint8_t reg, uint8_t* buf, uint8_t len)
{
    uint8_t i;

    NRF_CS_Write(0U);
    (void)NRF_SpiByte(NRF_CMD_R_REGISTER | reg);
    for (i = 0U; i < len; i++)
    {
        buf[i] = NRF_SpiByte(NRF_CMD_NOP);
    }
    NRF_CS_Write(1U);
}

static void NRF_WriteBuf(uint8_t reg, const uint8_t* buf, uint8_t len)
{
    uint8_t i;

    NRF_CS_Write(0U);
    (void)NRF_SpiByte(NRF_CMD_W_REGISTER | reg);
    for (i = 0U; i < len; i++)
    {
        (void)NRF_SpiByte(buf[i]);
    }
    NRF_CS_Write(1U);
}

void NRF24L01_SetRxMode(void)
{
    NRF_CE_Write(0U);
    NRF_WriteReg(NRF_CONFIG, 0x0FU); /* PWR_UP | EN_CRC | CRCO | PRIM_RX */
    NRF_CE_Write(1U);
}

void NRF24L01_SetTxMode(void)
{
    NRF_CE_Write(0U);
    NRF_WriteReg(NRF_CONFIG, 0x0EU); /* PWR_UP | EN_CRC | CRCO */
    NRF_CE_Write(0U);
}

uint8_t NRF24L01_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    uint8_t buf[5] = {0xCAU, 0x57U, 0xCAU, 0x57U, 0xCAU};
    uint8_t i;

    __HAL_RCC_GPIOG_CLK_ENABLE();
    BSP_DWT_DelayInit();

    /* CE/CS outputs, IRQ input with pull-up */
    GPIO_InitStruct.Pin = NRF_CE_PIN | NRF_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    NRF_CE_Write(0U);
    NRF_CS_Write(1U);

    GPIO_InitStruct.Pin = NRF_IRQ_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* SPI2 is shared with W25Q128; (re)configure it here to stay standalone */
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4; /* ~18 MHz */
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 10U;
    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        Error_Handler();
    }

    /* Transceiver defaults (ALIENTEK-compatible) */
    NRF_WriteReg(NRF_EN_AA, 0x01U);      /* auto-ACK on pipe 0 */
    NRF_WriteReg(NRF_EN_RXADDR, 0x01U);  /* enable pipe 0 */
    NRF_WriteReg(NRF_SETUP_AW, 0x03U);   /* 5-byte addresses */
    NRF_WriteReg(NRF_SETUP_RETR, 0x1AU); /* 500 us, 10 retries */
    NRF_WriteReg(NRF_RF_CH, 40U);        /* channel 40 */
    NRF_WriteReg(NRF_RF_SETUP, 0x0FU);   /* 2 Mbps, 0 dBm */
    NRF_WriteReg(NRF_RX_PW_P0, NRF_MAX_PAYLOAD);
    NRF_WriteBuf(NRF_TX_ADDR, nrf_addr, 5U);
    NRF_WriteBuf(NRF_RX_ADDR_P0, nrf_addr, 5U);
    NRF_WriteReg(NRF_STATUS, 0x70U); /* clear IRQ flags */
    (void)NRF_SpiByte(NRF_CMD_FLUSH_TX);
    (void)NRF_SpiByte(NRF_CMD_FLUSH_RX);

    NRF24L01_SetRxMode();

    /* SPI/register check: write a pattern to TX_ADDR and read it back */
    NRF_WriteBuf(NRF_TX_ADDR, buf, 5U);
    NRF_ReadBuf(NRF_TX_ADDR, buf, 5U);
    for (i = 0U; i < 5U; i++)
    {
        if (buf[i] != ((i % 2U == 0U) ? 0xCAU : 0x57U))
        {
            return 1U;
        }
    }
    return 0U;
}

uint8_t NRF24L01_TxPacket(uint8_t* buf, uint8_t len)
{
    uint32_t start;
    uint8_t status;

    if (len > NRF_MAX_PAYLOAD)
    {
        return 1U;
    }

    NRF24L01_SetTxMode();

    NRF_CS_Write(0U);
    (void)NRF_SpiByte(NRF_CMD_W_TX_PAYLOAD);
    while (len-- > 0U)
    {
        (void)NRF_SpiByte(*buf++);
    }
    NRF_CS_Write(1U);

    /* CE pulse >= 10 us starts the transmission */
    NRF_CE_Write(1U);
    BSP_DWT_DelayUs(15U);
    NRF_CE_Write(0U);

    start = HAL_GetTick();
    do
    {
        status = NRF_ReadReg(NRF_STATUS);
        if ((status & NRF_STATUS_TX_DS) != 0U)
        {
            NRF_WriteReg(NRF_STATUS, 0x70U);
            (void)NRF_SpiByte(NRF_CMD_FLUSH_TX);
            return 0U;
        }
        if ((status & NRF_STATUS_MAX_RT) != 0U)
        {
            NRF_WriteReg(NRF_STATUS, 0x70U);
            (void)NRF_SpiByte(NRF_CMD_FLUSH_TX);
            return 1U;
        }
    } while ((HAL_GetTick() - start) < NRF_TIMEOUT_MS);

    return 1U;
}

uint8_t NRF24L01_RxPacket(uint8_t* buf, uint8_t* len)
{
    uint8_t status;
    uint8_t i;
    uint8_t max_len = *len;

    status = NRF_ReadReg(NRF_STATUS);
    if ((status & NRF_STATUS_RX_DR) == 0U)
    {
        return 1U;
    }

    NRF_CS_Write(0U);
    (void)NRF_SpiByte(NRF_CMD_R_RX_PAYLOAD);
    for (i = 0U; (i < max_len) && (i < NRF_MAX_PAYLOAD); i++)
    {
        buf[i] = NRF_SpiByte(NRF_CMD_NOP);
    }
    NRF_CS_Write(1U);

    NRF_WriteReg(NRF_STATUS, 0x70U);
    (void)NRF_SpiByte(NRF_CMD_FLUSH_RX);
    *len = i;
    return 0U;
}
