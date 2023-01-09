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
#include "Triangulation.h"
#include <filter.h>
#include <math.h>
#include <fract2float_conv.h>
//#include <btc.h>
//#include <btc_struct.h>

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
					ADC_CONTROL_3	| 0x080 //Set sample rate to 32 kHz
					
};
// SPORT0 DMA transmit buffer
volatile int iTxBuffer1[8];
// SPORT0 DMA receive buffer
volatile int iRxBuffer1[8];

//The delay lines for each of the mic inputs
//Is used in this program and modified by the isr
fract16 AdelayLine[LIFETIME] = {0};
fract16 BdelayLine[LIFETIME] = {0};
fract16 CdelayLine[LIFETIME] = {0};

//Needed for the FFT calculation, holds sines and cosines
//Should only be modified once at the beginning of the program
complex_fract16 twiddle_table[LIFETIME];

//These will hold the outputs from the FFT
complex_fract16 AFrequencyDomain[LIFETIME];
complex_fract16 BFrequencyDomain[LIFETIME];
complex_fract16 CFrequencyDomain[LIFETIME];

//These will hold the part of the FFT we actually care about
//Global for now to be plottable
float ATransform[LIFETIME];
float BTransform[LIFETIME];
float CTransform[LIFETIME];	
int omegaAxis[LIFETIME];

time_t sysTime;

float xPoints[MAXSOURCES] = {0.0};
float yPoints[MAXSOURCES] = {0.0};

//Microphone coordinates, cm currently
const float bx = -19;
const float by = 22.98;
const float cx = 29.13;
const float cy = 10.603;

extern bool isData;

//Background Telemetry Channel setup for live plotting
//float BTC_CHAN0[LIFETIME];
//BTC_MAP_BEGIN
//BTC_MAP_ENTRY("A Transform", (long)&BTC_CHAN0,sizeof(BTC_CHAN0))
//BTC_MAP_END

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
	//btc_init();
	
	twidfftrad2_fr16(twiddle_table, LIFETIME); //do the initial calculations for the table, only needs to be done once
	
	static signalSource sources[MAXSOURCES];//Avoiding dynamic memory allocation on an embedded processor
	
	int i = 0;
	for(i = 0; i < LIFETIME; i++)
		omegaAxis[i] = i+1;
	
	int historyCounter = 0;
	unsigned int count = 0;	
	clock_t time_begin;
	clock_t curr_time;
	time_begin = clock();
	curr_time = clock();
	//while(true){
	while((((float)curr_time - (float)time_begin)/(float)CLOCKS_PER_SEC) < 10){
		if(isData){
			/*
			 * FREQUENCY TRANSFORMATION
			 */
		 
			 //use rfft_fr16, real fract input and outputs complex fract, this may need to be a critical section
			 //__asm__("CLI R0 ;\n");	 
			 //ssync();
			 rfft_fr16(AdelayLine, AFrequencyDomain, twiddle_table, 2, LIFETIME, 0, 0);
			 rfft_fr16(BdelayLine, BFrequencyDomain, twiddle_table, 2, LIFETIME, 0, 0);
			 rfft_fr16(CdelayLine, CFrequencyDomain, twiddle_table, 2, LIFETIME, 0, 0);
			 //__asm__("STI R0 ;\n");	 
			 //ssync();
	 	 
			 //Get strictly the part we care about
		 	 int k = 0;
		 	 for(k = 0; k < LIFETIME; k++){
		 	 	ATransform[k] = sqrt(pow(fr16_to_float(AFrequencyDomain[k].re), 2) + pow(fr16_to_float(AFrequencyDomain[k].im), 2));
		 	 	BTransform[k] = sqrt(pow(fr16_to_float(BFrequencyDomain[k].re), 2) + pow(fr16_to_float(BFrequencyDomain[k].im), 2));
		 	 	CTransform[k] = sqrt(pow(fr16_to_float(CFrequencyDomain[k].re), 2) + pow(fr16_to_float(CFrequencyDomain[k].im), 2));
		 	 }
	 	 
			/*
			 * SIGNAL SOURCE DETECTION AND UPDATES
			 */
		 
			 //find the sources in the current iteration
			 int maxFreqA = 0;
			 int maxFreqB = 0;
			 int maxFreqC = 0;
		 
			 for(k = LOWERBOUND; k < LIFETIME; k++){
			 	maxFreqA = (ATransform[k] > ATransform[maxFreqA]) ? k : maxFreqA;
			 	maxFreqB = (BTransform[k] > ATransform[maxFreqB]) ? k : maxFreqB;
			 	maxFreqC = (CTransform[k] > ATransform[maxFreqC]) ? k : maxFreqC;
			 }
		 
			 if(ATransform[maxFreqA] < THRESHOLD)
			 	maxFreqA = 0;
		 	
			 if(ATransform[maxFreqB] < THRESHOLD)
			 	maxFreqB = 0;
		 	
			 if(ATransform[maxFreqC] < THRESHOLD)
			 	maxFreqC = 0;
		 
			 bool aExists = false;
			 if(maxFreqA != 0){
				 for (k = 0; k < MAXSOURCES; k++){
				 	if((maxFreqA >= (sources[k].mainFrequency-DETECTIONWIDTH)) && (maxFreqA <= (sources[k].mainFrequency+DETECTIONWIDTH))){
				 		aExists = true;
				 		if(!sources[k].isAknown){
				 			sources[k].timeA = clock();
				 			sources[k].isAknown = true;
				 		}
				 	}	
				 }
			 }
			 else
			 	aExists = true;
			 
			 bool bExists = false;
			 if(maxFreqB != 0){
				 for (k = 0; k < MAXSOURCES; k++){
				 	if((maxFreqB >= (sources[k].mainFrequency-DETECTIONWIDTH)) && (maxFreqB <= (sources[k].mainFrequency+DETECTIONWIDTH))){
				 		bExists = true;
				 		if(!sources[k].isBknown){
				 			sources[k].timeB = clock();
				 			sources[k].isBknown = true;
				 		}
				 	}	
				 }
			 }
			 else
			 	bExists = true;
			 
			 bool cExists = false;
			 if(maxFreqC != 0){
				 for (k = 0; k < MAXSOURCES; k++){
				 	if((maxFreqC >= (sources[k].mainFrequency-DETECTIONWIDTH)) && (maxFreqC <= (sources[k].mainFrequency+DETECTIONWIDTH))){
				 		aExists = true;
				 		if(!sources[k].isCknown){
				 			sources[k].timeC = clock();
				 			sources[k].isCknown = true;
				 		}
				 	}	
				 }
			 }
			 else
			 	cExists = true;
			 	
			 if(!aExists){
			 		int place = 0;
			 		while(sources[place].isSource && (place < MAXSOURCES))
			 			++place;
			 		if(place >= MAXSOURCES){
			 			sources[0].timeA = clock();
			 			sources[0].lifetime = clock();
			 			sources[0].mainFrequency = maxFreqA;
			 			sources[0].isAknown = true;
			 			sources[0].isSource = true;
			 		}
			 		else{
			 			sources[place].timeA = clock();
			 			sources[place].lifetime = clock();
			 			sources[place].mainFrequency = maxFreqA;
			 			sources[place].isAknown = true;
			 			sources[place].isSource = true;
			 		}
			 }
			 
			 if(!bExists){
			 		int place = 0;
			 		while(sources[place].isSource && (place < MAXSOURCES))
			 			++place;
			 		if(place >= MAXSOURCES){
			 			sources[0].timeB = clock();
			 			sources[0].lifetime = clock();
			 			sources[0].mainFrequency = maxFreqB;
			 			sources[0].isBknown = true;
			 			sources[0].isSource = true;
			 		}
			 		else{
			 			sources[place].timeB = clock();
			 			sources[place].lifetime = clock();
			 			sources[place].mainFrequency = maxFreqB;
			 			sources[place].isBknown = true;
			 			sources[place].isSource = true;
			 		}
			 }
			 
			 if(!cExists){
			 		int place = 0;
			 		while(sources[place].isSource && (place < MAXSOURCES))
			 			++place;
			 		if(place >= MAXSOURCES){
			 			sources[0].timeC = clock();
			 			sources[0].lifetime = clock();
			 			sources[0].mainFrequency = maxFreqC;
			 			sources[0].isCknown = true;
			 			sources[0].isSource = true;
			 		}
			 		else{
			 			sources[place].timeC = clock();
			 			sources[place].lifetime = clock();
			 			sources[place].mainFrequency = maxFreqC;
			 			sources[place].isCknown = true;
			 			sources[place].isSource = true;
			 		}
			 }
			 
			 for(k = 0; k < MAXSOURCES; k++){
			 	if(sources[k].isAknown && sources[k].isBknown && sources[k].isCknown){
			 		sources[k].lifetime = time(0);
			 		//C = cx^2 + cy^2 - cs^2 - 660cs*r
					sources[k].C = pow(cx, 2) + pow(cy, 2) -pow((float)(((float)sources[k].timeC - (float)sources[k].timeA))/(float)CLOCKS_PER_SEC, 2) - 660*(((float)sources[k].timeC - (float)sources[k].timeA)/(float)CLOCKS_PER_SEC)*INIT_RADIUS;
			 		//B = bx^2 + by^2  - bs^2 - 660bs*r
					sources[k].B = pow(bx, 2) + pow(by, 2) - pow(((float)sources[k].timeB - (float)sources[k].timeA)/(float)CLOCKS_PER_SEC, 2) - 660*(((float)sources[k].timeB - (float)sources[k].timeA)/(float)CLOCKS_PER_SEC)*INIT_RADIUS;
				//	sources[k].isLocked = true;
			 	}	
			 }
			 
			 //Compare with the currently known sources and set accordingly
			/* for(k = 0; k < MAXSOURCES; k++){
			 	if((maxFreqA >= (sources[k].mainFrequency-DETECTIONWIDTH)) && (maxFreqA <= (sources[k].mainFrequency+DETECTIONWIDTH)) && !sources[k].isAknown){
			 		sources[k].timeA = time(0);
			 		sources[k].isAknown = true;
			 	}else{
			 		int place = 0;
			 		while(sources[place].isSource && (place < MAXSOURCES))
			 			++place;
			 		if(place >= MAXSOURCES){
			 			sources[0].timeA = time(0);
			 			sources[0].lifetime = time(0);
			 			sources[0].mainFrequency = maxFreqA;
			 			sources[0].isAknown = true;
			 			sources[0].isSource = true;
			 		}
			 		else{
			 			sources[place].timeA = time(0);
			 			sources[place].lifetime = time(0);
			 			sources[place].mainFrequency = maxFreqA;
			 			sources[place].isAknown = true;
			 			sources[place].isSource = true;
			 		}
			 	}
			 	if((maxFreqB >= (sources[k].mainFrequency-5)) && (maxFreqB <= (sources[k].mainFrequency+5)) && !sources[k].isBknown){
			 		sources[k].timeB = time(0);
			 		sources[k].isBknown = true;
			 	}
			 	else{
			 		int place = 0;
			 		while(sources[place].isSource && (place < MAXSOURCES))
			 			++place;
			 		if(place >= MAXSOURCES){
			 			sources[0].timeB = time(0);
			 			sources[0].lifetime = time(0);
			 			sources[0].mainFrequency = maxFreqB;
			 			sources[0].isBknown = true;
			 			sources[0].isSource = true;
			 		}
			 		else{
			 			sources[place].timeB = time(0);
			 			sources[place].lifetime = time(0);
			 			sources[place].mainFrequency = maxFreqB;
			 			sources[place].isBknown = true;
			 			sources[place].isSource = true;
			 		}
			 	}
			 	if((maxFreqC >= (sources[k].mainFrequency-5)) && (maxFreqC <= (sources[k].mainFrequency+5)) && !sources[k].isCknown){
			 		sources[k].timeC = time(0);
			 		sources[k].isCknown = true;
			 	}
			 	else{
			 		int place = 0;
			 		while(sources[place].isSource && (place < MAXSOURCES))
			 			++place;
			 		if(place >= MAXSOURCES){
			 			sources[0].timeC = time(0);
			 			sources[0].lifetime = time(0);
			 			sources[0].mainFrequency = maxFreqC;
			 			sources[0].isCknown = true;
			 			sources[0].isSource = true;
			 		}
			 		else{
			 			sources[place].timeC = time(0);
			 			sources[place].lifetime = time(0);
			 			sources[place].mainFrequency = maxFreqC;
			 			sources[place].isCknown = true;
			 			sources[place].isSource = true;
			 		}
			 	}
			 	if(sources[k].isAknown && sources[k].isBknown && sources[k].isCknown){
			 		sources[k].lifetime = time(0);
			 		//C = cx^2 + cy^2 - cs^2 - 660cs*r
					sources[k].C = pow(cx, 2) + pow(cy, 2) -pow(difftime(sources[k].timeC, sources[k].timeA), 2) - 660*(difftime(sources[k].timeC, sources[k].timeA))*INIT_RADIUS;
			 		//B = bx^2 + by^2  - bs^2 - 660bs*r
					sources[k].B = pow(bx, 2) + pow(by, 2) - pow(difftime(sources[k].timeB, sources[k].timeA), 2) - 660*(difftime(sources[k].timeB, sources[k].timeA))*INIT_RADIUS;
			 	}
			 } 
		 
			 //cleanup
			 //for(k = 0; k < MAXSOURCES; k++){
			 //	if(difftime(time(0), sources[k].lifetime) > DISPLAY_LIFETIME)
			 //		sources[k].isSource = false;
			 //}*/
			 
			 
		 	 //cleanup
			 /*for(k = 0; k < MAXSOURCES; k++){
			 	if(!sources[k].isLocked && ((((float)clock() - (float)sources[k].lifetime) / (float)CLOCKS_PER_SEC) < DISPLAY_LIFETIME)){
			 		sources[k].isAknown = false;
			 		sources[k].isBknown = false;
			 		sources[k].isCknown = false;
			 		sources[k].mainFrequency = 0;
			 		sources[k].x0 = 0;
			 		sources[k].y0 = 0;
			 		sources[k].isCalculated = false;
			 		sources[k].lifetime = 0;
			 	}	
			 }*/
	 
			/*
			 * TRIANGULATION
			 */
	  
		  	for(k = 0; k < MAXSOURCES; k++){
		  		if(sources[k].isAknown && sources[k].isCknown && sources[k].isBknown && !sources[k].isCalculated){
		  			//common denominator = 2(cybx - cxby)
					float denom = (cy*bx) - (cx*by);
					denom *= 2;
	
					//x0 = (Bcy - Cby) / 2(cybx - cxby)
					sources[k].x0 = ((sources[k].B)*cy) - ((sources[k].C)*by);
					sources[k].x0 /= denom;
	
					//y0 = (Cbx - Bcx) / 2(cybx - cxby)
					sources[k].y0 = ((sources[k].C)*bx) - ((sources[k].B)*cx);
					sources[k].y0 /= denom;	
				
					sources[k].isCalculated = true;
		  		}
		  	}

		//	btc_write_array(0, (unsigned int*)ATransform, sizeof(ATransform));
		//	btc_poll();
			++count;
			isData = false;
			
			//Add points to the history table
			for(k = 0; k < MAXSOURCES; k++){
				bool isInList = false;
				if(sources[k].isSource){
					int listCounter = 0;
					for(listCounter = 0; listCounter < MAXSOURCES; listCounter++){
						if((sources[k].x0 == xPoints[listCounter]) && (sources[k].y0 == yPoints[listCounter]))
							isInList = true;
					}
					if(!isInList){
						xPoints[historyCounter] = sources[k].x0;
						yPoints[historyCounter] = sources[k].y0;
						++historyCounter;
						historyCounter = (historyCounter >= MAXSOURCES) ? 0 : historyCounter;
					}
				}
			}
		}
		curr_time = clock();
	}
	
	yPoints[MAXSOURCES-1] = -70;
	xPoints[MAXSOURCES-1] = 0;
	
/*	int k;
	int counter = 0;
	for(k = 0; k < MAXSOURCES; k++){
		if(sources[k].isLocked){
			yPoints[counter] = sources[k].y0;
			xPoints[counter] = sources[k].x0;
			++counter;
		}
	}*/
	
/*	int k;
	for(k = 0; k < MAXSOURCES; k++){
		if(sources[k].isSource)
			yPoints[k] = (float) sources[k].mainFrequency;; 	
	}*/
}
