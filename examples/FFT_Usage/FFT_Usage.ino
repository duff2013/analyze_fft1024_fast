#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <analyze_fft1024_fast.h>

AudioAnalyzeFFT1024_Fast  fastfft;
AudioSynthWaveformSine    sinewave;
AudioOutputAnalog         dac;

AudioConnection patchCord1(sinewave, 0, fastfft, 0);
AudioConnection patchCord2(sinewave, 0, dac, 0);

void setup() {
    while (!Serial);
    delay(100);
    Serial.println("Fast FFT Usage Example...");
    AudioMemory(24);
    fastfft.windowFunction(AudioWindowHanning1024);
    sinewave.amplitude(0.8);
    sinewave.frequency(440);
}

void loop() {
    float p1 = fastfft.processorUsage();
    float p2 = fastfft.processorUsageMax();
    Serial.printf("Fast FFT Usage: %6.2f\t\tFast FFT Max Usage: %6.2f\n", p1, p2);
    delay(50);
}