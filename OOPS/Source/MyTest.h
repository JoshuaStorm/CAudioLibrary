/*
  ==============================================================================

    MyTest.h
    Created: 18 Jan 2017 11:55:17am
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef MYTEST_H_INCLUDED
#define MYTEST_H_INCLUDED

#define YIN_BUFFER_LENGTH 2048 //higher better for low frequencies

// Yin stuff
Yin yin;
int yindex = 0;
int16_t forYin[YIN_BUFFER_LENGTH];
float finalPitch;
float maxProb;
void    yinPitchDetect  (void);

// OOPS stuff
float nRevDelayBufs[14][REV_DELAY_LENGTH];
float prcRevDelayBufs[3][REV_DELAY_LENGTH];

float skBuff[2][REV_DELAY_LENGTH];

float sampleRate;

float gGain;
float gFund;
float gAttack,gDecay, gSustain, gRelease;
float gMidiNote;
bool  gTrigger, gDSR, gHSR;

WaveformType waveform = Sine;

#define NUM_OSC 10
tRamp ramp;
tTriangle tri[NUM_OSC];
tSquare sqr[NUM_OSC];
tCycle cyc[NUM_OSC];
tSawtooth saw[NUM_OSC];
tEnvelope env;
float oscGain[NUM_OSC] = {.5, .2, .05, .05, .1, .025, .025, .025, .0125, .0125};

float sampleRateGlobal;




#endif  // MYTEST_H_INCLUDED
