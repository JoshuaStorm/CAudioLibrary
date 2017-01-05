/*
  ==============================================================================

    CAudioLibrary.h
    Created: 4 Dec 2016 9:26:17pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef CAUDIOLIBRARY_H_INCLUDED
#define CAUDIOLIBRARY_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
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

int      tRampInit   (tRamp *r, float sr, float time, int samples_per_tick);
int      tRampSetTime(tRamp *r, float time);
int      tRampSetDest(tRamp *r, float dest);
float    tRampTick   (tRamp *r);

/* OnePole */
typedef struct _tOnePole
{
    float gain;
    float a0,a1;
    float b0,b1;
    
    float lastIn, lastOut;
    
} tOnePole;

int      tOnePoleInit(tOnePole *f, float thePole);
void     tOnePoleSetB0(tOnePole *f, float b0);
void     tOnePoleSetA1(tOnePole *f, float a1);
void     tOnePoleSetPole(tOnePole *f, float thePole);
void     tOnePoleSetCoefficients(tOnePole *f, float b0, float a1);
void     tOnePoleSetGain(tOnePole *f, float gain);
float    tOnePoleTick(tOnePole *f, float input);

/* OneZero */
typedef struct _tOneZero
{
    float gain;
    float b0,b1;
    
    float lastIn, lastOut;
    
} tOneZero;

int      tOneZeroInit(tOneZero *f, float theZero);
void     tOneZeroSetB0(tOneZero *f, float b0);
void     tOneZeroSetB1(tOneZero *f, float b1);
void     tOneZeroSetZero(tOneZero *f, float theZero);
void     tOneZeroSetCoefficients(tOneZero *f, float b0, float b1);
void     tOneZeroSetGain(tOneZero *f, float gain);
float    tOneZeroTick(tOneZero *f, float input);

/* TwoZero */
typedef struct _tTwoZero
{
    float gain;
    float b0, b1, b2;
    
    float sr, inv_sr;
    
    float lastIn[2];
    
} tTwoZero;

int      tTwoZeroInit           (tTwoZero *f, float sampleRate);
void     tTwoZeroSetB0          (tTwoZero *f, float b0);
void     tTwoZeroSetB1          (tTwoZero *f, float b1);
void     tTwoZeroSetB2          (tTwoZero *f, float b2);
void     tTwoZeroSetNotch       (tTwoZero *f, float frequency, float radius);
void     tTwoZeroSetCoefficients(tTwoZero *f, float b0, float b1, float b2);
void     tTwoZeroSetGain        (tTwoZero *f, float gain);
float    tTwoZeroTick           (tTwoZero *f, float input);

/* PoleZero */
typedef struct _tPoleZero
{
    float gain;
    float a0,a1;
    float b0,b1;
    
    float lastIn, lastOut;
    
} tPoleZero;

int      tPoleZeroInit(tPoleZero *pzf);
void     tPoleZeroSetB0(tPoleZero *pzf, float b0);
void     tPoleZeroSetB1(tPoleZero *pzf, float b1);
void     tPoleZeroSetA1(tPoleZero *pzf, float a1);
void     tPoleZeroSetCoefficients(tPoleZero *pzf, float b0, float b1, float a1);
void     tPoleZeroSetAllpass(tPoleZero *pzf, float coeff);
void     tPoleZeroSetBlockZero(tPoleZero *pzf, float thePole);
void     tPoleZeroSetGain(tPoleZero *pzf, float gain);
float    tPoleZeroTick(tPoleZero *pzf, float input);

typedef struct _tDelay
{
    uint32_t in_index, out_index;
    float bottomFrac,topFrac;
    float lastOut,lastIn;
    float *buff;
    float delay;
    
} tDelay;

int      tDelayInit      (tDelay *d, float *buff);
int      tDelaySetDelay  (tDelay *d, float delay);
float    tDelayGetDelay  (tDelay *d);
float    tDelayGetLastOut(tDelay *d);
float    tDelayGetLastIn (tDelay *d);
float    tDelayTick      (tDelay *d, float sample);

/* Envelope generator. */
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

int      tEnvelopeInit(tEnvelope *env, float sr, float attack, float decay, int loop,
                              const float *exponentialTable, const float *attackDecayIncTable);
int      tEnvelopeAttack(tEnvelope *env, float attack);
int      tEnvelopeDecay(tEnvelope *env, float decay);
int      tEnvelopeLoop(tEnvelope *env, int loop);
int      tEnvelopeOn(tEnvelope *env, float velocity);
float    tEnvelopeTick(tEnvelope *env);

/* Attack-Decay-Sustain-Release envelope. */
typedef struct _tADSR {
    
    float inv_sr;
    
} tADSR;

int tADSRInit(tADSR *d, float attack, float decay, float sustain, float release);


/* State Variable Filter adapted from ???. */
typedef enum SVFType {
    SVFTypeHighpass = 0,
    SVFTypeLowpass,
    SVFTypeBandpass,
    SVFTypeNotch,
    SVFTypePeak,
} SVFType;

typedef struct _tSVF {
    SVFType type;
    float inv_sr;
    float cutoff, Q;
    float ic1eq,ic2eq;
    float g,k,a1,a2,a3;
    
} tSVF;

int      tSVFInit(tSVF *svf, float sr, SVFType type, uint16_t cutoffKnob, float Q);
float    tSVFTick(tSVF *svf, float v0);
int      tSVFSetFreq(tSVF *svf, uint16_t cutoffKnob);
int      tSVFSetQ(tSVF *svf, float Q);

/* Efficient version of SVF for 14-bit control input, [0, 4096). */
typedef struct _tSVFEfficient {
    SVFType type;
    float inv_sr;
    float cutoff, Q;
    float ic1eq,ic2eq;
    float g,k,a1,a2,a3;
    
} tSVFEfficient;

// 0-4096 is mapped to midi notes 16-130 for cutoff frequency
int      tSVFEfficientInit    (tSVFEfficient *svf, float sr, SVFType type, uint16_t cutoffKnob, float Q);
float    tSVFEfficientTick    (tSVF *svf, float v0);
int      tSVFEfficientSetFreq (tSVF *svf, uint16_t cutoffKnob);
int      tSVFEfficientSetQ    (tSVF *svf, float Q);

/* Envelope Follower */
typedef struct _tEnvelopeFollower
{
    float y;
    float a_thresh;
    float d_coeff;
    
} tEnvelopeFollower;

int      tEnvelopeFollowerInit           (tEnvelopeFollower *ef, float attackThreshold, float decayCoeff);
int      tEnvelopeFollowerDecayCoeff     (tEnvelopeFollower *ef, float decayCoeff);
int      tEnvelopeFollowerAttackThresh   (tEnvelopeFollower *ef, float attackThresh);
float    tEnvelopeFollowerTick           (tEnvelopeFollower *ef, float x);

/* Phasor [0.0, 1.0) */
typedef struct _tPhasor
{
    float phase;
    float inc;
    float inv_sr;
    
} tPhasor;

int      tPhasorInit(tPhasor *p, float sr);
int      tPhasorFreq(tPhasor *p, float freq);
float    tPhasorTick(tPhasor *p);

/* Cycle */
typedef struct _tCycle
{
    // Underlying phasor
    float phase;
    float inc;
    float inv_sr;
    
    // Wavetable synthesis
    const float *wt; //wavetable
    int wtlen; //wavetable length
    
} tCycle;

int      tCycleInit(tCycle *c, float sr, const float *table, int len);
int      tCycleFreq(tCycle *c, float freq);
float    tCycleTick(tCycle *c);

/* Sawtooth */
typedef struct _tSawtooth
{
    // Underlying phasor
    float phase;
    float inc;
    float inv_sr;
    
} tSawtooth;

int      tSawtoothInit(tSawtooth *t, float sr);
int      tSawtoothFreq(tSawtooth *s, float freq);
float    tSawtoothTick(tSawtooth *s);
int      tSawtoothInit(tSawtooth *s, float sr);

/* Triangle */
typedef struct _tTriangle
{
    // Underlying phasor
    float phase;
    float inc;
    float inv_sr;
    
} tTriangle;

int      tTriangleInit(tTriangle *t, float sr);
int      tTriangleFreq(tTriangle *t, float freq);
float    tTriangleTick(tTriangle *t);

/* Pulse */
typedef struct _tPulse
{
    // Underlying phasor
    float phase;
    float inc;
    float inv_sr;
    
    float pw;
    
} tPulse;

int      tPulseInit  (tPulse *p, float sr, float pwidth);
int      tPulseWidth (tPulse *p, float pwidth);
int      tPulseFreq  (tPulse *p, float freq);
float    tPulseTick  (tPulse *p);


/* Noise */
typedef enum NoiseType
{
    NoiseTypeWhite=0,
    NoiseTypePink,
    NoiseTypeNil,
} NoiseType;

typedef struct _tNoise
{
    NoiseType type;
    float pinkb0, pinkb1, pinkb2;
    float(*rand)();
    
} tNoise;

int      tNoiseInit      (tNoise *n, float sr, float (*randomNumberGenerator)(), NoiseType type);
float    tNoiseTick  (tNoise *n);

typedef struct _tHighpass
{
    float inv_sr;
    float xs, ys, R;
    float cutoff;
    
} tHighpass;

int      tHighpassInit   (tHighpass *hp, float sr, float freq);
int      tHighpassFreq   (tHighpass *hp, float freq);
float    tHighpassTick   (tHighpass *hp, float x);

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

int     tPRCRevInit      (tPRCRev *r, float sr, float t60, float delayBuffers[3][DELAY_BUFFER_LENGTH_2]);
void    tPRCRevSetT60    (tPRCRev *r, float t60);
void    tPRCRevSetMix    (tPRCRev *r, float mix);
float   tPRCRevTick      (tPRCRev *r, float input);

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

int      tNRevInit   (tNRev *r, float sr, float t60, float delayBuffers[14][DELAY_BUFFER_LENGTH_2]);
void     tNRevSetT60 (tNRev *r, float t60);
void     tNRevSetMix (tNRev *r, float mix);
float    tNRevTick   (tNRev *r, float input);

/*
 typedef struct _tDelayL {
 float inv_sr;
 
 void (*setDelay)(struct _tDelayL *self, float del);
 float (*getDelay)(struct _tDelayL *self);
 
 float (*tapOut)(struct _tDelayL *self, uint32_t tapDelay);
 void (*tapIn)(struct _tDelayL *self, float val, uint32_t tapDelay);
 
 float (*lastOut)(struct _tDelayL *self);
 
 float (*nextOut)(struct _tDelayL *self);
 
 float (*tick)(struct _tDelayL *self, float input);
 
 uint32_t inPoint;
 uint32_t outPoint;
 float delay;
 float alpha, omAlpha;
 float nextOutput;
 int doNextOut;
 
 } tDelayL;
 
 void tDelayLInit(tDelayL *del, float delay);
 */

#endif  // CAUDIOLIBRARY_H_INCLUDED
