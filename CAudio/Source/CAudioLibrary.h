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

#include "CAudio.h"

#pragma Basic oscillators/waveforms
/* tPhasor: Aliasing phasor [0.0, 1.0) */
int      tPhasorInit(tPhasor *p, float sr);
int      tPhasorFreq(tPhasor *p, float freq);
float    tPhasorTick(tPhasor *p);

/* tCycle: Cycle/Sine waveform */
int      tCycleInit(tCycle *c, float sr);
int      tCycleSetFreq(tCycle *c, float freq);
float    tCycleTick(tCycle *c);

/* tSawtooth: Anti-aliased Sawtooth waveform*/
int      tSawtoothInit(tSawtooth *t, float sr);
int      tSawtoothSetFreq(tSawtooth *s, float freq);
float    tSawtoothTick(tSawtooth *s);

/* tTriangle: Anti-aliased Triangle waveform */
int      tTriangleInit(tTriangle *t, float sr);
int      tTriangleSetFreq(tTriangle *t, float freq);
float    tTriangleTick(tTriangle *t);

/* tSquare: Anti-aliased Square waveform */
int      tSquareInit  (tSquare *p, float sr);
int      tSquareSetFreq  (tSquare *p, float freq);
float    tSquareTick  (tSquare *p);

/* tNoise: Noise generator */
int      tNoiseInit      (tNoise *n, float sr, float (*randomNumberGenerator)(), NoiseType type);
float    tNoiseTick  (tNoise *n);

#pragma Filters

/* tOnePole: OnePole filter */
int      tOnePoleInit           (tOnePole *f, float thePole);
void     tOnePoleSetB0          (tOnePole *f, float b0);
void     tOnePoleSetA1          (tOnePole *f, float a1);
void     tOnePoleSetPole        (tOnePole *f, float thePole);
void     tOnePoleSetCoefficients(tOnePole *f, float b0, float a1);
void     tOnePoleSetGain        (tOnePole *f, float gain);
float    tOnePoleTick           (tOnePole *f, float input);

/* TwoPole filter */
int      tTwoPoleInit           (tTwoPole *f, float sr);
void     tTwoPoleSetB0          (tTwoPole *f, float b0);
void     tTwoPoleSetA1          (tTwoPole *f, float a1);
void     tTwoPoleSetA2          (tTwoPole *f, float a2);
void     tTwoPoleSetResonance   (tTwoPole *f, float freq, float radius, int normalize);
void     tTwoPoleSetCoefficients(tTwoPole *f, float b0, float a1, float a2);
void     tTwoPoleSetGain        (tTwoPole *f, float gain);
float    tTwoPoleTick           (tTwoPole *f, float input);

/* OneZero filter */
int      tOneZeroInit(tOneZero *f, float theZero);
void     tOneZeroSetB0(tOneZero *f, float b0);
void     tOneZeroSetB1(tOneZero *f, float b1);
void     tOneZeroSetZero(tOneZero *f, float theZero);
void     tOneZeroSetCoefficients(tOneZero *f, float b0, float b1);
void     tOneZeroSetGain(tOneZero *f, float gain);
float    tOneZeroTick(tOneZero *f, float input);

/* TwoZero filter */
int      tTwoZeroInit           (tTwoZero *f, float sampleRate);
void     tTwoZeroSetB0          (tTwoZero *f, float b0);
void     tTwoZeroSetB1          (tTwoZero *f, float b1);
void     tTwoZeroSetB2          (tTwoZero *f, float b2);
void     tTwoZeroSetNotch       (tTwoZero *f, float frequency, float radius);
void     tTwoZeroSetCoefficients(tTwoZero *f, float b0, float b1, float b2);
void     tTwoZeroSetGain        (tTwoZero *f, float gain);
float    tTwoZeroTick           (tTwoZero *f, float input);

/* PoleZero filter */
int      tPoleZeroInit(tPoleZero *pzf);
void     tPoleZeroSetB0(tPoleZero *pzf, float b0);
void     tPoleZeroSetB1(tPoleZero *pzf, float b1);
void     tPoleZeroSetA1(tPoleZero *pzf, float a1);
void     tPoleZeroSetCoefficients(tPoleZero *pzf, float b0, float b1, float a1);
void     tPoleZeroSetAllpass(tPoleZero *pzf, float coeff);
void     tPoleZeroSetBlockZero(tPoleZero *pzf, float thePole);
void     tPoleZeroSetGain(tPoleZero *pzf, float gain);
float    tPoleZeroTick(tPoleZero *pzf, float input);

/* BiQuad filter */
int      tBiQuadInit           (tBiQuad *f, float sr);
void     tBiQuadSetB0          (tBiQuad *f, float b0);
void     tBiQuadSetA1          (tBiQuad *f, float a1);
void     tBiQuadSetA2          (tBiQuad *f, float a2);
void     tBiQuadSetNotch       (tBiQuad *f, float freq, float radius);
void     tBiQuadSetResonance   (tBiQuad *f, float freq, float radius, int normalize);
void     tBiQuadSetCoefficients(tBiQuad *f, float b0, float a1, float a2);
void     tBiQuadSetGain        (tBiQuad *f, float gain);
float    tBiQuadTick           (tBiQuad *f, float input);

/* State Variable Filter, adapted from ???. */
int      tSVFInit(tSVF *svf, float sr, SVFType type, uint16_t cutoffKnob, float Q);
float    tSVFTick(tSVF *svf, float v0);
int      tSVFSetFreq(tSVF *svf, uint16_t cutoffKnob);
int      tSVFSetQ(tSVF *svf, float Q);

/* Efficient State Variable Filter for 14-bit control input, [0, 4096). */
int      tSVFEfficientInit    (tSVFEfficient *svf, float sr, SVFType type, uint16_t cutoffKnob, float Q);
float    tSVFEfficientTick    (tSVF *svf, float v0);
int      tSVFEfficientSetFreq (tSVF *svf, uint16_t cutoffKnob);
int      tSVFEfficientSetQ    (tSVF *svf, float Q);

/* Simple Highpass filter */
int      tHighpassInit   (tHighpass *hp, float sr, float freq);
int      tHighpassFreq   (tHighpass *hp, float freq);
float    tHighpassTick   (tHighpass *hp, float x);

#pragma Time-based Utilities
/* Non-interpolating delay */
int         tDelayInit      (tDelay *d, uint32_t delay, uint32_t maxDelay, float *buff);
int         tDelaySetDelay  (tDelay *d, uint32_t delay);
uint32_t    tDelayGetDelay  (tDelay *d);
void        tDelayTapIn     (tDelay *d, float in, uint32_t tapDelay);
float       tDelayTapOut    (tDelay *d, uint32_t tapDelay);
float       tDelayAddTo     (tDelay *d, float value, uint32_t tapDelay);
float       tDelayTick      (tDelay *d, float sample);

float       tDelayGetLastOut(tDelay *d);
float       tDelayGetLastIn (tDelay *d);

/* Linearly-interpolating delay*/
int         tDelayLInit      (tDelayL *d, float delay, uint32_t maxDelay, float *buff);
int         tDelayLSetDelay  (tDelayL *d, float delay);
uint32_t    tDelayLGetDelay  (tDelayL *d);
void        tDelayLTapIn     (tDelayL *d, float in, uint32_t tapDelay);
float       tDelayLTapOut    (tDelayL *d, uint32_t tapDelay);
float       tDelayLAddTo     (tDelayL *d, float value, uint32_t tapDelay);
float       tDelayLTick      (tDelayL *d, float sample);

float       tDelayLGetLastOut(tDelayL *d);
float       tDelayLGetLastIn (tDelayL *d);

/* Allpass-interpolating delay*/
int         tDelayAInit      (tDelayA *d, float delay, uint32_t maxDelay, float *buff);
int         tDelayASetDelay  (tDelayA *d, float delay);
uint32_t    tDelayAGetDelay  (tDelayA *d);
void        tDelayATapIn     (tDelayA *d, float in, uint32_t tapDelay);
float       tDelayATapOut    (tDelayA *d, uint32_t tapDelay);
float       tDelayAAddTo     (tDelayA *d, float value, uint32_t tapDelay);
float       tDelayATick      (tDelayA *d, float sample);

float       tDelayAGetLastOut(tDelayA *d);
float       tDelayAGetLastIn (tDelayA *d);


/* Attack-Decay envelope */
int      tEnvelopeInit(tEnvelope *env, float sr, float attack, float decay, int loop,
                              const float *exponentialTable, const float *attackDecayIncTable);
int      tEnvelopeAttack(tEnvelope *env, float attack);
int      tEnvelopeDecay(tEnvelope *env, float decay);
int      tEnvelopeLoop(tEnvelope *env, int loop);
int      tEnvelopeOn(tEnvelope *env, float velocity);
float    tEnvelopeTick(tEnvelope *env);

/* Attack-Decay-Sustain-Release envelope. */
int tADSRInit(tADSR *d, float attack, float decay, float sustain, float release);

/* Ramp */
int      tRampInit   (tRamp *r, float sr, float time, int samples_per_tick);
int      tRampSetTime(tRamp *r, float time);
int      tRampSetDest(tRamp *r, float dest);
float    tRampTick   (tRamp *r);

#pragma Complex/Miscellaneous
/* Envelope Follower */
int      tEnvelopeFollowerInit           (tEnvelopeFollower *ef, float attackThreshold, float decayCoeff);
int      tEnvelopeFollowerDecayCoeff     (tEnvelopeFollower *ef, float decayCoeff);
int      tEnvelopeFollowerAttackThresh   (tEnvelopeFollower *ef, float attackThresh);
float    tEnvelopeFollowerTick           (tEnvelopeFollower *ef, float x);

/* PRCRev: Reverb, adapted from STK, algorithm by Perry Cook. */
int     tPRCRevInit      (tPRCRev *r, float sr, float t60, float delayBuffers[3][REV_DELAY_LENGTH]);
void    tPRCRevSetT60    (tPRCRev *r, float t60);
void    tPRCRevSetMix    (tPRCRev *r, float mix);
float   tPRCRevTick      (tPRCRev *r, float input);

/* NRev: Reverb, adpated from STK. */
int      tNRevInit   (tNRev *r, float sr, float t60, float delayBuffers[14][REV_DELAY_LENGTH]);
void     tNRevSetT60 (tNRev *r, float t60);
void     tNRevSetMix (tNRev *r, float mix);
float    tNRevTick   (tNRev *r, float input);


#endif  // CAUDIOLIBRARY_H_INCLUDED
