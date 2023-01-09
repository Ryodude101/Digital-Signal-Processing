%Adaptive Filter Design Program

% Parameters
% Sample Rate: 32 kHz
% Desired Signal: Voice, Music, Or Other Audio Signal
% Interfering Signals: Sinusoid, Multiple Sinusoids, another type of 
%     periodic waveform, or narrowband noise. Frequency will be slow
%     varying
% Parameters entered by user at run time: delay (D), predictor length (M),
%     and step size (delta). Allow for a maximum D and M such that the delay
%     and predictor length can each be up to 0.1 seconds long at the specified
%     sample rate. The step size should be a constant, not a function of n.
% Test Values: D=50, M=40, delta=0.001

% Stability:
% 0 < delta < 1/lambda(max)
% 
% gradient(k) = -2*input_sig*error
% w[k+1] = w[k] - delta*gradient(k)
% 
% runtime algorithm:
% w[k] = weight vector (filter coefficients), initialized to zero
% s[k] = w[k]*x[k]; (x = input_sig), convolution of the signal
% e[k] = d[k] - s[k]; d[k] is the "reference signal"
% w[k+1] = w[k] + delta*x[k]*e[k]; correlation of the signal

function simpleLMS(D, M, del , length)
    weights = zeros(1,M);

    sampleRate = 32e3;
    sampleTime = 1/sampleRate;

    xAxis = 0:sampleTime:length;

    perfectSig = sin(4000*pi*xAxis);
    noise = rand(length(xAxis));

    inputSig = noise + perfectSig;
    
    error = 0;
    for i = D:length(xAxis)
        for j = 1:M
            
        end
        error = inputSig(i) - xn;
    end

end
