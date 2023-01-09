#include "Talkthrough.h"

//--------------------------------------------------------------------------//
// Function:	Init1836()													//
//																			//
// Description:	This function sets up the SPI port to configure the AD1836. //
//				The content of the array sCodec1836TxRegs is sent to the 	//
//				codec.														//
//--------------------------------------------------------------------------//
void Init1836(void)
{
	volatile int wait_reset;
	volatile int wait_dma_finish;

	// reset codec
	// set PF15 as output
	*pFIO0_DIR |= (1 << AD1836_RESET_bit); 
	ssync();
	// clear bit to enable AD1836
	*pFIO0_FLAG_S |= (1 << AD1836_RESET_bit);	
	ssync();

	// wait to recover from reset
	for (wait_reset=0; wait_reset<0xf000; wait_reset++);

	// Enable PF4
	*pSPI_FLG = FLS4;
	// Set baud rate SCK = HCLK/(2*SPIBAUD)	
	*pSPI_BAUD = 16;
	// configure spi port
	// SPI DMA write, 16-bit data, MSB first, SPI Master
	*pSPI_CTL = TIMOD_DMA_TX | SIZE | MSTR;
	
	// Set up DMA2 channel 4 to SPI transmit
	*pDMA2_4_PERIPHERAL_MAP = 0x4000;
	
	// Configure DMA2 channel4
	// 16-bit transfers
	*pDMA2_4_CONFIG = WDSIZE_16;
	// Start address of data buffer
	*pDMA2_4_START_ADDR = (void *)sCodec1836TxRegs;
	// DMA inner loop count
	*pDMA2_4_X_COUNT = CODEC_1836_REGS_LENGTH;
	// Inner loop address increment
	*pDMA2_4_X_MODIFY = 2;
	
	// enable DMAs
	*pDMA2_4_CONFIG = (*pDMA2_4_CONFIG | DMAEN);
	// enable spi
	*pSPI_CTL = (*pSPI_CTL | SPE);
	ssync();
	
	// wait until dma transfers for spi are finished 
	for (wait_dma_finish=0; wait_dma_finish<0xaff; wait_dma_finish++);
	
	// disable spi
	*pSPI_CTL = 0x0000;
}


//--------------------------------------------------------------------------//
// Function:	Init_Sport0													//
//																			//
// Description:	Configure Sport0 for TDM mode, to transmit/receive data 	//
//				to/from the AD1836. Configure Sport for external clocks and //
//				frame syncs.												//
//--------------------------------------------------------------------------//
void Init_Sport0(void)
{
	// Sport0 receive configuration
	// External CLK, External Frame sync, MSB first
	// 32-bit data
	*pSPORT0_RCR1 = RFSR;
	*pSPORT0_RCR2 = SLEN_32;
	
	// Sport0 transmit configuration
	// External CLK, External Frame sync, MSB first
	// 24-bit data
	*pSPORT0_TCR1 = TFSR;
	*pSPORT0_TCR2 = SLEN_32;
	
	// Enable MCM 8 transmit & receive channels
	*pSPORT0_MTCS0 = 0x000000FF;
	*pSPORT0_MRCS0 = 0x000000FF;
	
	// Set MCM configuration register and enable MCM mode
	*pSPORT0_MCMC1 = 0x0000;
	*pSPORT0_MCMC2 = 0x101c;
}


//--------------------------------------------------------------------------//
// Function:	Init_DMA													//
//																			//
// Description:	Initialize DMA2_0 in autobuffer mode to receive and DMA2_1 	//
//				in autobuffer mode to transmit								//
//--------------------------------------------------------------------------//
void Init_DMA(void)
{
	// Set up DMA2 channel 0 to Sport receive
	*pDMA2_0_PERIPHERAL_MAP = 0x0000;

	// Configure DMA2 channel0
	// 32-bit transfers, Interrupt on completion, Autobuffer mode
	*pDMA2_0_CONFIG = WNR | WDSIZE_32 | DI_EN | FLOW_1;
	// Start address of data buffer
	*pDMA2_0_START_ADDR = (void *)iRxBuffer1;
	// DMA inner loop count
	*pDMA2_0_X_COUNT = 8;
	// Inner loop address increment
	*pDMA2_0_X_MODIFY	= 4;
	
	
	// Set up DMA2 channel 1 to Sport transmit
	*pDMA2_1_PERIPHERAL_MAP = 0x1000;
	
	// Configure DMA2 channel 1
	// 32-bit transfers, Autobuffer mode
	*pDMA2_1_CONFIG = WDSIZE_32 | FLOW_1;
	// Start address of data buffer
	*pDMA2_1_START_ADDR = (void *)iTxBuffer1;
	// DMA inner loop count
	*pDMA2_1_X_COUNT = 8;
	// Inner loop address increment
	*pDMA2_1_X_MODIFY	= 4;
}


//--------------------------------------------------------------------------//
// Function:	Init_Interrupts												//
//																			//
// Description:	Initialize Interrupt for Sport0 RX							//
//--------------------------------------------------------------------------//
void Init_Sport_Interrupts(void)
{
	// assign interrupt channel 23 (DMA2_0) to IVG9 
	*pSICA_IAR2 = Peripheral_IVG(23,9);	
	
	// assign ISRs to interrupt vectors
	// Sport0 RX ISR -> IVG 9
	register_handler(ik_ivg9, Sport0_RX_ISR);		

	// clear pending IVG9 interrupts
	*pILAT |= EVT_IVG9;		
	ssync();
	
	// enable Sport0 RX interrupt
	*pSICA_IMASK0 |= SIC_MASK(23);
	ssync();
	
}


//--------------------------------------------------------------------------//
// Function:	Enable_DMA_Sport											//
//																			//
// Description:	Enable DMA2_0, DMA2_1, Sport0 TX and Sport0 RX					//
//--------------------------------------------------------------------------//
void Enable_DMA_Sport0(void)
{
	// enable DMAs
	*pDMA2_1_CONFIG	= (*pDMA2_1_CONFIG | DMAEN);
	*pDMA2_0_CONFIG	= (*pDMA2_0_CONFIG | DMAEN);
	
	// enable Sport0 TX and RX
	*pSPORT0_TCR1 	= (*pSPORT0_TCR1 | TSPEN);
	*pSPORT0_RCR1 	= (*pSPORT0_RCR1 | RSPEN);
}

