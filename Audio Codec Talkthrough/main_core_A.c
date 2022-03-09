//--------------------------------------------------------------------------//
//																			//
//	 Name: 	Talkthrough for the ADSP-BF561 EZ-KIT Lite						//
//																			//
//--------------------------------------------------------------------------//
//																			//
//	(C) Copyright 2003 - Analog Devices, Inc.  All rights reserved.			//
//																			//
//	Project Name:	BF561 C Talkthrough TDM									//
//																			//
//	Date Modified:	16/10/03		HD		Rev 0.2							//
//																			//
//	Software:		VisualDSP++3.5											//
//																			//
//	Hardware:		ADSP-BF561 EZ-KIT Board									//
//																			//
//	Connections:	Dipswitch SW4 : set #6 to "on"							//
//					Dipswitch SW4 : set #5 to "off"							//
//					Connect an input source (such as a radio) to the Audio	//
//					input jack and an output source (such as headphones) to //
//					the Audio output jack									//
//																			//
//	Purpose:		This program sets up the SPI port on the ADSP-BF561 to  //
//					configure the AD1836 codec.  The SPI port is disabled 	//
//					after initialization.  The data to/from the codec are 	//
//					transfered over SPORT0 in TDM mode						//
//																			//
//--------------------------------------------------------------------------//

#include "Talkthrough.h"

//--------------------------------------------------------------------------//
// Variables																//
//																			//
// Description:	The variables iChannelxLeftIn and iChannelxRightIn contain 	//
//				the data coming from the codec AD1836.  The (processed)		//
//				playback data are written into the variables 				//
//				iChannelxLeftOut and iChannelxRightOut respectively, which 	//
//				are then sent back to the codec in the SPORT0 ISR.  		//
//				The values in the array iCodec1836TxRegs can be modified to //
//				set up the codec in different configurations according to   //
//				the AD1836 data sheet.										//
//--------------------------------------------------------------------------//
// left input data from ad1836
int iChannel0LeftIn, iChannel1LeftIn;
// right input data from ad1836
int iChannel0RightIn, iChannel1RightIn;
// left ouput data for ad1836	
int iChannel0LeftOut, iChannel1LeftOut;
// right ouput data for ad1836
int iChannel0RightOut, iChannel1RightOut;
// array for registers to configure the ad1836
// names are defined in "Talkthrough.h"
volatile short sCodec1836TxRegs[CODEC_1836_REGS_LENGTH] =
{									
					DAC_CONTROL_1	| 0x000,
					DAC_CONTROL_2	| 0x000,
					DAC_VOLUME_0	| 0x3ff,
					DAC_VOLUME_1	| 0x3ff,
					DAC_VOLUME_2	| 0x3ff,
					DAC_VOLUME_3	| 0x3ff,
					DAC_VOLUME_4	| 0x3ff,
					DAC_VOLUME_5	| 0x3ff,
					ADC_CONTROL_1	| 0x000,
					ADC_CONTROL_2	| 0x180,
					ADC_CONTROL_3	| 0x000
					
};
// SPORT0 DMA transmit buffer
volatile int iTxBuffer1[8];
// SPORT0 DMA receive buffer
volatile int iRxBuffer1[8];


//--------------------------------------------------------------------------//
// Function:	main														//
//																			//
// Description:	After calling a few initalization routines, main() just 	//
//				waits in a loop forever.  The code to process the incoming  //
//				data can be placed in the function Process_Data() in the 	//
//				file "Process_Data.c".										//
//--------------------------------------------------------------------------//
void main(void)
{

	// unblock Core B if dual core operation is desired	
#ifndef RUN_ON_SINGLE_CORE	// see talkthrough.h
	*pSICA_SYSCR &= 0xFFDF; // clear bit 5 to unlock  
#endif

 	Init1836();
	Init_Sport0();
	Init_DMA();
	Init_Sport_Interrupts();
	Enable_DMA_Sport0();
	
	while(1);
//     32-bit ALU/MAC  used   1 out of    5 ( 20.0%)
//     Group 1         used   1 out of    5 ( 20.0%)
//     Group 1 or 2    used   2 out of   10 ( 20.0%)
//     Store           used   1 out of    5 ( 20.0%)
//     32-bit ALU/MAC  used   1 out of    5 ( 20.0%)
//     Group 1         used   1 out of    5 ( 20.0%)
//     Group 1 or 2    used   2 out of   10 ( 20.0%)
//     Store           used   1 out of    5 ( 20.0%)
}
