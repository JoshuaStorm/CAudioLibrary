/*
  ==============================================================================

    CAudioLibraryTest.h
    Created: 4 Dec 2016 9:14:16pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef CAUDIOLIBRARYTEST_H_INCLUDED
#define CAUDIOLIBRARYTEST_H_INCLUDED

#include "CAudioLink.h"

#include "CAudioAPI.h"

#include "Wavetables.h"

#include "Yin.h"


#define YIN_BUFFER_LENGTH 2048 //higher better for low frequencies

Yin yin;

int yindex = 0;
int16_t forYin[YIN_BUFFER_LENGTH];


float nRevDelayBufs[14][REV_DELAY_LENGTH];
float prcRevDelayBufs[3][REV_DELAY_LENGTH];

float skBuff[2][REV_DELAY_LENGTH];

float sampleRate;

float gGain;
float gFund;
bool  gTrigger, gDSR, gHSR;

WaveformType waveform = Sine;

#define NUM_OSC 5

tRamp ramp;
tTriangle tri[NUM_OSC];
tSquare sqr[NUM_OSC];
tCycle cyc[NUM_OSC];
tSawtooth saw[NUM_OSC];
float oscGain[NUM_OSC] = {0.3, 0.2, .1, .1, .05};

float sampleRateGlobal;

void init(float sampleRate)
{
    sampleRateGlobal = sampleRate;
    DBG("Initializing C Audio Library Test.");
    
    OOPCInit(sampleRate, &randomNumberGenerator);

    for (int i = 0; i < NUM_OSC; i++)
    {
        tTriangleInit(&tri[i]);
        tTriangleSetFreq(&tri[i], 100.0f * (i+1));
    
        tCycleInit(&cyc[i]);
        tCycleSetFreq(&cyc[i], 100.0f * (i+1));
        
        tSquareInit(&sqr[i]);
        tSquareSetFreq(&sqr[i], 100.0f * (i+1));

        tSawtoothInit(&saw[i]);
        tSawtoothSetFreq(&saw[i], 100.0f * (i+1));
    }
    
    tRampInit(&ramp, 20.0f, 1);
    
}

// Sample-rate callback.
float tick(float input)
{
    float sample    = 0.0f;
    
    // Oscillators.
    if (waveform == Sine)
    {
        for (int i = 0; i < NUM_OSC; i++)   sample += oscGain[i] * tCycleTick(&cyc[i]);
    }
    else if (waveform == Triangle)
    {
        for (int i = 0; i < NUM_OSC; i++)   sample += oscGain[i] * tTriangleTick(&tri[i]);
    }
    else if (waveform == Square)
    {
        for (int i = 0; i < NUM_OSC; i++)   sample += oscGain[i] * tSquareTick(&sqr[i]);
    }
    else if (waveform == Sawtooth)
    {
        for (int i = 0; i < NUM_OSC; i++)   sample += oscGain[i] * tSawtoothTick(&saw[i]);
    }
 
    // Gain ramp.
    sample *= tRampTick(&ramp);
    
    if (++yindex == YIN_BUFFER_LENGTH) yindex = 0;
    
    forYin[yindex] = (int16_t)(sample * INT16_MAX);

    return sample;
    
}

float finalPitch;
float maxProb;

void yinPitchDetect(void)
{
    int buffer_length = 100;
    Yin yin;
    float pitch;
    
    float maxProb = 0.0f;
    float finalPitch = 0.0f;
    
    while (buffer_length < YIN_BUFFER_LENGTH)
    {
        Yin_init(&yin, buffer_length, 0.05);
        pitch = Yin_getPitch(&yin, forYin);
        
        float tempProb = Yin_getProbability(&yin);
        if (pitch >= 10 && tempProb > maxProb)
        {
            maxProb = tempProb;
            finalPitch = pitch;
        }
        
        buffer_length++;
    }
    
    if (finalPitch >= 10) DBG("YinPitch: " + String(finalPitch));
    
    DBG("YinProbability: " + String(maxProb));
}

// Block-rate callback.
void block(void)
{
    float val = 0.0f;
    
    
    // Trigger
    gTrigger = getButtonState("Yin");
    
    if (gTrigger)
    {
        setButtonState("Yin", false);
        yinPitchDetect();
    }

    gDSR = getButtonState("DoubleSR");
    
    if (gDSR)
    {
        setButtonState("DoubleSR", false);
        sampleRateGlobal *= 2;
        OOPCSetSampleRate(sampleRateGlobal);
        
        DBG("sampleRateGlobal: " + String(sampleRateGlobal)); 
    }
    
    gHSR = getButtonState("HalfSR");
    
    if (gHSR)
    {
        setButtonState("HalfSR", false);
        sampleRateGlobal /= 2;
        OOPCSetSampleRate(sampleRateGlobal);
        
        DBG("sampleRateGlobal: " + String(sampleRateGlobal));
    }
    
    
    // Gain
    gGain = getSliderValue("Gain");
    
    tRampSetDest(&ramp, gGain);
    
    // Fundamental
    val =  getSliderValue("OscPitch");
    
    if (gFund != val)
    {
        gFund = val;
        
        for (int i = 0; i < NUM_OSC; i++)
        {
            tTriangleSetFreq(&tri[i], (10.0f + gFund * 1000.0f) * (i+1));
            
            tSquareSetFreq(&sqr[i], (10.0f + gFund * 1000.0f) * (i+1));
            
            tSawtoothSetFreq(&saw[i], (10.0f + gFund * 1000.0f) * (i+1));
            
            tCycleSetFreq(&cyc[i], (10.0f + gFund * 1000.0f) * (i+1));
            
        }
        
        DBG("OscPitch: " + String(10.0f + gFund * 1000.0f));
    }
    
    WaveformType state = (WaveformType) getComboBoxState("Waveform");
    
    if (waveform != state)
    {
        waveform = state;
        
        DBG("waveform: " + cWaveformType[waveform]);
    }
    
}


#endif  // CAUDIOLIBRARYTEST_H_INCLUDED
