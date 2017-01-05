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


float nRevDelayBufs[14][DELAY_BUFFER_LENGTH_2];
float prcRevDelayBufs[3][DELAY_BUFFER_LENGTH_2];

float sampleRate;

float gGain;
float gFund;
float gNoiseLevel;
float gSVFCutoff;
float gResonance;
float gAttack,gDecay;
float gOnePolePole;
float gOneZeroZero;
float gTwoZeroFreq, gTwoZeroRadius;
bool  gTrigger;
float gRevTime,gRevMix;


#define NUM_OSC 5

tTriangle osc[NUM_OSC];
float oscGain[NUM_OSC] = {0.3, 0.2, .1, .1, .05};

tNoise      noise;

tSVF        svf;

tEnvelope   env;

tOnePole onepole;

tOneZero onezero;

tTwoZero twozero;

tNRev rev;

tRamp ramp;

#define REVERB 0
#define SVF 0
#define ONEPOLE 0
#define ONEZERO 1


void init(float sampleRate)
{
    DBG("Initializing C Audio Library Test.");
    
    for (int i = 0; i < NUM_OSC; i++)
    {
        tTriangleInit(&osc[i], sampleRate);
        tTriangleFreq(&osc[i], 100.0f * (i+1));
    }
    
    tNoiseInit(&noise, sampleRate, &randomNumberGenerator, NoiseTypeWhite);
    
    tEnvelopeInit(&env, sampleRate, 2.0f, 500.0f, 0, exp_decay, attack_decay_inc);

#if SVF
    tSVFInit(&svf, sampleRate, SVFTypeLowpass, 2000, 1.0f);
#endif
    
#if ONEPOLE
    tOnePoleInit(&onepole, 0.5f);
#endif
    
#if ONEZERO
    tOneZeroInit(&onezero, 0.5f);
#endif
    
    tTwoZeroInit(&twozero, sampleRate);
    
#if REVERB
    tNRevInit(&rev, sampleRate, 0.0f, nRevDelayBufs);
#endif
    
    tRampInit(&ramp, sampleRate, 20.0f, 1);
    
}

void triggerEnvelope(void)
{
    tEnvelopeOn(&env, 1.0f);
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
        tEnvelopeAttack(&env, 2.0f + 2000.0f * gAttack);
    }
    
    val = getSliderValue("Decay");
    if (gDecay != val)
    {
        gDecay = val;
        float temp =  2.0f + 2000.0f * gDecay;
        DBG(String(temp));
        tEnvelopeDecay(&env, 2.0f + 2000.0f * gDecay);
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
    
    tRampSetDest(&ramp, gGain);
    
    // Noise
    gNoiseLevel = getSliderValue("Noise");
    
    // Fundamental
    val =  getSliderValue("OscPitch");
    
    if (gFund != val)
    {
        gFund = val;
        
        for (int i = 0; i < NUM_OSC; i++)
        {
            tTriangleFreq(&osc[i], (50.0f + gFund * 500.0f) * (i+1));
        }
    }
    /*
    val = getSliderValue("Zero");
    
    if (gOneZeroZero != val)
    {
        gOneZeroZero = val;
        
        tOneZeroSetZero(&onezero, gOneZeroZero);
    }
     */
    
    val = getSliderValue("TZFrequency");
    
    if (gTwoZeroFreq != val)
    {
        gTwoZeroFreq = val;
        
        tTwoZeroSetNotch(&twozero, gTwoZeroFreq * 10000.0f, gTwoZeroRadius);
    }
    
    val = getSliderValue("TZRadius");
    
    if (gTwoZeroRadius != val)
    {
        gTwoZeroRadius = val;
        
        tTwoZeroSetNotch(&twozero, gTwoZeroFreq * 10000.0f, gTwoZeroRadius);
    }
    
    
    
#if SVF
    // SVF Cutoff
    val = getSliderValue("SVF Cutoff");
    
    if (gSVFCutoff != val)
    {
        gSVFCutoff = val;
    
        tSVFSetFreq(&svf, 30 + gSVFCutoff * 4000);
    }
    
    // SVF Resonance
    val = getSliderValue("SVF Resonance");
    
    if (gResonance != val)
    {
        gResonance = val;
        
        tSVFSetQ(&svf, 1.0f + gResonance * 9.0f);
    }
#endif
    
#if ONEPOLE
    // Allpass pole
    val = getSliderValue("OnePole Pole");
    
    if (gOnePolePole != val)
    {
        gOnePolePole = val;
        
        tOnePoleSetPole(&onepole, gOnePolePole);
    }
#endif
    
#if REVERB
    val = getSliderValue("Reverb Time");
    
    if (gRevTime != val)
    {
        gRevTime = val;
        
        tNRevSetT60(&rev, gRevTime * 20.0f);
    }
    
    val = getSliderValue("Reverb Mix");
    
    if (gRevMix != val)
    {
        gRevMix = val;
        
        tNRevSetMix(&rev, gRevMix);
        
        tNRevSetMix(&rev, gRevMix);
    }
#endif
}



// Sample-rate callback.
float tick(float input)
{
    float sample    = 0.0f;
    
    // Oscillators.
    for (int i = 0; i < NUM_OSC; i++)
        sample += oscGain[i] * tTriangleTick(&osc[i]);
    sample *= 0.8f;
    
    // Noise.
    if (gNoiseLevel >= 0.5f)    sample += gNoiseLevel * tNoiseTick(&noise);
    
    // Envelope.
    sample *= tEnvelopeTick(&env);
    
#if REVERB
    // Reverb.
    if (gRevMix > 0.05f)        sample = tNRevTick(&rev, sample);
#endif
    
#if SVF
    // Lowpass.
    sample = tSVFTick(&svf, sample);
#endif
    
    sample = tTwoZeroTick(&twozero, sample);
    
    // Gain ramp.
    sample *= tRampTick(&ramp);
    
    return sample;
    
}

#endif  // CAUDIOLIBRARYTEST_H_INCLUDED
