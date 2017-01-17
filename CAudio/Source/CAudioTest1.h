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


float nRevDelayBufs[14][REV_DELAY_LENGTH];
float prcRevDelayBufs[3][REV_DELAY_LENGTH];

float skBuff[2][REV_DELAY_LENGTH];

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
float gTwoPoleFreq, gTwoPoleRadius;
float gBiQuadNotchFreq, gBiQuadNotchRadius;
float gBiQuadResFreq, gBiQuadResRadius;
bool  gTrigger;
float gRevTime,gRevMix;
float gSKFreq, gSKDamping, gSKPickPos, gSKDetune;
WaveformType waveform = Sine;


#define NUM_OSC 5

tTriangle tri[NUM_OSC];
tSquare sqr[NUM_OSC];
tCycle cyc[NUM_OSC];
tSawtooth saw[NUM_OSC];
float oscGain[NUM_OSC] = {0.3, 0.2, .1, .1, .05};

tNoise      noise;

tSVF        svf;

tEnvelope   env;

tOnePole onepole;
tTwoPole twopole;

tOneZero onezero;
tTwoZero twozero;

tBiQuad biquad;

tNRev rev;

tRamp ramp;

tStifKarp pluck;


#define YIN_BUFFER_LENGTH 4096

Yin yin;

int yindex = 0;
int16_t forYin[YIN_BUFFER_LENGTH];


#define REVERB 0
#define SVF 0
#define ONEPOLE 0
#define ONEZERO 0
#define TWOZERO 0
#define TWOPOLE 0
#define BIQUAD 0

#define BLOCK 0


void init(float sampleRate)
{
    DBG("Initializing C Audio Library Test.");
    
    for (int i = 0; i < NUM_OSC; i++)
    {
        tTriangleInit(&tri[i], sampleRate);
        tTriangleSetFreq(&tri[i], 100.0f * (i+1));
        
        tCycleInit(&cyc[i], sampleRate);
        tCycleSetFreq(&cyc[i], 100.0f * (i+1));
        
        tSquareInit(&sqr[i], sampleRate);
        tSquareSetFreq(&sqr[i], 100.0f * (i+1));

        tSawtoothInit(&saw[i], sampleRate);
        tSawtoothSetFreq(&saw[i], 100.0f * (i+1));
    }
    
    tRampInit(&ramp, sampleRate, 20.0f, 1);
    
    tNoiseInit(&noise, sampleRate, &randomNumberGenerator, NoiseTypeWhite);
    
    tEnvelopeInit(&env, sampleRate, 2.0f, 500.0f, 0, exp_decay, attack_decay_inc);
    
    tStifKarpInit(&pluck, sampleRate, 20.0f, &randomNumberGenerator, skBuff);

#if SVF
    tSVFInit(&svf, sampleRate, SVFTypeLowpass, 2000, 1.0f);
#endif
    
#if ONEPOLE
    tOnePoleInit(&onepole, 0.5f);
#endif
    
#if ONEZERO
    tOneZeroInit(&onezero, 0.5f);
#endif
    
#if TWOZERO
    tTwoZeroInit(&twozero, sampleRate);
#endif
    
#if TWOPOLE
    tTwoPoleInit(&twopole, sampleRate);
#endif
    
    tBiQuadInit(&biquad, sampleRate);
    
#if REVERB
    tNRevInit(&rev, sampleRate, 0.0f, nRevDelayBufs);
#endif
    
}

int count;



// Sample-rate callback.
float tick(float input)
{
    float sample    = 0.0f;
    
#if REVERB
    if (gRevMix > 0.05f)        sample = tNRevTick(&rev, sample);
#endif
    
    

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
