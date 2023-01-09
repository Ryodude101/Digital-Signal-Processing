_______________________________________________________________________________

ADSP-BF561 EZ-KIT Lite C Based AD1836 TALKTHRU

Analog Devices, Inc.
DSP Division
Three Technology Way
P.O. Box 9106
Norwood, MA 02062

Date Created:	16/10/03

_______________________________________________________________________________

This directory contains an example ADSP-BF561 subroutine that demonstrates the 
initialization of the link between SPORT0 on the ADSP-BF561 and the AD1836 
stereo Codec.  This is done by initializing the link in TDM mode and implementing
a simple talk-through routine.  

Files contained in this directory:

readme.txt			this file
main.c				C file containing the main program and variable 
				declaration
Initialize.c			C file containing all initialization routines
ISR.c				C file containing the interrupt service routine for 
				SPORT0_RX
Process_data.c			C file for processing incoming data
Talkthrough.h			C header file containing prototypes and macros
VisualDSP++  project files
_______________________________________________________________________________


CONTENTS

I.	FUNCTIONAL DESCRIPTION
II.	PROJECT STRUCTURE
III.	OPERATION DESCRIPTION


I.    FUNCTIONAL DESCRIPTION

The Talkthru demo demonstrates the initialization of SPORT0 to establish a link 
between the ADSP-BF561 and the AD1836 Codec.
 
The program simply sets up the SPI port to configure the AD1836 codec to TDM 
mode.  SPORT0 is then setup to receive/transmit audio samples from the AD1836.  
Audio samples received from the AD1836 are moved into the DSP's receive buffer, 
using DMA.  The samples are processed by the ADSP-BF561 and placed in the 
transmit buffer.  In turn, the transmit buffer is used to transmit data to the 
AD1836.  This results in a simple talk-through where data is moved in and out of
the DSP with out performing any computations on the data. All processing is done 
on CoreA, CoreB is not used.


II.   PROJECT STRUCTURE

This is a single core project. It consists of a main project - containing only 
system defines and linker settings - ".\Talkthrough_TDM_C.dpj",
four sub-projects - containing source code - 
						".\CoreA.dpj" (code exclusive to Core A, in L1 memory)
						".\CoreB.dpj" (code exclusive to Core B, in L1 memory)
						".\sml2.dpj" (code shared between the cores, in on-chip L2 memory)
						".\sml3.dpj" (code shared between the cores, in on-chip L3 memory),
and one project group that ties everything together:
						".\Talkthrough_TDM_C.dpg",		



III.  OPERATION DESCRIPTION

- Open the project group,"Talkthrough_TDM_C.dpg" in the VisualDSP 
	Integrated Development Environment (IDDE).
- From the "Project" menu, select "Build Project".(program is then loaded 
	automatically into DSP).
- Setup an input source (such as a radio) to the audio in jack of the EZ-KIT Lite 
- Setup an output source (such as headphones) to the audio out jack of the EZ-KIT Lite
  (Audio IN and OUT setup connections can be made Looking at the front of the BF561 
  EZ-Kit Lite Board's "Audio" Connector jacks as follows. "X" indicate jack connections
  to use);

Video In/out	Audio Out	Audio In
White O O O	O O X		X O  
Red   O O O	O X O		O X
	
- Run the executables by pressing "multiprocessor run" (CTRL-F5) on the toolbar. DO NOT use 
        the single core (F5) button. 

- Listen to the operation of the talk-through.
- Note: Make sure SW4.5 and SW4.6 are both set to OFF.

