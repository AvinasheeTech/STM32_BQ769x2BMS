/**
  ******************************************************************************
  * @file           BMS_SPI.h
  * @brief          functions and structure related to SPI communication protocol
  ******************************************************************************
  */

#ifndef _BMS_SPI_H_
#define _BMS_SPI_H_

#include "main.h"

/** @defgroup SPI_Error_Code SPI Error Code
  * @brief    SPI Error Code
  * @{
  */
#define SPI_ERROR_NONE         ((uint32_t)0x00000000)   /*!< No error             */
#define SPI_ERROR_MODF         ((uint32_t)0x00000001)   /*!< MODF error           */
#define SPI_ERROR_CRC          ((uint32_t)0x00000002)   /*!< CRC error            */
#define SPI_ERROR_OVR          ((uint32_t)0x00000004)   /*!< OVR error            */
#define SPI_ERROR_FRE          ((uint32_t)0x00000008)   /*!< FRE error            */
#define SPI_ERROR_DMA          ((uint32_t)0x00000010)   /*!< DMA transfer error   */
#define SPI_ERROR_FLAG         ((uint32_t)0x00000010)   /*!< Flag: RXNE,TXE, BSY  */
/**
  * @}
  */

/**
 * @brief SPI structure for handling data
 */
typedef struct{
	SPI_TypeDef *Instance;
	uint8_t TxXfersize;   /* SPI Tx transfer size */
	uint8_t TxXfercount;  /* SPI Tx Transfer Counter */
	uint8_t *pTxbuffer;   /* Pointer to SPI Tx transfer Buffer */
	uint8_t RxXfersize;   /* SPI Rx transfer size */
	uint8_t RxXfercount;  /* SPI Rx Transfer Counter */
	uint8_t *pRxbuffer;   /* Pointer to SPI Rx transfer Buffer */
	uint32_t errorcode;   /* SPI Error code */
	uint16_t timeoutcount;/* SPI timeout count */
}SPI_Handle_t;

void SPI_WriteReg(uint8_t reg_addr,uint8_t* reg_data,uint8_t count);
void SPI_ReadReg(uint8_t reg_addr,uint8_t* reg_data,uint8_t count);
void SPI_TransmitRecieve(SPI_Handle_t *hSPIx,uint8_t* Tx_data,uint8_t* Rx_data,uint8_t count);


#endif  //_BMS_SPI_H_ end
