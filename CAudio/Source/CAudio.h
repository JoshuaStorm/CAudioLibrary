/*
  ==============================================================================

    CAudio.h
    Created: 11 Jan 2017 8:32:51pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef CAUDIO_H_INCLUDED
#define CAUDIO_H_INCLUDED

#include "stdint.h"
#include "stdlib.h"

#define DOUBLE 0

#if     !DOUBLE
#define OOPCFloat float
#else
#define OOPCFloat double
#endif

#define DELAY_BUFFER_LENGTH 16384
#define REV_DELAY_LENGTH 65535

#define VERY_SMALL_FLOAT 1.0e-38f

#define DELAY_BUFFER_SIZE 8192//16384

#define PI 3.14159265358979
#define TWO_PI (2 * PI)

#define ONE_OVER_128 1.0f / 128.0f

#define NI 20



#pragma Oscillators
// Phasor: basic aliasing phasor
typedef struct _tPhasor
{
    float phase;
    float inc,freq;
    
    void (*sampleRateChanged)(void);
    
} tPhasor;

// Cycle: Sine waveform
typedef struct _tCycle
{
    // Underlying phasor
    float phase;
    float inc,freq;
    
    void (*sampleRateChanged)(void);
    
} tCycle;

// Sawtooth waveform
typedef struct _tSawtooth
{
    // Underlying phasor
    float phase;
    float inc,freq;
    
    void (*sampleRateChanged)(void);
    
} tSawtooth;

// Triangle waveform
typedef struct _tTriangle
{
    // Underlying phasor
    float phase;
    float inc,freq;
    
    void (*sampleRateChanged)(void);
    
} tTriangle;

// Square waveform
typedef struct _tSquare
{
    // Underlying phasor
    float phase;
    float inc,freq;
    
    void (*sampleRateChanged)(void);
    
} tSquare;

// Noise Types
typedef enum NoiseType
{
    NoiseTypeWhite=0,
    NoiseTypePink,
    NoiseTypeNil,
} NoiseType;

// Noise
typedef struct _tNoise
{
    NoiseType type;
    float pinkb0, pinkb1, pinkb2;
    float(*rand)();
    
    void (*sampleRateChanged)(void);
    
} tNoise;

#pragma Filters
// OnePole filter
typedef struct _tOnePole
{
    float gain;
    float a0,a1;
    float b0,b1;
    
    float lastIn, lastOut;
    
    void (*sampleRateChanged)(void);
    
} tOnePole;

// TwoPole filter
typedef struct _tTwoPole
{
    float gain;
    float a0, a1, a2;
    float b0;
    
    float lastOut[2];
    
    void (*sampleRateChanged)(void);
    
} tTwoPole;

// OneZero filter
typedef struct _tOneZero
{
    float gain;
    float b0,b1;
    float lastIn, lastOut;
    
    void (*sampleRateChanged)(void);
    
} tOneZero;

// TwoZero filter
typedef struct _tTwoZero
{
    float gain;
    float b0, b1, b2;
    
    float lastIn[2];
    
    void (*sampleRateChanged)(void);
    
} tTwoZero;

// PoleZero filter
typedef struct _tPoleZero
{
    float gain;
    float a0,a1;
    float b0,b1;
    
    float lastIn, lastOut;
    
    void (*sampleRateChanged)(void);
    
} tPoleZero;

// BiQuad filter
typedef struct _tBiQuad
{
    float gain;
    float a0, a1, a2;
    float b0, b1, b2;
    
    float lastIn[2];
    float lastOut[2];
    
    void (*sampleRateChanged)(void);
    
    
} tBiQuad;

/* State Variable Filter types */
typedef enum SVFType {
    SVFTypeHighpass = 0,
    SVFTypeLowpass,
    SVFTypeBandpass,
    SVFTypeNotch,
    SVFTypePeak,
} SVFType;

// State Variable Filter, adapted from
typedef struct _tSVF {
    SVFType type;
    float cutoff, Q;
    float ic1eq,ic2eq;
    float g,k,a1,a2,a3;
    
    void (*sampleRateChanged)(void);
    
} tSVF;

// State Variable Filter, adapted from ???
typedef struct _tSVFEfficient {
    SVFType type;
    float cutoff, Q;
    float ic1eq,ic2eq;
    float g,k,a1,a2,a3;
    
    void (*sampleRateChanged)(void);
    
} tSVFEfficient;

// Highpass filter
typedef struct _tHighpass
{
    float xs, ys, R;
    float cutoff;
    
    void (*sampleRateChanged)(void);
    
} tHighpass;

#pragma Basic Utilities
/* Ramp */
typedef struct _tRamp {
    float inc;
    float inv_sr_ms;
    float curr,dest;
    float time;
    int samples_per_tick;
    
    void (*sampleRateChanged)(void);
    
} tRamp;

// Non-interpolating delay line.
typedef struct _tDelay
{
    float gain;
    float *buff;
    
    float lastOut, lastIn;
    
    uint32_t inPoint, outPoint;
    
    uint32_t delay, maxDelay;
    
    void (*sampleRateChanged)(void);
    
} tDelay;

// Linear interpolating delay line. User must supply own buffer.
typedef struct _tDelayL
{
    float gain;
    float *buff;
    
    float lastOut, lastIn;
    
    uint32_t inPoint, outPoint;
    
    uint32_t maxDelay;
    
    float delay;
    
    float alpha, omAlpha;
    
    void (*sampleRateChanged)(void);
    
} tDelayL;

// Allpass delay line. User must supply own buffer.
typedef struct _tDelayA
{
    float gain;
    float *buff;
    
    float lastOut, lastIn;
    
    uint32_t inPoint, outPoint;
    
    uint32_t maxDelay;
    
    float delay;
    
    float alpha, omAlpha, coeff;
    
    float apInput;
    
    void (*sampleRateChanged)(void);
    
} tDelayA;


// Basic Attack-Decay envelope
typedef struct _tEnvelope {
    
    const float *exp_buff;
    const float *inc_buff;
    uint32_t buff_size;
    
    float next;
    
    float attackInc, decayInc, rampInc;
    
    int inAttack, inDecay, inRamp;
    
    int loop;
    
    float gain, rampPeak;
    
    float attackPhase, decayPhase, rampPhase;
    
    void (*sampleRateChanged)(void);
    
} tEnvelope;

// Attack-Decay-Sustain-Release envelope
typedef struct _tADSR
{

    
    void (*sampleRateChanged)(void);
    
} tADSR;

#pragma Physical Models

/* Karplus Strong model */
typedef struct _tPluck
{
    tDelayA     delayLine; // Allpass or Linear??  big difference...
    tOneZero    loopFilter;
    tOnePole    pickFilter;
    tNoise      noise;
    
    float lastOut;
    float loopGain;
    
    float sr;
    
    void (*sampleRateChanged)(void);
    
} tPluck;

/* Stif Karplus Strong model */
typedef struct _tStifKarp
{
    tDelayA  delayLine;
    tDelayL  combDelay;
    tOneZero filter;
    tNoise   noise;
    tBiQuad  biquad[4];
    
    uint32_t length;
    float loopGain;
    float baseLoopGain;
    float lastFrequency;
    float lastLength;
    float stretching;
    float pluckAmplitude;
    float pickupPosition;
    
    float lastOut;
    
    void (*sampleRateChanged)(void);
    
} tStifKarp;

#pragma Complex/Miscellaneous/Other
// Envelope Follower
typedef struct _tEnvelopeFollower
{
    float y;
    float a_thresh;
    float d_coeff;
    
    void (*sampleRateChanged)(void);
    
} tEnvelopeFollower;



// PRCRev: Reverb based on Perry Cook algorithm.
typedef struct _tPRCRev
{
    float mix, t60;
    
    float inv_441;
    
    tDelay allpassDelays[2];
    tDelay combDelay;
    float allpassCoeff;
    float combCoeff;
    
    float lastIn, lastOut;
    
    void (*sampleRateChanged)(void);
    
} tPRCRev;

// NRev: Reverb
typedef struct _tNRev
{
    float mix, t60;
    
    float inv_sr, inv_441;
    
    tDelay allpassDelays[8];
    tDelay combDelays[6];
    float allpassCoeff;
    float combCoeffs[6];
    float lowpassState;
    
    float lastIn, lastOut;
    
    void (*sampleRateChanged)(void);
    
} tNRev;

typedef enum OOPCRegistryIndex
{
    T_PHASOR = 0,
    T_CYCLE,
    T_SAWTOOTH,
    T_TRIANGLE,
    T_SQUARE,
    T_NOISE,
    T_ONEPOLE,
    T_TWOPOLE,
    T_ONEZERO,
    T_TWOZERO,
    T_POLEZERO,
    T_BIQUAD,
    T_SVF,
    T_SVFE,
    T_HIGHPASS,
    T_DELAY,
    T_DELAYL,
    T_DELAYA,
    T_ENVELOPE,
    T_ADSR,
    T_RAMP,
    T_ENVELOPEFOLLOW,
    T_PRCREV,
    T_NREV,
    T_PLUCK,
    T_STIFKARP,
    T_INDEXCNT
}OOPCRegistryIndex;

#pragma OOPC
typedef struct _OOPC
{
    float sampleRate, invSampleRate;
    
    float   (*random)(void);
             
    tPhasor*            tPhasorRegistry[NI];
    tCycle*             tCycleRegistry[NI];
    tSawtooth*          tSawtoothRegistry[NI];
    tTriangle*          tTriangleRegistry[NI];
    tSquare*            tSquareRegistry[NI];
    tNoise*             tNoiseRegistry[NI];
    tOnePole*           tOnePoleRegistry[NI];
    tTwoPole*           tTwoPoleRegistry[NI];
    tOneZero*           tOneZeroRegistry[NI];
    tTwoZero*           tTwoZeroRegistry[NI];
    tPoleZero*          tPoleZeroRegistry[NI];
    tBiQuad*            tBiQuadRegistry[NI];
    tSVF*               tSVFRegistry[NI];
    tSVFEfficient*      tSVFEfficientRegistry[NI];
    tHighpass*          tHighpassRegistry[NI];
    tDelay*             tDelayRegistry[NI];
    tDelayL*            tDelayLRegistry[NI];
    tDelayA*            tDelayARegistry[NI];
    tEnvelope*          tEnvelopeRegistry[NI];
    tADSR*              tADSRRegistry[NI];
    tRamp*              tRampRegistry[NI];
    tEnvelopeFollower*  tEnvelopeFollowerRegistry[NI];
    tPRCRev*            tPRCRevRegistry[NI];
    tNRev*              tNRevRegistry[NI];
    tPluck*             tPluckRegistry[NI];
    tStifKarp*          tStifKarpRegistry[NI];
     
    int registryIndex[T_INDEXCNT];
             
} OOPC;

#endif  // CAUDIO_H_INCLUDED
