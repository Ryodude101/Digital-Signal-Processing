#ifndef _TRIANGULATION_H_
	#define _TRIANGULATION_H_
	
	#include <time.h>
	#include <complex.h>
	 
	#define INIT_RADIUS 1.3
	#define LIFETIME 512 //Largest Power of 2 that runs, out of memory
	#define MAXSOURCES 100
	#define DISPLAY_LIFETIME 4000
	#define RUN_TIME 1024 
	#define THRESHOLD 0.03
	#define DETECTIONWIDTH 2
	#define LOWERBOUND 5
	
	/* Information needed about every signal
	 * Time A Detected
	 * Time B Detected
	 * Time C Detected
	 * Lifetime after initial Detection
	 * B
	 * C
	 * Is it already a known source?
	 */
	typedef struct signalSource{
	 	clock_t timeA;
	 	clock_t timeB;
	 	clock_t timeC;
	 	float lifetime;
	 	float B;
	 	float C;
	 	bool isAknown;
	 	bool isBknown;
	 	bool isCknown;
	 	unsigned int mainFrequency;
	 	bool isSource;
	 	float x0;
	 	float y0;
	 	bool isCalculated;
	 	//bool isLocked;
	 }signalSource;
#endif
