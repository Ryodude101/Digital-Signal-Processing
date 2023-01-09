#include "Talkthrough.h"
#include "Triangulation.h"
#include <stdfix.h>

//--------------------------------------------------------------------------//
// Function:	Process_Data()												//
//																			//
// Description: This function is called from inside the SPORT0 ISR every 	//
//				time a complete audio frame has been received. The new 		//
//				input samples can be found in the variables iChannel0LeftIn,//
//				iChannel0RightIn, iChannel1LeftIn and iChannel1RightIn 		//
//				respectively. The processed	data should be stored in 		//
//				iChannel0LeftOut, iChannel0RightOut, iChannel1LeftOut,		//
//				iChannel1RightOut, iChannel2LeftOut and	iChannel2RightOut	//
//				respectively.												//
//--------------------------------------------------------------------------//

extern fract16 AdelayLine[LIFETIME];
extern fract16 BdelayLine[LIFETIME];
extern fract16 CdelayLine[LIFETIME];

bool isData = false;

void Process_Data(void)
{	
	/*
	 * INPUTS
	 */
	static int count = LIFETIME - 1;
	if(!isData){
		//Get new samples
		AdelayLine[count] = bitsr((short)(iChannel0LeftIn>>16)); //Mic A input
		BdelayLine[count] = bitsr((short)(iChannel0RightIn>>16)); //Mic B input
		CdelayLine[count] = bitsr((short)(iChannel1LeftIn>>16)); //Mic C input
		
		--count;
		
		if(count < 0){
			isData = true;
			count = LIFETIME - 1;	
		}
		
		/*
		 * OUTPUTS
		 */
		iChannel0LeftOut = iChannel0LeftIn;
		iChannel0RightOut = iChannel0RightIn;
		iChannel1LeftOut = iChannel1LeftIn;
	}
}
