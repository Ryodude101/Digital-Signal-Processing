#include "Talkthrough.h"
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
void Process_Data(void)
{
	/* Old Program, merely passthrough. Holding onto it for debugging purposes.
	iChannel0LeftOut = iChannel0LeftIn;
	iChannel0RightOut = iChannel0RightIn;
	iChannel1LeftOut = iChannel1LeftIn;
	iChannel1RightOut = iChannel1RightIn;*/
	
	
	//Constant coefficents for the difference equations
	static const fract b10 = 0.00282002964871r;
	static const fract b11 = 0.00564005929742r;
	static const fract b12 = 0.00282002964871r;
	static const fract a11 = -0.87502859678969r;
	static const fract a12 = 0.49062148093976r;
	static const fract b20 =  0.04762576543440r;
	static const fract b21 = -0.09525153086881r;
	static const fract b22 = 0.04762576543440r;
	static const fract a21 = -0.89065625680738r;
	static const fract a22 = 0.49121467651756r;
	
	//Variables for the delayed values
	static fract x11 = 0r;
	static fract x12 = 0r;
	static fract y11 = 0r;
	static fract y12 = 0r;
	static fract x21 = 0r;
	static fract x22 = 0r;
	static fract y21 = 0r;
	static fract y22 = 0r;
	
	fract in0 = rbits((short)(iChannel0LeftIn>>16));
	
	accum in1 = 2*(b10*in0) + 2*(b11*x11) + 2*(b12*x12) - 2*(a11*y11) - 2*(a12*y12);
	
	in1 = (in1 > FRACT_MAX) ? FRACT_MAX : in1;
	in1 = (in1 < FRACT_MIN) ? FRACT_MIN : in1;
	
	accum sum = 2*(b20*((fract)in1)) + 2*(b21*x21) + 2*(b22*x22) - 2*(a21*y21) - 2*(a22*y22);
	
	//Clip the signal
	sum = (sum > FRACT_MAX) ? FRACT_MAX : sum;
	sum = (sum < FRACT_MIN) ? FRACT_MIN : sum;
	
	//Shif  o7
	x12 = x11;
	x11 = in0;
	y12 = y11;
	y11 = (fract)in1;
	x22 = x21;
	x21 = (fract)in1;
	y22 = y21;
	y21 = (fract)sum;
	
	iChannel0LeftOut = bitslr((long fract)sum);
	iChannel0RightOut = iChannel0LeftIn;
}
