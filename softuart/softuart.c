/*
 Software Uart For Stm32
 By Liyanboy74
 https://github.com/liyanboy74
*/

#include "softuart.h"

// Some internal define
#if(SoftUart_PARITY)
#define SoftUart_IDEF_LEN_C1 (SoftUart_DATA_LEN+2)
#else
#define SoftUart_IDEF_LEN_C1 (SoftUart_DATA_LEN+1)
#endif
#define SoftUart_IDEF_LEN_C2 (SoftUart_IDEF_LEN_C1 + SoftUart_STOP_Bit)

// All Soft Uart Config and State
SoftUart_S       	SUart   [Number_Of_SoftUarts];

// TX RX Data Buffer
SoftUartBuffer_S 	SUBuffer[Number_Of_SoftUarts];

// For timing division
__IO  uint8_t 		SU_Timer=0;

// Parity var
static uint8_t DV,PCount;

// Read RX single Pin Value
GPIO_PinState SoftUartGpioReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	return HAL_GPIO_ReadPin(GPIOx,GPIO_Pin);
}

// Write TX single Pin Value
void SoftUartGpioWritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
	HAL_GPIO_WritePin(GPIOx,GPIO_Pin,PinState);
}

// Initial Soft Uart
SoftUartState_E SoftUartInit(uint8_t SoftUartNumber,GPIO_TypeDef *TxPort,uint16_t TxPin,GPIO_TypeDef *RxPort,uint16_t RxPin)
{
	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;

	SUart[SoftUartNumber].TxNComplated=0;

	SUart[SoftUartNumber].RxBitCounter=0;
	SUart[SoftUartNumber].RxBitShift=0;
	SUart[SoftUartNumber].RxIndex=0;

	SUart[SoftUartNumber].TxEnable=0;
	SUart[SoftUartNumber].RxEnable=0;

	SUart[SoftUartNumber].TxBitCounter=0;
	SUart[SoftUartNumber].TxBitShift=0;
	SUart[SoftUartNumber].TxIndex=0;

	SUart[SoftUartNumber].TxSize=0;

	SUart[SoftUartNumber].Buffer=&SUBuffer[SoftUartNumber];

	SUart[SoftUartNumber].RxPort=RxPort;
	SUart[SoftUartNumber].RxPin=RxPin;

	SUart[SoftUartNumber].TxPort=TxPort;
	SUart[SoftUartNumber].TxPin=TxPin;

	SUart[SoftUartNumber].RxTimingFlag=0;
	SUart[SoftUartNumber].RxBitOffset=0;

	return SoftUart_OK;
}

// Send one bit to TX pin
void SoftUartTransmitBit(SoftUart_S *SU,uint8_t Bit0_1)
{
	SoftUartGpioWritePin(SU->TxPort,SU->TxPin,(GPIO_PinState)Bit0_1);
}

// Enable Soft Uart Receiving
SoftUartState_E SoftUartEnableRx(uint8_t SoftUartNumber)
{
	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;
	SUart[SoftUartNumber].RxEnable=1;
	return SoftUart_OK;
}

// Disable Soft Uart Receiving
SoftUartState_E SoftUartDisableRx(uint8_t SoftUartNumber)
{
	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;
	SUart[SoftUartNumber].RxEnable=0;
	return SoftUart_OK;
}

// Read Size of Received Data in buffer
uint8_t SoftUartRxAlavailable(uint8_t SoftUartNumber)
{
	return SUart[SoftUartNumber].RxIndex;
}



// Move Received Data to Another Buffer
SoftUartState_E SoftUartReadRxBuffer(uint8_t SoftUartNumber,uint8_t *Buffer,uint8_t Len)
{
	int i;
	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;
	for(i=0;i<Len;i++)
	{
		Buffer[i]=SUart[SoftUartNumber].Buffer->Rx[i];
	}
	for(i=0;i<SUart[SoftUartNumber].RxIndex;i++)
	{
		SUart[SoftUartNumber].Buffer->Rx[i]=SUart[SoftUartNumber].Buffer->Rx[i+Len];
	}
	SUart[SoftUartNumber].RxIndex-=Len;
	return SoftUart_OK;
}

// Soft Uart Transmit Data Process
void SoftUartTxProcess(SoftUart_S *SU)
{
	if(SU->TxEnable)
	{
		// Start
		if(SU->TxBitCounter==0)
		{
			SU->TxNComplated=1;
			SU->TxBitShift=0;
			SoftUartTransmitBit(SU,0);
			SU->TxBitCounter++;
			PCount=0;
		}
		// Data
		else if(SU->TxBitCounter<(SoftUart_DATA_LEN+1))
		{
			DV=((SU->Buffer->Tx[SU->TxTail])>>(SU->TxBitShift))&0x01;
			SoftUartTransmitBit(SU,DV);
			SU->TxBitCounter++;
			SU->TxBitShift++;

			if(DV)PCount++;
		}
		// Parity
		else if(SU->TxBitCounter<SoftUart_IDEF_LEN_C1)
		{
			// Check Even or Odd
			DV=PCount%2;

			// if Odd Parity
			if(SoftUart_PARITY==1)DV=!DV;

			SoftUartTransmitBit(SU,DV);
			SU->TxBitCounter++;
		}
		// Stop
		else if(SU->TxBitCounter<SoftUart_IDEF_LEN_C2)
		{
			SoftUartTransmitBit(SU,1);
			SU->TxBitCounter++;
		}
		//Complete
		else if(SU->TxBitCounter==SoftUart_IDEF_LEN_C2)
		{
			uint16_t tail;

			// Reset Bit Counter
			SU->TxBitCounter=0;

			// Byte has been shifted out: advance the ring tail
			tail = SU->TxTail + 1;
			if (tail >= SoftUartTxBufferSizeTX) tail = 0;
			SU->TxTail = tail;

			// Ready To Send Another Data
			if(SU->TxTail != SU->TxHead)
			{
				// Continue Sending
				SU->TxNComplated=1;
				SU->TxEnable=1;
			}
			else
			{
				// Finish
				SU->TxNComplated=0;
				SU->TxEnable=0;
			}
		}
	}
}

// Soft Uart Receive Data Process
void SoftUartRxDataBitProcess(SoftUart_S *SU,uint8_t B0_1)
{
	if(SU->RxEnable)
	{
		// Start
		if(SU->RxBitCounter==0)
		{
			// Start Bit is 0
			if(B0_1)return;

			SU->RxBitShift=0;
			SU->RxBitCounter++;
			SU->Buffer->Rx[SU->RxIndex]=0;
		}
		// Data
		else if(SU->RxBitCounter<(SoftUart_DATA_LEN+1))
		{
			SU->Buffer->Rx[SU->RxIndex]|=((B0_1&0x01)<<SU->RxBitShift);
			SU->RxBitCounter++;
			SU->RxBitShift++;
		}
		// Parity
		else if(SU->RxBitCounter<SoftUart_IDEF_LEN_C1)
		{
			// Need to be check
			// B0_1;
			SU->RxBitCounter++;
		}
		// Stop & Complete
		else if(SU->RxBitCounter<SoftUart_IDEF_LEN_C2)
		{
			SU->RxBitCounter=0;
			SU->RxTimingFlag=0;

			//Stop Bit must be 1
			if(B0_1)
			{
				// Received successfully
				// Change RX Buffer Index
				if((SU->RxIndex)<(SoftUartRxBufferSizeRX-1))(SU->RxIndex)++;
			}
			// if not : ERROR -> Overwrite data
		}
	}
}

// Wait Until Transmit Completed
// You do not usually need to use this function!
void SoftUartWaitUntilTxComplate(uint8_t SoftUartNumber)
{
	while(SUart[SoftUartNumber].TxNComplated);
}

uint8_t SoftUartGetTxNComplated(uint8_t SoftUartNumber)
{
	return SUart[SoftUartNumber].TxNComplated;
}

// Copy Data to Transmit Buffer and Start Sending
SoftUartState_E SoftUartPuts(uint8_t SoftUartNumber,uint8_t *Data,uint8_t Len)
{
/*

	int i;

	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;
	if (SUart[SoftUartNumber].TxNComplated) {

		SUart[SoftUartNumber].TxSize += Len;

		for(i=0;i<Len;i++)
		{
			SUart[SoftUartNumber].Buffer->Tx[SUart[SoftUartNumber].TxSize + i]= Data[i];
		}

		return SoftUart_OK;
	}

	SUart[SoftUartNumber].TxIndex=0;
	SUart[SoftUartNumber].TxSize=Len;

	for(i=0;i<Len;i++)
	{
		SUart[SoftUartNumber].Buffer->Tx[i]= Data[i];
	}

	SUart[SoftUartNumber].TxNComplated=1;
	SUart[SoftUartNumber].TxEnable=1;

	return SoftUart_OK;

*/

	if(SoftUartNumber>=Number_Of_SoftUarts)return SoftUart_Error;

	/* the old single frame buffer is replaced by the TX ring buffer */
	return SoftUartPutsRing(SoftUartNumber, Data, Len);
}

/* -------------------------------------------------------------------------
 * Non blocking transmit.
 *
 * Appends Len bytes to the TX ring buffer and returns at once. The bytes are
 * shifted out by SoftUartTxProcess() from the TIM7 interrupt, one bit every
 * 5 interrupt calls (9600 baud), so the caller is never blocked.
 *
 * Single producer (main loop) / single consumer (TIM7 ISR):
 *   - producer writes Buffer->Tx[head..] and publishes TxHead,
 *   - consumer reads Buffer->Tx[Tail] and advances TxTail.
 * The head publication and the "start transmitter" decision are done with
 * interrupts disabled (a few instructions) to avoid a lost wakeup when the
 * ISR finishes the last byte exactly while the producer pushes a new one.
 * ---------------------------------------------------------------------- */
SoftUartState_E SoftUartPutsRing(uint8_t SoftUartNumber, const uint8_t *Data, uint16_t Len)
{
	SoftUart_S *SU;
	uint16_t i, head, next;
	uint32_t primask;

	if (SoftUartNumber >= Number_Of_SoftUarts) return SoftUart_Error;
	if (Data == 0 || Len == 0) return SoftUart_Error;

	SU = &SUart[SoftUartNumber];

	head = SU->TxHead;
	for (i = 0; i < Len; i++)
	{
		next = head + 1;
		if (next >= SoftUartTxBufferSizeTX) next = 0;
		if (next == SU->TxTail)					/* ring is full */
		{
			SU->TxDropped += (uint32_t)(Len - i);
			break;
		}
		SU->Buffer->Tx[head] = Data[i];
		head = next;
	}

	primask = __get_PRIMASK();
	__disable_irq();
	SU->TxHead = head;
	if (!SU->TxEnable)							/* transmitter was idle - start it */
	{
		SU->TxBitCounter = 0;
		SU->TxBitShift   = 0;
		SU->TxNComplated = 1;
		SU->TxEnable     = 1;					/* published last */
	}
	if (!primask) __enable_irq();

	return (i == Len) ? SoftUart_OK : SoftUart_Error;
}

uint32_t SoftUartGetTxDropped(uint8_t SoftUartNumber)
{
	if (SoftUartNumber >= Number_Of_SoftUarts) return 0;
	return SUart[SoftUartNumber].TxDropped;
}

// Capture RX and Get BitOffset
uint8_t SoftUartScanRxPorts(void)
{
	int i;
	uint8_t Buffer=0x00,Bit;

	for(i=0;i<Number_Of_SoftUarts;i++)
	{
		// Read RX GPIO Value
		Bit=SoftUartGpioReadPin(SUart[i].RxPort,SUart[i].RxPin);

		// Starting conditions
		if(!SUart[i].RxBitCounter && !SUart[i].RxTimingFlag && !Bit)
		{
			// Save RX Bit Offset
			// Calculate middle position of data puls
			SUart[i].RxBitOffset=((SU_Timer+2)%5);

			// Timing Offset is Set
			SUart[i].RxTimingFlag=1;
		}

		// Add all RX GPIO State to Buffer
		Buffer|=((Bit&0x01)<<i);
	}
	return Buffer;
}

// SoftUartHandler must call in interrupt every 0.2*(1/BR)
// if BR=9600 then 0.2*(1/9600)=20.8333333 uS
void SoftUartHandler(void)
{
	int     	i;
	uint8_t 	SU_DBuffer;

	// Capture RX and Get BitOffset
	SU_DBuffer = SoftUartScanRxPorts();

	for(i=0;i < Number_Of_SoftUarts;i++)
	{
		// Receive Data if we in middle data pulse position
		if(SUart[i].RxBitOffset == SU_Timer)
		{
			SoftUartRxDataBitProcess(&SUart[i],((SU_DBuffer>>i)&0x01));
		}
	}

	// Sending always happens in the first time slot
	if(SU_Timer==0)
	{
		// Transmit Data
		for(i=0;i < Number_Of_SoftUarts;i++)
		{
			SoftUartTxProcess(&SUart[i]);
		}
	}

	// Timing process
	SU_Timer++;
	if(SU_Timer >= 5)SU_Timer=0;
}
