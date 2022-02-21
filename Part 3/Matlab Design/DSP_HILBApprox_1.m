%DSP_HILBApprox_1.m
%Ryan Colon
%02.12.22
%Purpose is to create a causal approximation of a Hilbert Transform Filter
%meeting following specifications

%Specifications:
%Sample rate: 32 kHz
%Input Signal Range: 100 Hz to 14 kHz
%Frequency shift range: -16 kHz to +16 kHz
%Attenuation of unwanted component: >-50 dB
%Frequency Shift needs to be able to change mid program

clear
clc

Fs = 32e3;
n = 1200;
amps = [0 1 1 1 1 0];
freqs = [0 49 59 15.7e3 15.701e3 16e3] / (Fs/2);
w = [0.1 1 0.2];


Coeffs = firpm(n, freqs, amps, w, 'hilbert');
freqz(Coeffs)
ylim([-0.2 0.2])

%Write the coefficients to a file for submission
FID = fopen('RyanColon_HilCoeff17.txt','w');

if FID > 0
    fprintf(FID, "Ryan Colon, Hilbert Transform Filter, Attempt 17 (Guess)\n");
    
    for i = 1:(length(Coeffs)-1)
        fprintf(FID, '%.14f\n', Coeffs(i));
    end
    fprintf(FID, '%.14f', Coeffs(length(Coeffs)));
    fclose(FID);
end