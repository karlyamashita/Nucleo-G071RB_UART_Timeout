/*
 * PollingRoutine.c
 *
 *  Created on: Oct 24, 2023
 *      Author: karl.yamashita
 *
 *
 *      Template for projects.
 *
 *      The object of this PollingRoutine.c/h files is to not have to write code in main.c which already has a lot of generated code.
 *      It is cumbersome having to scroll through all the generated code for your own code and having to find a USER CODE section so your code is not erased when CubeMX re-generates code.
 *      
 *      Direction: Call PollingInit before the main while loop. Call PollingRoutine from within the main while loop
 * 
 *      Example;
        // USER CODE BEGIN WHILE
        PollingInit();
        while (1)
        {
            PollingRoutine();
            // USER CODE END WHILE

            // USER CODE BEGIN 3
        }
        // USER CODE END 3

 */


#include "main.h"


const char version[] = "v1.0.1";

#define UART2_DMA_RX_QUEUE_SIZE 4 // queue size
#define UART2_DMA_TX_QUEUE_SIZE 4
UART_DMA_Data uart2_dmaDataRxQueue[UART2_DMA_RX_QUEUE_SIZE] = {0};
UART_DMA_Data uart2_dmaDataTxQueue[UART2_DMA_TX_QUEUE_SIZE] = {0};

UART_DMA_Struct_t uart2_msg =
{
	.huart = &huart2,
	.rx.queueSize = UART2_DMA_RX_QUEUE_SIZE,
	.rx.msgQueue = uart2_dmaDataRxQueue,
	.tx.queueSize = UART2_DMA_TX_QUEUE_SIZE,
	.tx.msgQueue = uart2_dmaDataTxQueue,
	.dma.dmaPtr.SkipOverFlow = true
};

char msg_copy_command[UART_DMA_QUEUE_DATA_SIZE] = {0};
char str[UART_DMA_QUEUE_DATA_SIZE] = {0};
uint32_t countDown;

void PollingInit(void)
{
    UART_DMA_EnableRxInterruptIdle(&uart2_msg);

    // register callbacks
    TimerCallbackRegisterOnly(&timerCallback, CountDown);
    TimerCallbackRegisterOnly(&timerCallback, ButtonDebounced);

    // toggle LED to indicate STM32 is alive
    TimerCallbackRegisterOnly(&timerCallback, LED_Toggle);
    TimerCallbackTimerStart(&timerCallback, LED_Toggle, 500, TIMER_REPEAT);

    STM32_Ready(&uart2_msg);
}

void PollingRoutine(void)
{
    TimerCallbackPoll(&timerCallback);

	UART_DMA_ParseCircularBuffer(&uart2_msg);

	UART_ParseCommands(&uart2_msg);
}

void UART_ParseCommands(UART_DMA_Struct_t *msg)
{
	int status = 0;
	char *ptr;
	char retStr[UART_DMA_QUEUE_DATA_SIZE] = "OK";

	if(UART_DMA_RxMsgRdy(msg))
	{
		memset(&msg_copy_command, 0, sizeof(msg_copy_command));// clear
		memcpy(&msg_copy_command, msg->rx.msgToParse->data, strlen((char*)msg->rx.msgToParse->data) - 2); // remove CR/LF

		ptr = (char*)msg->rx.msgToParse->data;
		RemoveSpaces(ptr);
		ToLower(ptr);

		if(strncmp(ptr, "version", strlen("version"))== 0)
		{
			sprintf(retStr, "%s", version);
		}
		else if(strncmp(ptr, "hello", strlen("hello"))== 0)
		{
			status = ReceivedHello();
		}
		else
		{
			status = COMMAND_UNKNOWN;
		}

		// check return status
		if(status == NO_ACK)
		{
			return;
		}
		else if(status != 0) // other return status other than NO_ACK or NO_ERROR
		{
			UART_DMA_PrintError(msg, msg_copy_command, status);
		}
		else // NO_ERROR
		{
			UART_DMA_PrintReply(msg, msg_copy_command, retStr);
		}

		memset(&msg->rx.msgToParse->data, 0, UART_DMA_QUEUE_DATA_SIZE); // clear current buffer index

	}
}

void STM32_Ready(UART_DMA_Struct_t *msg)
{
	sprintf(str, "STM32 Ready - %s", version);

	UART_DMA_NotifyUser(msg, str, strlen(str), true);
}

void LED_Toggle(void)
{
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
}

// we received hello, so disable timer callback and print Good message
int ReceivedHello()
{
	TimerCallbackDisable(&timerCallback, CountDown);

	sprintf(str, "Good! You're there!");
	UART_DMA_NotifyUser(&uart2_msg, str, strlen(str), true);

	return NO_ACK;
}

// start counting down and printing to console
void CountDown(void)
{
	if(!countDown)
	{
		// countDown reached 0, so print error
		TimerCallbackDisable(&timerCallback, CountDown);
		UART_ResponseFailed();
	}
	else
	{
		// print count down
		sprintf(str, "Count down: %ld", countDown);
		UART_DMA_NotifyUser(&uart2_msg, str, strlen(str), true);
	}
	countDown--;
}

/* Description: User can do what is needed if the other device fails to respond.
 *		For this demo, we're just writing a failed message.
 */
void UART_ResponseFailed(void)
{
	sprintf(str, "Failed to get reply!");

	UART_DMA_NotifyUser(&uart2_msg, str, strlen(str), true);
}

// Button has been debounced so send inquiry to other device
void ButtonDebounced(void)
{
	sprintf(str, "are you there?");
	UART_DMA_NotifyUser(&uart2_msg, str, strlen(str), true);

	// init to start counting from 5
	countDown = 5;
	// init count down timer to print every 1 second
	TimerCallbackTimerStart(&timerCallback, CountDown, 1000, TIMER_REPEAT);
}

