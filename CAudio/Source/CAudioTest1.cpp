/*
  ==============================================================================

    CAudioTest1.cpp
    Created: 17 Jan 2017 12:23:33pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#include "CAudioTest1.h"

#include "CAudioAPI.h"

#include "Wavetables.h"

#include "Yin.h"


AudioVisualiserComponent *scope;


float nRevDelayBufs[14][REV_DELAY_LENGTH];
float prcRevDelayBufs[3][REV_DELAY_LENGTH];

float skBuff[2][REV_DELAY_LENGTH];

int16_t forYin[4096];

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

Yin yin;

#define REVERB 1
#define SVF 0
#define ONEPOLE 0
#define ONEZERO 0
#define TWOZERO 0
#define TWOPOLE 0
#define BIQUAD 0

#define BLOCK 1


void init(AudioVisualiserComponent* s, float sampleRate)
{
    scope = s;
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
int yindex = 0;
// Sample-rate callback.
float tick(float input)
{
    float sample    = 0.0f;
    
    //sample = tStifKarpTick(&pluck);
    
    
    
#if BLOCK
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
    
    sample *= 0.8f;
 

    // Noise.
    if (gNoiseLevel >= 0.05f)    sample += gNoiseLevel * tNoiseTick(&noise);
    
    // Envelope.
    //sample *= tEnvelopeTick(&env);
    
#if SVF
    // Lowpass.
    sample = tSVFTick(&svf, sample);
#endif
    
#if TWOZERO
    sample = tTwoZeroTick(&twozero, sample);
#endif
    
#if TWOPOLE
    sample = tTwoPoleTick(&twopole, sample);
#endif
 
#if BIQUAD
    sample = tBiQuadTick(&biquad, sample);
#endif
    
#endif
    
#if REVERB
    if (gRevMix > 0.05f)        sample = tNRevTick(&rev, sample);
#endif

    // Gain ramp.
    sample *= tRampTick(&ramp);
    
    if (++yindex == 4096)   yindex = 0;
    
    forYin[yindex] = (int16_t) (sample * INT16_MAX);

    return sample;
    
}

void yinGetPitch(void)
{
    float pitch;
    
    int16_t buffer_length = 100;
    
    while (pitch < 10 && buffer_length < 4096)
    {
        Yin_init(&yin, buffer_length, 0.05);
        pitch = Yin_getPitch(&yin, forYin);
        buffer_length++;
    }
    
    DBG("YinPitch: " + String(pitch));
}


void triggerEnvelope(void)
{
    tEnvelopeOn(&env, 1.0f);
    
    tStifKarpPluck(&pluck, 0.9f);
}

float gSamplesBlock;




// Block-rate callback.
void block(void)
{
    float val = 0.0f;
    
    val = getSliderValue("SamplesBlock");
    
    if (gSamplesBlock != val)
    {
        gSamplesBlock = val;
        
        scope->setSamplesPerBlock(gSamplesBlock * 100.0  + 1);
    }
    
    val = getSliderValue("SK Freq");
    
    if (gSKFreq != val)
    {
        gSKFreq = val;
        
        tStifKarpSetFrequency(&pluck, 10.0f + 2000.0f * gSKFreq);
        
    }
    
    val = getSliderValue("SK Damping");
    
    if (gSKDamping != val)
    {
        gSKDamping = val;
        
        tStifKarpControlChange(&pluck, SKStringDamping, gSKDamping * 128.0f);
    }
    
    val = getSliderValue("SK Detune");
    
    if (gSKDetune != val)
    {
        gSKDetune = val;
        
        tStifKarpControlChange(&pluck, SKDetune, gSKDetune * 128.0f);
    }
    
    val = getSliderValue("SK PickPos");
    
    if (gSKPickPos != val)
    {
        gSKPickPos = val;
        
        tStifKarpControlChange(&pluck, SKPickPosition, gSKPickPos * 128.0f);
    }
    
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
    
    // Trigger
    gTrigger = getButtonState("Tap");
    
    if (gTrigger)
    {
        setButtonState("Tap", false);
        triggerEnvelope();
    }
    
    // Yin
    if (getButtonState("Yin"))
    {
        setButtonState("Yin", false);
        yinGetPitch();
    }
    
    
    // Gain
    gGain = getSliderValue("Gain");
    
    tRampSetDest(&ramp, gGain);
    
    
#if BLOCK
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
        tEnvelopeDecay(&env, 2.0f + 2000.0f * gDecay);
    }
    
    // Noise
    gNoiseLevel = getSliderValue("Noise");
    
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
        
        DBG("Pitch: " + String(10.0f + gFund * 1000.0f));
    }
    
#if ONEZERO
    val = getSliderValue("Zero");
    
    if (gOneZeroZero != val)
    {
        gOneZeroZero = val;
        
        tOneZeroSetZero(&onezero, gOneZeroZero);
    }
#endif
    
#if TWOZERO
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
#endif
    
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
    
#if TWOPOLE
    val = getSliderValue("TwoPoleFreq");
    
    if (gTwoPoleFreq != val)
    {
        gTwoPoleFreq = val;
        
        tTwoPoleSetResonance(&twopole, gTwoPoleFreq, gTwoPoleRadius, 0);
    }
    
    val = getSliderValue("TwoPoleRadius");
    
    if (gTwoPoleRadius != val)
    {
        gTwoPoleRadius = val;
        
        tTwoPoleSetResonance(&twopole, gTwoPoleFreq, gTwoPoleRadius, 0);
    }
#endif
    
    val = getSliderValue("BiQuadNotchFreq");
    
    if (gBiQuadNotchFreq != val)
    {
        gBiQuadNotchFreq = val;
        
        tBiQuadSetNotch(&biquad, gBiQuadNotchFreq * 10000.0f, gBiQuadNotchRadius);
    }
    
    val = getSliderValue("BiQuadNotchRadius");
    
    if (gBiQuadNotchRadius != val)
    {
        gBiQuadNotchRadius = val;
        
        tBiQuadSetNotch(&biquad, gBiQuadNotchFreq * 10000.0f, gBiQuadNotchRadius);
    }
    
    val = getSliderValue("BiQuadResFreq");
    
    if (gBiQuadResFreq != val)
    {
        gBiQuadResFreq = val;
        
        tBiQuadSetResonance(&biquad, gBiQuadResFreq * 10000.0f, gBiQuadResRadius, 0);
    }
    
    val = getSliderValue("BiQuadResRadius");
    
    if (gBiQuadResRadius != val)
    {
        gBiQuadResRadius = val;
        
        tBiQuadSetResonance(&biquad, gBiQuadResFreq * 10000.0f, gBiQuadResRadius, 0);
    }


    
    WaveformType state = (WaveformType) getComboBoxState("Waveform");
    
    if (waveform != state)  waveform = state;
    
#endif
    
    
}


