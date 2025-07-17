/*
 * IRQ_Handler.c
 *
 *  Created on: Jul 17, 2025
 *      Author: karl.yamashita
 */


#include "main.h"


void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	int i = 0;
	uint32_t size = 0;

	if(huart == uart2_msg.huart)
	{
		size = UART_DMA_GetSize(&uart2_msg, Size);

		for(i = 0; i < size; i++)
		{
			uart2_msg.dma.circularBuffer[uart2_msg.dma.circularPtr.index_IN] = uart2_msg.dma.dma_data[uart2_msg.dma.dmaPtr.index_IN];
			RingBuff_Ptr_Input(&uart2_msg.dma.circularPtr, UART_DMA_CIRCULAR_SIZE);
			RingBuff_Ptr_Input(&uart2_msg.dma.dmaPtr, UART_DMA_BUFFER_SIZE);
		}
	}
	// repeat for other UART ports using (else if)
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == uart2_msg.huart)
	{
		uart2_msg.tx.txPending = false;
		UART_DMA_SendMessage(&uart2_msg);
	}
	// repeat for other UART ports using (else if)
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	GPIO_EXTI_Handler(GPIO_Pin);
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	GPIO_EXTI_Handler(GPIO_Pin);
}
