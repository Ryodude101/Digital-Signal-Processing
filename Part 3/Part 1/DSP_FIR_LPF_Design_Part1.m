%DSP_FIR_LPF_Design_Part1
%Ryan Colon
%01.15.22
%Purpose is to create an LPF meeting following specifications
%using firpmord and firpm to do the calculation

%Specifications:
%Passband:
%   Frq: 0-3.7 kHz
%   Gain: +1 - +1.5 dB
%Stopband:
%   Frq: 4.3 kHz
%   Gain: <-50 dB
%Sample Rate: 48 kHz
%Number of coefficients: no more than 10%

%Following code was heavily inspired by an example provided in the
%matlab documentation for firpmord

clear
clc

passbandFreq = 3.7e3;
stopbandFreq = 4.3e3;

Rp = 0.544; %Passband ripple in dB
Rs = 52.4; %Stopband ripple in dB (65 is current submitted)
Fs = 48e3; %Sampling frequency
F = [passbandFreq stopbandFreq]; %Frequency band edges in Hz
A = [1.1548 0]; %Band amplitudes

Dev = [(10^(Rp/20)-1)/(10^(Rp/20)+1) 10^(-Rs/20)]; %Calculate ripple

[n,fo,ao,w] = firpmord(F,A,Dev,Fs); %Approximate filter parameters
b = firpm(n,fo,ao,w); %Design filter
[h,w2] = freqz(b,1,1024,Fs);
hval = 20*log10(abs(h));
plot(w2, hval) 
title('Magnitude Response')
xlabel('Frequency (Hz)')
ylabel('Magnitude (dB)')

for i = 1:length(hval)
    %check and make sure passband meets spec
    if w2(i) <= passbandFreq && (hval(i) >= 1.5 || hval(i) <= 1.0)
        fprintf('We dont meet passband spec: %f at %d\n', hval(i), w2(i));
        return
    end
   
    %check and make sure stopband meets spec
    if w2(i) >= stopbandFreq && hval(i) > -50
        fprintf('We dont meet stopband spec: %f at %d\n', hval(i), w2(i));
        return
    end
end

impz(b) %Plot Impulse Response
ylabel('Amplitude (Unitless)')

fprintf("Filter coefficients calculated\n")
fprintf("Number of coefficients: %d", n)
fprintf("\nWriting coefficients to file...\n")

%Write the coefficients to a file for submission
FID = fopen('RyanColon_FIR_Coefficients_Rev4.txt','w');

if FID > 0
    fprintf(FID, "Ryan Colon, FIR Filter, Attempt 4 (Optimizing)\n");
    for i = 1:(length(b)-1)
        fprintf(FID, '%.14f\n', b(i));
    end
    fprintf(FID, '%.14f', b(length(b))); 
    fclose(FID);
end

disp("Done")
