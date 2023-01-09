/* FFT_Test.c
 * 04.11.22
 * Ryan Colon
 * This C program is to test the FFT library provided for C and see if I can identify sources
 */

#include "FFT_Test.h"

int main(){
    fftw_complex *in, *out;

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N); //2 Dimensional arrays [i][0] = real and [i][1] = imag
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*N);

    //Assign a sin value to in, should show up in the DFT at the proper frequency
    float thisTheta1 = 0;
    float thisTheta2 = 0;
    float deltaTheta1 = 2*M_PI*FREQ1/SAMP_RATE;
    float deltaTheta2 = 2*M_PI*FREQ2/SAMP_RATE;
    for(int i = 0; i < N; i++){
        in[i][0] = sin(thisTheta1) + sin(thisTheta2);
        thisTheta1 += deltaTheta1;
        thisTheta1 = (fabs(thisTheta1) >= 2*M_PI) ? 0 : thisTheta1;
        thisTheta2 += deltaTheta2;
        thisTheta2 = (fabs(thisTheta2) >= 2*M_PI) ? 0 : thisTheta2;
    }

    fftw_plan p; //struct for FFT plan
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE); //load plan
    fftw_execute(p); //execute plan

    //Calculate the magnitude
    float mags[N];
    for(int i = 0; i < N; i++)
        mags[i] = sqrt( pow(out[i][0], 2) + pow(out[i][1], 2) );

    //setup the frequency axis
    float freq_axis[N];
    for(int i = 0; i < (int)floor(N/2); i++)
        freq_axis[i] = ((float)i/N)*SAMP_RATE;

    //Identify Sources
    for(int i = 0; i < (int)floor(N/2); i++) //Only needs to be done on one side (or else I'm in some math trouble)
        if(mags[i] > 1000)
            printf("Signal source at %f Hz\n", freq_axis[i]);

    //Plot the transform using gnuplot
    FILE* gnuplot = popen("gnuplot", "w");
    fprintf(gnuplot, "plot '-' u 1:2 t 'Frequency Plot' w lp\n");
    for(int i = 0; i < floor(N / 2); i++)
        fprintf(gnuplot, "%f %f\n", freq_axis[i], mags[i]);
    
    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
    getchar();

    //clean up memory and end program
    pclose(gnuplot);
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);
    return 0;
}