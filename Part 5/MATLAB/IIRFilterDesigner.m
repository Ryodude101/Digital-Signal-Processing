% IIR Filter Design Code
% 03.29.22
% Ryan Colon
% might need to use tf2sos I think

% Specifications:
% Bandpass IIR, Chebyshev Type I
% 4 poles
% center freq: 1.2 kHz
% passband bandwidth: 101.25 Hz, X = 0.05(M+42)^2
% passband ripple: 1.5 dB
% gain at max points: 5 dB
% Sample Rate: 16 kHz

clear
clc
                          
M = 3; %March is my birthday month
bandWidth = 0.05 * (M + 42)^2; %Resultant passband Bandwith in Hz
centerFreq = 1.2e3;
passbandFreqs = [(centerFreq - (bandWidth / 2)) (centerFreq + (bandWidth / 2))];
order = 4;
passbandRipple = 1.5; %dB
sampleRate = 16e3;

%Order is divided by 2 because in the matlab algorith
%it represents 1/2 the order for bandpass filters
[state input output feedthrough] = cheby1(order / 2, passbandRipple, (passbandFreqs / (sampleRate/2)));

%Get the second order system and transfer function forms
sos = ss2sos(state, input, output, feedthrough);

%I want the max gain to be 5 dB so convert 5 dB
%to V/V and multiply the numerator by it
gain = 10^(5/20);

num1 = 14*sqrt(gain)*(sos(1, 1:3));
denom1 = sos(1, 4:6);
num2 = (1/14)*sqrt(gain)*(sos(2, 1:3));
denom2 = sos(2, 4:6);

d1 = dfilt.df1(num1, denom1);
d2 = dfilt.df2(num2, denom2);

Hd = dfilt.cascade(d1, d2);

freqz(Hd)
% freqz(d1)
% freqz(d2)

%Write the coefficients to a file for submission
FID = fopen('RyanColon_IIR_Coefficients_Rev1.txt','w');

if FID > 0
    fprintf(FID, 'Difference Equation 1: ')
    fprintf(FID, '%.14fr %.14fr %.14fr %.14fr %.14fr\n', num1(1)/2, num1(2)/2, num1(3)/2, denom1(2)/2, denom1(3)/2);
    fprintf(FID, 'Difference Equation 2: ')
    fprintf(FID, '%.14fr %.14fr %.14fr %.14fr %.14fr\n', num2(1)/2, num2(2)/2, num2(3)/2, denom2(2)/2, denom2(3)/2);
end