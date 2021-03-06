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

#define tick0(THIS)                         THIS.tick(&THIS)
#define tick1(THIS,IN)                      THIS.tick(&THIS,IN)

#define envOn(THIS,VEL)						THIS.on(&THIS,VEL)
#define setEnvelopeAttack(THIS, ATTACK)   	THIS.setAttack(&THIS, ATTACK)
#define setEnvelopeDecay(THIS,DECAY)		THIS.setDecay(&THIS,DECAY)

#define setRampDest(THIS,DEST)    			THIS.setDest(&THIS,DEST)

#define setFreq(THIS,FREQ)                  THIS.setFreq(&THIS,FREQ)
#define setFreqFromKnob(THIS,FREQ)    		THIS.setFreqFromKnob(&THIS,FREQ)

#define setQ(THIS,Q)						THIS.setQ(&THIS,Q)

#define setDecayCoeff(THIS,COEFF)           THIS.decayCoeff(&THIS,COEFF)

#define setDelay(THIS,DELAY)				THIS.setDelay(&THIS,DELAY)


#define VERY_SMALL_FLOAT 1.0e-38f

#define DELAY_BUFFER_SIZE 8192//16384


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
    
    float(*tick)(struct _tEnvelope *self);
    int(*on)(struct _tEnvelope *self, float velocity);
    int(*setDecay)(struct _tEnvelope *self, float decay);
    int(*setAttack)(struct _tEnvelope *self, float attack);
    int(*setLoop)(struct _tEnvelope *self, int loop);
} tEnvelope;

int tEnvelopeInit(tEnvelope *env,float sr, float attack, float decay, int loop, const float *exponentialTable, const float *attackDecayIncTable);

typedef struct _tADSR {
    
    float inv_sr;
    
    float(*tick)(struct _tEnvelope *self);
    int(*on)(struct _tEnvelope *self, float velocity);
    int(*off)(struct _tEnvelope *self);
    int(*setDecay)(struct _tEnvelope *self, float decay);
    int(*setAttack)(struct _tEnvelope *self, float attack);
    int(*setSustain)(struct _tEnvelope *self, float decay);
    int(*setRelease)(struct _tEnvelope *self, float attack);
    
} tADSR;

int tADSRInit(tADSR *d, float attack, float decay, float sustain, float release);

typedef struct _tDelay {
    
    uint32_t in_index, out_index;
    float bottomFrac,topFrac;
    float *buff;
    float(*tick)(struct _tDelay *self, float x);
    int(*setDelay)(struct _tDelay *self, float delay);
} tDelay;

int tDelayInit(tDelay *d, float *buff);


/* SVF Type */
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
    float(*tick)(struct _tSVF *self, float v0);
    int(*setFreqFromKnob)(struct _tSVF *self, uint16_t cutoffKnob);
    int(*setQ)(struct _tSVF *self, float Q);
} tSVF;

// 0-4096 is mapped to midi notes 16-130 for cutoff frequency
int tSVFInit(tSVF *svf, float sr, SVFType type, uint16_t cutoffKnob, float Q);

typedef struct _tSVFEfficient {
    SVFType type;
    float inv_sr;
    float cutoff, Q;
    float ic1eq,ic2eq;
    float g,k,a1,a2,a3;
    float(*tick)(struct _tSVF *self, float v0);
    int(*setFreqFromKnob)(struct _tSVF *self, uint16_t cutoffKnob);
    int(*setQ)(struct _tSVF *self, float Q);
} tSVFEfficient;

// 0-4096 is mapped to midi notes 16-130 for cutoff frequency
int tSVFEfficientInit(tSVFEfficient *svf, float sr, SVFType type, uint16_t cutoffKnob, float Q);

/* Envelope Follower */
typedef struct _tEnvelopeFollower {
    float y, a_thresh, d_coeff;
    int(*attackThresh)(struct _tEnvelopeFollower *self, float attackThresh);
    int(*decayCoeff)(struct _tEnvelopeFollower *self, float decayCoeff);
    float(*tick)(struct _tEnvelopeFollower *self, float x);
} tEnvelopeFollower;

int tEnvelopeFollowerInit(tEnvelopeFollower *ef, float attackThreshold, float decayCoeff);

/* Phasor [0.0, 1.0) */
typedef struct _tPhasor {
    float phase;
    float inc;
    float inv_sr;
    int(*setFreq)(struct _tPhasor *self, float freq);
    float(*tick)(struct _tPhasor *self);
} tPhasor;

int tPhasorInit(tPhasor *p, float sr);

/* Cycle */
typedef struct _tCycle {
    // Underlying phasor
    float phase;
    float inc;
    float inv_sr;
    
    // Wavetable synthesis
    const float *wt; //wavetable
    int wtlen; //wavetable length
    int(*setFreq)(struct _tCycle *self, float freq);
    float(*tick)(struct _tCycle *self);
} tCycle;

int tCycleInit(tCycle *c, float sr, const float *table, int len);

/* Sawtooth */
typedef struct _tSawtooth{
    // Underlying phasor
    float phase;
    float inc;
    float inv_sr;
    
    int(*setFreq)(struct _tSawtooth *self, float freq);
    float(*tick)(struct _tSawtooth *self);
} tSawtooth;

int tSawtoothInit(tSawtooth *t, float sr);

/* Triangle */
typedef struct _tTriangle{
    // Underlying phasor
    float phase;
    float inc;
    float inv_sr;
    
    int(*setFreq)(struct _tTriangle *self, float freq);
    float(*tick)(struct _tTriangle *self);
} tTriangle;

int tTriangleInit(tTriangle *t, float sr);

/* Pulse */
typedef struct _tPulse{
    // Underlying phasor
    float phase;
    float inc;
    float inv_sr;
    
    float pw;
    int(*pwidth)(struct _tPulse *self, float pwidth);
    int(*setFreq)(struct _tPulse *self, float freq);
    float(*tick)(struct _tPulse *self);
} tPulse;

int tPulseInit(tPulse *t, float sr, float pwidth);

/* Noise */
typedef enum NoiseType {
    NoiseTypeWhite=0,
    NoiseTypePink,
    NoiseTypeNil,
} NoiseType;

typedef struct _tNoise {
    float pinkb0, pinkb1, pinkb2;
    float(*rand)();
    float(*tick)(struct _tNoise *self);
} tNoise;

int tNoiseInit(tNoise *c, float sr, float (*randomNumberGenerator)(), NoiseType type);

typedef struct _tHighpass {
    float inv_sr;
    float xs, ys, R;
    float cutoff;
    int(*setFreq)(struct _tHighpass *self, float x);
    float(*tick)(struct _tHighpass *self, float x);
} tHighpass;

int tHighpassInit(tHighpass *hp, float sr, float freq);

#endif  // CAUDIOLIBRARY_H_INCLUDED
