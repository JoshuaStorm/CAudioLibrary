/*
  ==============================================================================

    OOPSTest1.cpp
    Created: 18 Jan 2017 10:37:04am
    Author:  Michael R Mulshine

  ==============================================================================
*/

#include "OOPSTest.h"
#include "MyTest.h"



void OOPSTest_init(float sampleRate)
{
    DBG("Initializing C Audio Library Test.");
    
    sampleRateGlobal = sampleRate;

    
    OOPSInit(sampleRate, &randomNumberGenerator);

    for (int i = 0; i < NUM_OSC; i++)
    {
        tTriangleInit(&tri[i]);
        tTriangleSetFreq(&tri[i], 100.0f * (i+1));
    
        tCycleInit(&cyc[i]);
        tCycleSetFreq(&cyc[i], 100.0f * (i+1));
        
        tSquareInit(&sqr[i]);
        tSquareSetFreq(&sqr[i], 100.0f * (i+1));

        tSawtoothInit(&saw[i]);
        tSawtoothSetFreq(&saw[i], 100.0f * (i+1));
    }
    
    gGain = 0.5f;
    
    setSliderValue("Gain", gGain);
    
    tEnvelopeInit(&env, 5, 500, 0, exp_decay, attack_decay_inc);
    
    tRampInit(&ramp, 20.0f, 1);
    
}

// Sample-rate callback.
float OOPSTest_tick(float input)
{
    float sample    = 0.0f;
    
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
    
    sample *= tEnvelopeTick(&env);
    // Gain ramp.
    sample *= tRampTick(&ramp);
    
    if (++yindex == YIN_BUFFER_LENGTH) yindex = 0;
    
    forYin[yindex] = (int16_t)(sample * INT16_MAX);
    

    return sample;
    
}


void    OOPSTest_noteOn          (int midiNoteNumber, float velocity)
{
    DBG("noteOn: " + String(midiNoteNumber) + " " + String(velocity));
    
    float freq = OOPSMtoF(midiNoteNumber);
    if (gMidiNote != freq)
    {
        gMidiNote = freq;
        
        for (int i = 0; i < NUM_OSC; i++)
        {
            tTriangleSetFreq(&tri[i], gMidiNote * (i+1));
            
            tSquareSetFreq(&sqr[i], gMidiNote * (i+1));
            
            tSawtoothSetFreq(&saw[i], gMidiNote * (i+1));
            
            tCycleSetFreq(&cyc[i], gMidiNote *  (i+1));
            
        }
        
        DBG("OscPitch: " + String(10.0f + gFund * 1000.0f));
    }
    
    tEnvelopeOn(&env, velocity);
}

void    OOPSTest_noteOff         (int midiNoteNumber)
{
    DBG("noteOff: " + String(midiNoteNumber));
}

// Block-rate callback.
void OOPSTest_block(void)
{
    float val = 0.0f;
    
    
    // Trigger
    gTrigger = getButtonState("Yin");
    
    if (gTrigger)
    {
        setButtonState("Yin", false);
        yinPitchDetect();
    }

    gDSR = getButtonState("DoubleSR");
    
    if (gDSR)
    {
        setButtonState("DoubleSR", false);
        sampleRateGlobal *= 2;
        OOPSSetSampleRate(sampleRateGlobal);
        
        DBG("sampleRateGlobal: " + String(sampleRateGlobal)); 
    }
    
    gHSR = getButtonState("HalfSR");
    
    if (gHSR)
    {
        setButtonState("HalfSR", false);
        sampleRateGlobal /= 2;
        OOPSSetSampleRate(sampleRateGlobal);
        
        DBG("sampleRateGlobal: " + String(sampleRateGlobal));
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
    
    val = getSliderValue("Attack");
    
    if (gAttack != val)
    {
        gAttack = val;
        
        tEnvelopeSetAttack(&env, 5.0f + gAttack * 2000.0f );
    }
    
    val = getSliderValue("Decay");
    
    if (gDecay != val)
    {
        gDecay = val;
        
        tEnvelopeSetDecay(&env, 5.0f + gDecay * 2000.0f );
    }
    
    WaveformType state = (WaveformType) getComboBoxState("Waveform");
    
    if (waveform != state)
    {
        waveform = state;
        
        DBG("waveform: " + cWaveformTypes[waveform]);
    }
    
}

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
