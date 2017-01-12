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


#define DELAY_BUFFER_LENGTH 16384
#define REV_DELAY_LENGTH 65535

#define VERY_SMALL_FLOAT 1.0e-38f

#define DELAY_BUFFER_SIZE 8192//16384

#define PI 3.14159265358979
#define TWO_PI (2 * PI)

#define ONE_OVER_128 1.0f / 128.0f

#pragma Oscillators
// Phasor: basic aliasing phasor
typedef struct _tPhasor
{
    float phase;
    float inc;
    float inv_sr;
    
} tPhasor;

// Cycle: Sine waveform
typedef struct _tCycle
{
    // Underlying phasor
    float phase;
    float inc;
    float inv_sr;
    
} tCycle;

// Sawtooth waveform
typedef struct _tSawtooth
{
    // Underlying phasor
    float phase;
    float inc,freq;
    float inv_sr;
    
} tSawtooth;

// Triangle waveform
typedef struct _tTriangle
{
    // Underlying phasor
    float phase;
    float inc,freq;
    float inv_sr;
    
} tTriangle;

// Square waveform
typedef struct _tSquare
{
    // Underlying phasor
    float phase;
    float inc,freq;
    float inv_sr;
    
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
    
} tNoise;

#pragma Filters
// OnePole filter
typedef struct _tOnePole
{
    float gain;
    float a0,a1;
    float b0,b1;
    
    float lastIn, lastOut;
    
} tOnePole;

// TwoPole filter
typedef struct _tTwoPole
{
    float gain;
    float a0, a1, a2;
    float b0;
    
    float sr, inv_sr;
    
    float lastOut[2];
    
} tTwoPole;

// OneZero filter
typedef struct _tOneZero
{
    float gain;
    float b0,b1;
    float sr;
    float lastIn, lastOut;
    
} tOneZero;

// TwoZero filter
typedef struct _tTwoZero
{
    float gain;
    float b0, b1, b2;
    
    float sr, inv_sr;
    
    float lastIn[2];
    
} tTwoZero;

// PoleZero filter
typedef struct _tPoleZero
{
    float gain;
    float a0,a1;
    float b0,b1;
    
    float lastIn, lastOut;
    
} tPoleZero;

// BiQuad filter
typedef struct _tBiQuad
{
    float gain;
    float a0, a1, a2;
    float b0, b1, b2;
    
    float sr, inv_sr;
    
    float lastIn[2];
    float lastOut[2];
    
    
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
    float inv_sr;
    float cutoff, Q;
    float ic1eq,ic2eq;
    float g,k,a1,a2,a3;
    
} tSVF;

// State Variable Filter, adapted from ???
typedef struct _tSVFEfficient {
    SVFType type;
    float inv_sr;
    float cutoff, Q;
    float ic1eq,ic2eq;
    float g,k,a1,a2,a3;
    
} tSVFEfficient;

// Highpass filter
typedef struct _tHighpass
{
    float inv_sr;
    float xs, ys, R;
    float cutoff;
    
} tHighpass;

#pragma Basic Utilities
/* Ramp */
typedef struct _tRamp {
    float inc;
    float inv_sr_ms;
    float curr,dest;
    float time;
    int samples_per_tick;
    
} tRamp;

// Non-interpolating delay line.
typedef struct _tDelay
{
    float gain;
    float *buff;
    
    float lastOut, lastIn;
    
    uint32_t inPoint, outPoint;
    
    uint32_t delay, maxDelay;
    
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
    
} tDelayA;


// Basic Attack-Decay envelope
typedef struct _tEnvelope {
    
    float inv_sr;
    const float *exp_buff;
    const float *inc_buff;
    uint32_t buff_size;
    
    float next;
    
    float attackInc, decayInc, rampInc;
    
    int inAttack, inDecay, inRamp;
    
    int loop;
    
    float gain, rampPeak;
    
    float attackPhase, decayPhase, rampPhase;
    
} tEnvelope;

// Attack-Decay-Sustain-Release envelope
typedef struct _tADSR
{
    
    float inv_sr;
    
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
    
    float sr;
    
} tStifKarp;

#pragma Complex/Miscellaneous/Other
// Envelope Follower
typedef struct _tEnvelopeFollower
{
    float y;
    float a_thresh;
    float d_coeff;
    
} tEnvelopeFollower;



// PRCRev: Reverb based on Perry Cook algorithm.
typedef struct _tPRCRev
{
    float mix, t60;
    
    float inv_sr, inv_441;
    
    tDelay allpassDelays[2];
    tDelay combDelay;
    float allpassCoeff;
    float combCoeff;
    
    float lastIn, lastOut;
    
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
    
} tNRev;

#endif  // CAUDIO_H_INCLUDED
