/**
  ******************************************************************************
  * @file           BMS_SPI.c
  * @brief          SPI protocol handling source file
  ******************************************************************************
  */

#include "BMS_SPI.h"

#define SPI_used       SPI1

SPI_Handle_t hSPIx;  //handler for SPI basic param structure
uint8_t Rx_data[] = {0x00,0x00,0x00};

/**
 * @brief  function to write BQ769x2 registers over SPI
 * @param  reg_addr address of register to be written
 * @param  reg_data pointer to data which is to be written
 * @param  count number of data bytes to be written
 * @retval None
 * @note   includes retries in case HFO has not started or if wait time is needed. See BQ76952 Software Development Guide for examples
 */
void SPI_WriteReg(uint8_t reg_addr,uint8_t* reg_data,uint8_t count){
    uint8_t addr = 0x80 | reg_addr;      //set R/W bit high for writing + 7bit address
    uint8_t TX_Buffer[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	unsigned int i;
	unsigned int match = 0;
	unsigned int retries = 10;
	hSPIx.Instance = SPI_used;

    for(i=0; i<count; i++) {
    	TX_Buffer[0] = addr;
		TX_Buffer[1] = reg_data[i];

		LL_GPIO_ResetOutputPin(SPI1_CS_GPIO_Port,SPI1_CS_Pin);
		SPI_TransmitRecieve(&hSPIx, TX_Buffer, Rx_data, 2);
		LL_GPIO_SetOutputPin(SPI1_CS_GPIO_Port,SPI1_CS_Pin);

		while ((match == 0) & (retries > 0)) {
			delay_ms(5);
			LL_GPIO_ResetOutputPin(SPI1_CS_GPIO_Port,SPI1_CS_Pin);
			SPI_TransmitRecieve(&hSPIx, TX_Buffer, Rx_data, 2);
			LL_GPIO_SetOutputPin(SPI1_CS_GPIO_Port,SPI1_CS_Pin);
			if ((Rx_data[0] == addr) & (Rx_data[1] == reg_data[i])){
				match = 1;
			}
			retries --;
		}

	    match = 0;
	    addr += 1;
	    delay_ms(5);
	  }

}

/**
 * @brief  function to read BQ769x2 registers over SPI
 * @param  reg_addr address of register to read from
 * @param  reg_data pointer to data to store read bytes
 * @param  count number of data bytes to be read
 * @retval None
 * @note   includes retries in case HFO has not started or if wait time is needed. See BQ76952 Software Development Guide for examples
 */
void SPI_ReadReg(uint8_t reg_addr,uint8_t* reg_data,uint8_t count){
    uint8_t addr;
    uint8_t TX_Buffer[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    unsigned int i;
    unsigned int match;
    unsigned int retries = 10;

    match = 0;
    addr = reg_addr;

    for(i=0; i<count; i++) {
		TX_Buffer[0] = addr;
		TX_Buffer[1] = 0xFF;

		LL_GPIO_ResetOutputPin(SPI1_CS_GPIO_Port,SPI1_CS_Pin);
		SPI_TransmitRecieve(&hSPIx, TX_Buffer, Rx_data, 2);
		LL_GPIO_SetOutputPin(SPI1_CS_GPIO_Port,SPI1_CS_Pin);

		while ((match == 0) & (retries > 0)) {
			delay_ms(5);
			LL_GPIO_ResetOutputPin(SPI1_CS_GPIO_Port,SPI1_CS_Pin);
			SPI_TransmitRecieve(&hSPIx, TX_Buffer, Rx_data, 2);
			LL_GPIO_SetOutputPin(SPI1_CS_GPIO_Port,SPI1_CS_Pin);
			if (Rx_data[0] == addr) {
				match = 1;
				reg_data[i] = Rx_data[1];
			}
			retries --;
		}
	match = 0;
	addr += 1;
	delay_ms(5);
  }
}

/**
 * @brief  function to perform transmit and recieve operations over SPI protocol
 * @param  hSPIx   pointer to SPI_handle_t structure
 * @param  Tx_data pointer to data to be trasnmitted
 * @param  Rx_data pointer to data to be received
 * @param  count number of data bytes to be sent and received
 * @retval None
 */
void SPI_TransmitRecieve(SPI_Handle_t *hSPIx,uint8_t* Tx_data,uint8_t* Rx_data,uint8_t count){

	/* Variable used to alternate Rx and Tx during transfer */
	uint32_t txallowed = 1U;

	/*initialisation*/
	hSPIx->errorcode = SPI_ERROR_NONE;

	hSPIx->pTxbuffer   = (uint8_t*)Tx_data;
	hSPIx->TxXfersize  = count;
	hSPIx->TxXfercount = count;

	hSPIx->pRxbuffer   = (uint8_t*)Rx_data;
	hSPIx->RxXfersize  = count;
	hSPIx->RxXfercount = count;

	//Reset CRC
	CLEAR_BIT((hSPIx->Instance)->CR1,SPI_CR1_CRCEN);
	SET_BIT((hSPIx->Instance)->CR1,SPI_CR1_CRCEN);

	/* Check if the SPI is already enabled */
    if ((hSPIx->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
    {
	 /* Enable SPI peripheral */
	  SET_BIT((hSPIx->Instance->CR1),SPI_CR1_SPE);
    }

	/* Transmit and Receive data in 8 Bit mode */
	while((hSPIx->TxXfercount > 0U) || (hSPIx->RxXfercount > 0U)){
		/*check TXE flag is set or not*/
		delay_ms(2);
		if(LL_SPI_IsActiveFlag_TXE(hSPIx->Instance) && (hSPIx->TxXfercount > 0U) && (txallowed == 1U)){
			LL_SPI_TransmitData8(hSPIx->Instance, (*hSPIx->pTxbuffer));
			hSPIx->pTxbuffer+=sizeof(uint8_t);
			hSPIx->TxXfercount--;

			/* Next Data is a reception (Rx). Tx not allowed */
			txallowed = 0U;

			if((hSPIx->TxXfercount == 0U) && LL_SPI_IsEnabledCRC(hSPIx->Instance)){
				/* Enable CRC Transmission */
					LL_SPI_SetCRCNext(hSPIx->Instance);       //set next byte as CRC
			}
		}

		/* Wait until RXNE flag is set */
		delay_ms(2);
		if(LL_SPI_IsActiveFlag_RXNE(hSPIx->Instance) && (hSPIx->RxXfercount > 0U)){
			*(hSPIx->pRxbuffer) = LL_SPI_ReceiveData8(hSPIx->Instance);
		    hSPIx->pRxbuffer++;
			hSPIx->RxXfercount--;

			/* Next Data is a Transmission (Tx). Tx is allowed */
			txallowed = 1U;
		}

	}

	/* Read CRC from DR to close CRC calculation process */
	if(LL_SPI_IsEnabledCRC(hSPIx->Instance)){
		/* Wait until RXNE flag is set */
		delay_ms(2);
		if(!LL_SPI_IsActiveFlag_RXNE(hSPIx->Instance)){
			hSPIx->errorcode = SPI_ERROR_CRC;
			return;
		}
		LL_SPI_ReceiveData8(hSPIx->Instance);
		uint8_t crc_data = *((uint8_t*)&(hSPIx->Instance->DR));
		unused(crc_data);
	}

	/*check if received CRC was correct*/
	if(LL_SPI_IsActiveFlag_CRCERR(hSPIx->Instance)){
		hSPIx->errorcode = SPI_ERROR_CRC;
		LL_SPI_ClearFlag_CRCERR(hSPIx->Instance);
	}
}

