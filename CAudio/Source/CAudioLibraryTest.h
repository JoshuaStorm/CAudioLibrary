/*
  ==============================================================================

    CAudioLibraryTest.h
    Created: 4 Dec 2016 9:14:16pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef CAUDIOLIBRARYTEST_H_INCLUDED
#define CAUDIOLIBRARYTEST_H_INCLUDED

#include "CAudioUtilities.h"

#include "CAudioLibrary.h"

#include "Wavetables.h"

float sampleRate;

float gGain;
float gFund;
float gNoiseLevel;
float gSVFCutoff;
float gResonance;
float gAttack,gDecay;
bool  gTrigger;

#define NUM_OSC 10

tTriangle osc[NUM_OSC];
float oscGain[NUM_OSC] = {0.3, 0.2, .1, .1, .05, .05, .075, .025, .05, .05};

tNoise      noise;

tSVF        svf;

tEnvelope   env;


void init(float sampleRate)
{
    DBG("Initializing C Audio Library Test.");
    
    for (int i = 0; i < NUM_OSC; i++)
    {
        tTriangleInit(&osc[i], sampleRate);
        setFreq(osc[i], 100.0f * (i+1));
    }
    
    tNoiseInit(&noise, sampleRate, &randomNumberGenerator, NoiseTypeWhite);
    
    tEnvelopeInit(&env, sampleRate, 2.0f, 500.0f, 0, exp_decay, attack_decay_inc);
    
    tSVFInit(&svf, sampleRate, SVFTypeLowpass, 2000, 1.0f);
    
    
}

void triggerEnvelope(void)
{
    envOn(env, 1.0f);
}

// Block-rate callback.
void block(void)
{
    float val = 0.0f;
    
    // Envelope
    val = getSliderValue("Attack");
    if (gAttack != val)
    {
        gAttack = val;
        setEnvelopeAttack(env, 2.0f + 2000.0f * gAttack);
    }
    
    val = getSliderValue("Decay");
    if (gDecay != val)
    {
        gDecay = val;
        setEnvelopeDecay(env, 2.0f + 2000.0f * gDecay);
    }
    
    // Trigger
    gTrigger = getButtonState("Tap");
    
    if (gTrigger)
    {
        setButtonState("Tap", false);
        triggerEnvelope();
    }
    
    // Gain
    gGain = getSliderValue("Gain");
    
    // Noise
    gNoiseLevel = getSliderValue("Noise");
    
    // Fundamental
    val =  getSliderValue("Frequency");
    
    if (gFund != val)
    {
        gFund = val;
        
        for (int i = 0; i < NUM_OSC; i++)
        {
            setFreq(osc[i], (50.0f + gFund * 500.0f) * (i+1));
        }
    }
    
    // SVF Cutoff
    val = getSliderValue("SVF Cutoff");
    
    if (gSVFCutoff != val)
    {
        gSVFCutoff = val;
    
        setFreqFromKnob(svf, 30 + gSVFCutoff * 4000);
    }
    
    // SVF Resonance
    val = getSliderValue("SVF Resonance");
    
    if (gResonance != val)
    {
        gResonance = val;
        
        setQ(svf, 1.0f + gResonance * 9.0f);
    }
}

// Sample-rate callback.
float tick(float input)
{
    float sample    = 0.0f;
    
    for (int i = 0; i < NUM_OSC; i++)
    {
        sample += oscGain[i] * tick0(osc[i]);
    }
    sample *= 0.8f;
    
    sample += gNoiseLevel * tick0(noise);
    
    sample = tick1(svf, sample);
    
    float val = tick0(env);
    
    //DBG(String(val));
    
    sample *= val;
    
    sample *= gGain;
    
    return sample;
    
}

#endif  // CAUDIOLIBRARYTEST_H_INCLUDED
