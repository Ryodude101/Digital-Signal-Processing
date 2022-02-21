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
Ap = 0.0001;
tW = 400;

d = fdesign.hilbert('TW,Ap', tW, Ap, Fs);

Hd = design(d, 'equiripple', 'SystemObject', true);
[Coeffs, A] = impz(Hd);
impz(Hd)
freqz(Hd)
zerophase(Hd, 'whole')

%Write the coefficients to a file for submission
FID = fopen('RyanColon_HilCoeff15.txt','w');

if FID > 0
    fprintf(FID, "Ryan Colon, Hilbert Transform Filter, Attempt 15 (Book)\n");
    for i = 1:(length(Coeffs)-1)
            fprintf(FID, '%.14f\n', Coeffs(i));
    end
    fprintf(FID, '%.14f', Coeffs(length(Coeffs)));
    fclose(FID);
end