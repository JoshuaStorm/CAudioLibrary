/*
  ==============================================================================

    CAudioLibraryTest.h
    Created: 4 Dec 2016 9:14:16pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef CAUDIOLIBRARYTEST_H_INCLUDED
#define CAUDIOLIBRARYTEST_H_INCLUDED

#include "Utilities.h"

#include "CAudioLibrary.h"

#include "Wavetables.h"

float sampleRate;

float gGain;
float gFund;
float gNoiseLevel;
float gSVFCutoff;
float gResonance;


#define NUM_OSC 10

tTriangle osc[NUM_OSC];
float oscGain[NUM_OSC] = {.4, .275, .1, .05, .05, .025, .025, .025, .025, .025};

tNoise      noise;

tSVF        svf;


void init(float sampleRate)
{
    DBG("Initializing C Audio Library Test.");
    
    for (int i = 0; i < NUM_OSC; i++)
    {
        tTriangleInit(&osc[i], sampleRate);
        setFreq(osc[i], 100.0f * (i+1));
    }
    
    tNoiseInit(&noise, sampleRate, &randomNumberGenerator, NoiseTypeWhite);
    
    tSVFInit(&svf, sampleRate, SVFTypeLowpass, 2000, 1.0f);
    
    
}

// Block-rate callback.
void block(void)
{
    gGain = getSliderValue("Gain");
    
    gNoiseLevel = getSliderValue("Noise Level");
    
    float prevFund = gFund;
    gFund = getSliderValue("Fundamental");
    
    if (gFund != prevFund)
    {
        for (int i = 0; i < NUM_OSC; i++)
        {
            setFreq(osc[i], (50.0f + gFund * 500.0f) * (i+1));
        }
    }
    
    gSVFCutoff = getSliderValue("SVF Cutoff");
    
    setFreqFromKnob(svf, 30 + gSVFCutoff * 4000);
    
    gResonance = getSliderValue("SVF Resonance");
    
    setQ(svf, 1.0f + gResonance * 9.0f);
}

// Sample-rate callback.
float tick(float input)
{
    float sample = 0.0;
    
    for (int i = 0; i < NUM_OSC; i++)
    {
        sample += oscGain[i] * tick0(osc[i]);
    }
    sample *= 0.8f;
    
    sample += gNoiseLevel * tick0(noise);
    
    sample = tick1(svf, sample);
    
    sample *= gGain;
    
    return sample;
}

#endif  // CAUDIOLIBRARYTEST_H_INCLUDED
