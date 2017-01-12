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
#define DELAY_BUFFER_LENGTH_2 (DELAY_BUFFER_LENGTH*2)

#define VERY_SMALL_FLOAT 1.0e-38f

#define DELAY_BUFFER_SIZE 8192//16384

#define PI 3.14159265358979
#define TWO_PI (2 * PI)

/* Ramp */
typedef struct _tRamp {
    float inc;
    float inv_sr_ms;
    float curr,dest;
    float time;
    int samples_per_tick;
    
} tRamp;

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

// Simple linear interpolating delay line. User must supply own buffer.
typedef struct _tDelay
{
    uint32_t in_index, out_index;
    float bottomFrac,topFrac;
    float lastOut,lastIn;
    float *buff;
    float delay;
    
} tDelay;

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
typedef struct _tADSR {
    
    float inv_sr;
    
} tADSR;

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

// Envelope Follower
typedef struct _tEnvelopeFollower
{
    float y;
    float a_thresh;
    float d_coeff;
    
} tEnvelopeFollower;

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

// Highpass filter
typedef struct _tHighpass
{
    float inv_sr;
    float xs, ys, R;
    float cutoff;
    
} tHighpass;

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
