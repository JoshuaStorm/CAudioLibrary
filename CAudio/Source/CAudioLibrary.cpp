/*
  ==============================================================================

    CAudioLibrary.cpp
    Created: 4 Dec 2016 9:26:17pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#include "CAudioLibrary.h"

// The C-embedded Audio Library.
#include "Wavetables.h"

#include "math.h"

#define TWO_TO_16 65536.f

/* Envelope */


#define EXPONENTIAL_TABLE_SIZE 65536


#define COMPUTE_INC() r->inc = ((r->dest-r->curr)/r->time * r->inv_sr_ms)*((float)r->samples_per_tick)



float dbuf1[DELAY_BUFFER_LENGTH_2];
float dbuf2[DELAY_BUFFER_LENGTH_2];
float dbuf3[DELAY_BUFFER_LENGTH_2];

typedef enum TableName
{
    T20 = 0,
    T40,
    T80,
    T160,
    T320,
    T640,
    T1280,
    T2560,
    T5120,
    T10240,
    T20480,
    TableNameNil
} TableName;

float   clipAU(float min, float val, float max) {
    
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    } else {
        return val;
    }
}

int     isPrimeAU(uint64 number )
{
    if ( number == 2 ) return true;
    if ( number & 1 ) {
        for ( int i=3; i<(int)sqrt((double)number)+1; i+=2 )
            if ( (number % i) == 0 ) return false;
        return true; // prime
    }
    else return false; // even
}
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ PRCRev ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //

void    tPRCRevSetT60(tPRCRev *r, float t60)
{
    if ( t60 <= 0.0 ) t60 = 0.001f;
    
    r->combCoeff = pow(10.0, (-3.0 * tDelayGetDelay(&r->combDelay) * r->inv_sr / t60 ));

}

void    tPRCRevSetMix(tPRCRev *r, float mix)
{
    r->mix = mix;
}

float   tPRCRevTick(tPRCRev *r, float input)
{
    float temp, temp0, temp1, temp2;
    float out;
    
    r->lastIn = input;
    
    temp = tDelayGetLastOut(&r->allpassDelays[0]);
    temp0 = r->allpassCoeff * temp;
    temp0 += input;
    tDelayTick(&r->allpassDelays[0], temp0);
    temp0 = -(r->allpassCoeff * temp0) + temp;
    
    temp = tDelayGetLastOut(&r->allpassDelays[1]);
    temp1 = r->allpassCoeff * temp;
    temp1 += temp0;
    tDelayTick(&r->allpassDelays[1], temp1);
    temp1 = -(r->allpassCoeff * temp1) + temp;
    
    temp2 = temp1 + ( r->combCoeff * tDelayGetLastOut(&r->combDelay));
    
    out = r->mix * tDelayTick(&r->combDelay,temp2);
    
    temp = (1.0 - r->mix) * input;
    
    out += temp;

    r->lastOut = out;
    
    return out;
}


int     tPRCRevInit(tPRCRev *r, float sr, float t60, float delayBuffers[3][DELAY_BUFFER_LENGTH_2])
{
    if (t60 <= 0.0) t60 = 0.001f;
    
    r->inv_sr = 1.0f/sr;
    r->inv_441 = 1.0f/44100.0f;
    
    int lengths[4] = { 341, 613, 1557, 2137 }; // Delay lengths for 44100 Hz sample rate.
    double scaler = sr * r->inv_441;
    
    int delay, i;
    if (scaler != 1.0)
    {
        for (i=0; i<4; i++)
        {
            delay = (int) scaler * lengths[i];
            if ( (delay & 1) == 0)
                delay++;
            while ( !isPrimeAU(delay) )
                delay += 2;
            lengths[i] = delay;
        }
    }
    
    tDelayInit(&r->allpassDelays[0], delayBuffers[0]);
    tDelayInit(&r->allpassDelays[1], delayBuffers[1]);
    tDelayInit(&r->combDelay, delayBuffers[2]);
    
    tDelaySetDelay(&r->allpassDelays[0], lengths[0]);
    tDelaySetDelay(&r->allpassDelays[1], lengths[1]);
    tDelaySetDelay(&r->combDelay, lengths[2]);
    
    tPRCRevSetT60(r, t60);
    r->allpassCoeff = 0.7f;
    r->mix = 0.5f;
    
    //CLEAR delays 
    
    return 1;
}

void    tNRevSetT60(tNRev *r, float t60)
{
    if (t60 <= 0.0)           t60 = 0.001f;
    
    for (int i=0; i<6; i++)   r->combCoeffs[i] = pow(10.0, (-3.0 * tDelayGetDelay(&r->combDelays[i]) * r->inv_sr / t60 ));
    
}

void    tNRevSetMix(tNRev *r, float mix)
{
    r->mix = mix;
}

float   tNRevTick(tNRev *r, float input)
{
    r->lastIn = input;
    
    float temp, temp0, temp1, temp2, out;
    int i;
    
    temp0 = 0.0;
    for ( i=0; i<6; i++ )
    {
        temp = input + (r->combCoeffs[i] * tDelayGetLastOut(&r->combDelays[i]));
        temp0 += tDelayTick(&r->combDelays[i],temp);
    }
    
    for ( i=0; i<3; i++ )
    {
        temp = tDelayGetLastOut(&r->allpassDelays[i]);
        temp1 = r->allpassCoeff * temp;
        temp1 += temp0;
        tDelayTick(&r->allpassDelays[i],temp1);
        temp0 = -(r->allpassCoeff * temp1) + temp;
    }
    
    // One-pole lowpass filter.
    r->lowpassState = 0.7f * r->lowpassState + 0.3f * temp0;
    temp = tDelayGetLastOut(&r->allpassDelays[3]);
    temp1 = r->allpassCoeff * temp;
    temp1 += r->lowpassState;
    tDelayTick(&r->allpassDelays[3], temp1 );
    temp1 = -(r->allpassCoeff * temp1) + temp;
    
    temp = tDelayGetLastOut(&r->allpassDelays[4]);
    temp2 = r->allpassCoeff * temp;
    temp2 += temp1;
    tDelayTick(&r->allpassDelays[4], temp2 );
    out = r->mix * ( -( r->allpassCoeff * temp2 ) + temp );
    
    /*
    temp = tDelayGetLastOut(&r->allpassDelays[5]);
    temp3 = r->allpassCoeff * temp;
    temp3 += temp1;
    tDelayTick(&r->allpassDelays[5], temp3 );
    lastFrame_[1] = effectMix_*( - ( r->allpassCoeff * temp3 ) + temp );
     */
    
    temp = ( 1.0f - r->mix ) * input;
    
    out += temp;
    
    r->lastOut = out;
    
    return out;
}


int     tNRevInit(tNRev *r, float sr, float t60, float delayBuffers[14][DELAY_BUFFER_LENGTH_2])
{
    if (t60 <= 0.0) t60 = 0.001f;
    
    r->inv_sr = 1.0f/sr;
    r->inv_441 = 1.0f/44100.0f;
    
    int lengths[15] = {1433, 1601, 1867, 2053, 2251, 2399, 347, 113, 37, 59, 53, 43, 37, 29, 19}; // Delay lengths for 44100 Hz sample rate.
    double scaler = sr / 25641.0f;
    
    int delay, i;
    
    for (i=0; i < 15; i++)
    {
        delay = (int) scaler * lengths[i];
        if ( (delay & 1) == 0)
            delay++;
        while ( !isPrimeAU(delay) )
            delay += 2;
        lengths[i] = delay;
    }
    
    for ( i=0; i<6; i++ )
    {
        tDelayInit(&r->combDelays[i], delayBuffers[i]);
        tDelaySetDelay(&r->combDelays[i], lengths[i]);
        r->combCoeffs[i] = pow(10.0, (-3 * lengths[i] * r->inv_sr / t60));
    }
    
    for ( i=0; i<8; i++ )
    {
        tDelayInit(&r->allpassDelays[i], delayBuffers[i+6]);
        tDelaySetDelay(&r->allpassDelays[i], lengths[i+6]);
    }
    
    for ( i=0; i<2; i++ )
    {
        tDelaySetDelay(&r->allpassDelays[i], lengths[i]);
        tDelaySetDelay(&r->combDelays[i], lengths[i+2]);
    }
    
    tNRevSetT60(r, t60);
    r->allpassCoeff = 0.7f;
    r->mix = 0.3f;

    return 0;
}

#pragma mark - Filters
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ OneZero Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tOneZeroInit(tOneZero *f, float theZero)
{
    f->gain = 1.0f;
    f->lastIn = 0.0f;
    f->lastOut = 0.0f;
    tOneZeroSetZero(f, theZero);
    
    return 0;
}

float   tOneZeroTick(tOneZero *f, float input)
{
    float in = input * f->gain;
    float out = f->b1 * f->lastIn + f->b0 * in;
    
    f->lastIn = in;
    
    return out;
}

void    tOneZeroSetZero(tOneZero *f, float theZero)
{
    if (theZero > 0.0f) f->b0 = 1.0f / (1.0f + theZero);
    else                f->b0 = 1.0f / (1.0f - theZero);
    
    f->b1 = -theZero * f->b0;
    
}

void    tOneZeroSetB0(tOneZero *f, float b0)
{
    f->b0 = b0;
}

void    tOneZeroSetB1(tOneZero *f, float b1)
{
    f->b1 = b1;
}

void    tOneZeroSetCoefficients(tOneZero *f, float b0, float b1)
{
    f->b0 = b0;
    f->b1 = b1;
}

void    tOneZeroSetGain(tOneZero *f, float gain)
{
    f->gain = gain;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ OneZero Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tTwoZeroInit(tTwoZero *f, float sr)
{
    f->gain = 1.0f;
    f->lastIn[0] = 0.0f;
    f->lastIn[1] = 0.0f;
    f->sr = sr;
    f->inv_sr = 1.0f/sr;
    
    return 0;
}

float   tTwoZeroTick(tTwoZero *f, float input)
{
    float in = input * f->gain;
    float out = f->b2 * f->lastIn[1] + f->b1 * f->lastIn[0] + f->b0 * in;
    
    f->lastIn[1] = f->lastIn[0];
    f->lastIn[0] = in;
    
    return out;
}

void    tTwoZeroSetNotch(tTwoZero *f, float freq, float radius)
{
    // Should also deal with frequency being > half sample rate / nyquist. See STK
    if (freq < 0.0f)    freq = 0.0f;
    if (radius < 0.0f)  radius = 0.0f;
    
    f->b2 = radius * radius;
    f->b1 = -2.0f * radius * cosf(TWO_PI * freq * f->inv_sr); // OPTIMIZE with LOOKUP or APPROXIMATION
    
    // Normalize the filter gain. From STK.
    if ( f->b1 > 0.0f ) // Maximum at z = 0.
        f->b0 = 1.0f / ( 1.0f + f->b1 + f->b2 );
    else            // Maximum at z = -1.
        f->b0 = 1.0f / ( 1.0f - f->b1 + f->b2 );
    f->b1 *= f->b0;
    f->b2 *= f->b0;

}

void    tTwoZeroSetB0(tTwoZero *f, float b0)
{
    f->b0 = b0;
}

void    tTwoZeroSetB1(tTwoZero *f, float b1)
{
    f->b1 = b1;
}

void    tTwoZeroSetCoefficients(tTwoZero *f, float b0, float b1, float b2)
{
    f->b0 = b0;
    f->b1 = b1;
    f->b2 = b2;
}

void    tTwoZeroSetGain(tTwoZero *f, float gain)
{
    f->gain = gain;
}


// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ OnePole Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //

void    tOnePoleSetB0(tOnePole *f, float b0)
{
    f->b0 = b0;
}

void    tOnePoleSetA1(tOnePole *f, float a1)
{
    if (a1 >= 1.0f)     a1 = 0.999999f;
    
    f->a1 = a1;
}

void    tOnePoleSetPole(tOnePole *f, float thePole)
{
    if (thePole >= 1.0f)    thePole = 0.999999f;
    
    // Normalize coefficients for peak unity gain.
    if (thePole > 0.0f)     f->b0 = (1.0f - thePole);
    else                    f->b0 = (1.0f + thePole);
    
    f->a1 = -thePole;
}

void    tOnePoleSetCoefficients(tOnePole *f, float b0, float a1)
{
    if (a1 >= 1.0f)     a1 = 0.999999f;
    
    f->b0 = b0;
    f->a1 = a1;
}

void    tOnePoleSetGain(tOnePole *f, float gain)
{
    f->gain = gain;
}

float   tOnePoleTick(tOnePole *f, float input)
{
    float in = input * f->gain;
    float out = (f->b0 * in) - (f->a1 * f->lastOut);
    
    f->lastIn = in;
    f->lastOut = out;
    
    return out;
}

int     tOnePoleInit(tOnePole *f, float thePole)
{
    f->gain = 1.0f;
    f->a0 = 1.0;
    
    tOnePoleSetPole(f, thePole);
    
    f->lastIn = 0.0f;
    f->lastOut = 0.0f;
    
    
    return 0;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ TwoPole Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tTwoPoleInit(tTwoPole *f, float sr)
{
    f->gain = 1.0f;
    f->a0 = 1.0;
    f->b0 = 1.0;
    
    f->sr = sr;
    f->inv_sr = 1.0f / sr;
    
    f->lastOut[0] = 0.0f;
    f->lastOut[1] = 0.0f;
    
    return 1;
}

float   tTwoPoleTick(tTwoPole *f, float input)
{
    float in = input * f->gain;
    float out = (f->b0 * in) - (f->a1 * f->lastOut[0]) - (f->a2 * f->lastOut[1]);
    
    f->lastOut[1] = f->lastOut[0];
    f->lastOut[0] = out;
    
    return out;
}

void    tTwoPoleSetB0(tTwoPole *f, float b0)
{
    f->b0 = b0;
}

void    tTwoPoleSetA1(tTwoPole *f, float a1)
{
    f->a1 = a1;
}

void    tTwoPoleSetA2(tTwoPole *f, float a2)
{
    f->a2 = a2;
}


void    tTwoPoleSetResonance(tTwoPole *f, float frequency, float radius, int normalize)
{
    if (frequency < 0.0f)   frequency = 0.0f; // need to also handle when frequency > nyquist
    if (radius < 0.0f)      radius = 0.0f;
    if (radius >= 1.0f)     radius = 0.999999f;
    
    f->a2 = radius * radius;
    f->a1 =  -2.0f * radius * cos(TWO_PI * frequency * f->inv_sr);
    
    if ( normalize )
    {
        // Normalize the filter gain ... not terribly efficient.
        float real = 1 - radius + (f->a2 - radius) * cos(TWO_PI * 2 * frequency * f->inv_sr);
        float imag = (f->a2 - radius) * sin(TWO_PI * 2 * frequency * f->inv_sr);
        f->b0 = sqrt( pow(real, 2) + pow(imag, 2) );
        
        // NEED TO OPTIMIZE and make sure sqrt/pow/cos are compatible. How do we want to handle this for platforms that it won't be compatible on?
    }
}

void    tTwoPoleSetCoefficients(tTwoPole *f, float b0, float a1, float a2)
{
    f->b0 = b0;
    f->a1 = a1;
    f->a2 = a2;
}

void    tTwoPoleSetGain(tTwoPole *f, float gain)
{
    f->gain = gain;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ PoleZero Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //

void    tPoleZeroSetB0(tPoleZero *pzf, float b0)
{
    pzf->b0 = b0;
}

void    tPoleZeroSetB1(tPoleZero *pzf, float b1)
{
    pzf->b1 = b1;
}

void    tPoleZeroSetA1(tPoleZero *pzf, float a1)
{
    if (a1 >= 1.0f) // a1 should be less than 1.0
    {
        a1 = 0.999999f;
    }
    
    pzf->a1 = a1;
}

void    tPoleZeroSetCoefficients(tPoleZero *pzf, float b0, float b1, float a1)
{
    if (a1 >= 1.0f) // a1 should be less than 1.0
    {
        a1 = 0.999999f;
    }
    
    pzf->b0 = b0;
    pzf->b1 = b1;
    pzf->a1 = a1;
}

void    tPoleZeroSetAllpass(tPoleZero *pzf, float coeff)
{
    if (coeff >= 1.0f) // allpass coefficient >= 1.0 makes filter unstable
    {
        coeff = 0.999999f;
    }
    
    pzf->b0 = coeff;
    pzf->b1 = 1.0f;
    pzf->a0 = 1.0f;
    pzf->a1 = coeff;
}

void    tPoleZeroSetBlockZero(tPoleZero *pzf, float thePole)
{
    if (thePole >= 1.0f) // allpass coefficient >= 1.0 makes filter unstable
    {
        thePole = 0.999999f;
    }
    
    pzf->b0 = 1.0f;
    pzf->b1 = -1.0f;
    pzf->a0 = 1.0f;
    pzf->a1 = -thePole;
}

void    tPoleZeroSetGain(tPoleZero *pzf, float gain)
{
    pzf->gain = gain;
}

float   tPoleZeroTick(tPoleZero *pzf, float input)
{
    float in = input * pzf->gain;
    float out = (pzf->b0 * in) + (pzf->b1 * pzf->lastIn) - (pzf->a1 * pzf->lastOut);
    
    pzf->lastIn = in;
    pzf->lastOut = out;
    
    return out;
}

int     tPoleZeroInit(tPoleZero *pzf)
{
    pzf->gain = 1.0f;
    pzf->b0 = 1.0;
    pzf->a0 = 1.0;
    
    pzf->lastIn = 0.0f;
    pzf->lastOut = 0.0f;
    
    return 1;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ BiQuad Filter ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tBiQuadInit(tBiQuad *f, float sr)
{
    f->gain = 1.0f;

    f->sr = sr;
    f->inv_sr = 1.0f/sr;
    
    f->b0 = 0.0f;
    f->a0 = 0.0f;
    
    f->lastIn[0] = 0.0f;
    f->lastIn[1] = 0.0f;
    f->lastOut[0] = 0.0f;
    f->lastOut[1] = 0.0f;
    
    return 0;
}

float   tBiQuadTick(tBiQuad *f, float input)
{
    float in = input * f->gain;
    float out = f->b0 * in + f->b1 * f->lastIn[0] + f->b2 * f->lastIn[1];
    out -= f->a2 * f->lastOut[1] + f->a1 * f->lastOut[0];
    
    f->lastIn[1] = f->lastIn[0];
    f->lastIn[0] = in;
    
    f->lastOut[1] = f->lastOut[0];
    f->lastOut[0] = out;
    
    return out;
}

void    tBiQuadSetResonance(tBiQuad *f, float freq, float radius, int normalize)
{
    // Should also deal with frequency being > half sample rate / nyquist. See STK
    if (freq < 0.0f)    freq = 0.0f;
    if (radius < 0.0f)  radius = 0.0f;
    if (radius >= 1.0f)  radius = 1.0f;
    
    f->a2 = radius * radius;
    f->a1 = -2.0f * radius * cosf(TWO_PI * freq * f->inv_sr);
    
    if (normalize)
    {
        f->b0 = 0.5f - 0.5f * f->a2;
        f->b1 = 0.0f;
        f->b2 = -f->b0;
    }
}

void    tBiQuadSetNotch(tBiQuad *f, float freq, float radius)
{
    // Should also deal with frequency being > half sample rate / nyquist. See STK
    if (freq < 0.0f)    freq = 0.0f;
    if (radius < 0.0f)  radius = 0.0f;
    
    f->b2 = radius * radius;
    f->b1 = -2.0f * radius * cosf(TWO_PI * freq * f->inv_sr); // OPTIMIZE with LOOKUP or APPROXIMATION
    
    // Does not attempt to normalize filter gain.
}

void tBiQuadSetEqualGainZeros(tBiQuad *f)
{
    f->b0 = 1.0f;
    f->b1 = 0.0f;
    f->b2 = -1.0f;
}

void    tBiQuadSetB0(tBiQuad *f, float b0)
{
    f->b0 = b0;
}

void    tBiQuadSetB1(tBiQuad *f, float b1)
{
    f->b1 = b1;
}

void    tBiQuadSetB2(tBiQuad *f, float b2)
{
    f->b2 = b2;
}

void    tBiQuadSetA1(tBiQuad *f, float a1)
{
    f->a1 = a1;
}

void    tBiQuadSetA2(tBiQuad *f, float a2)
{
    f->a2 = a2;
}

void    tBiQuadSetCoefficients(tBiQuad *f, float b0, float b1, float b2, float a1, float a2)
{
    f->b0 = b0;
    f->b1 = b1;
    f->b2 = b2;
    f->a1 = a1;
    f->a2 = a2;
}

void    tBiQuadSetGain(tBiQuad *f, float gain)
{
    f->gain = gain;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Delay ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tDelaySetDelay(tDelay *d, float delay)
{
    float outPointer;
    
    outPointer = (float)d->in_index - delay;  // read chases write
    
    while (outPointer < 0) {
        outPointer += DELAY_BUFFER_SIZE; // modulo maximum length
    }
    
    d->out_index = (uint32_t) outPointer;  // integer part
    d->bottomFrac = outPointer - (float)d->out_index; // fractional part
    d->topFrac = 1.0f - d->bottomFrac;
    
    d->delay = delay;
    
    return 0;
}

float   tDelayGetDelay(tDelay *d)
{
    return d->delay;
}

float   tDelayGetLastOut(tDelay *d)
{
    return d->lastOut;
}

float   tDelayGetLastIn(tDelay *d)
{
    return d->lastIn;
}

float   tDelayTick(tDelay *d, float sample)
{
    float output_sample;
    d->buff[(d->in_index)] = sample;
    d->lastIn = sample;
    
    /* Interpolation */
    output_sample = d->buff[d->out_index] * d->topFrac;
    if (d->out_index+1 < DELAY_BUFFER_SIZE)
        output_sample += d->buff[d->out_index+1] * d->bottomFrac;
    else
        output_sample += d->buff[0] * d->bottomFrac;
    
    // Increment pointers.
    d->in_index = d->in_index + 1;
    d->out_index = d->out_index  +1;
    if (d->in_index == DELAY_BUFFER_SIZE) d->in_index -= DELAY_BUFFER_SIZE;
    if (d->out_index >= DELAY_BUFFER_SIZE) d->out_index -= DELAY_BUFFER_SIZE;
    
    d->lastOut = output_sample;
    
    return output_sample;
}


/* Delay */
int     tDelayInit(tDelay *d, float *buff)
{
    d->buff = buff;
    d->bottomFrac = 0.0f;
    d->topFrac = 0.0f;
    d->in_index = 0;
    d->out_index = 0;
    d->lastIn = 0.0f;
    d->lastOut = 0.0f;
    d->delay = 0.001f;
    
    return 0;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Envelope ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
int     tEnvelopeAttack(tEnvelope *env, float attack)
{
    uint16_t attackIndex;
    
    if (attack < 0) {
        attackIndex = 0;
    } else if (attack < 8192) {
        attackIndex = ((uint16_t)(attack * 8.0f))-1;
    } else {
        attackIndex = UINT16_MAX;
    }
    
    env->attackInc = env->inc_buff[attackIndex];
    
    return 0;
}

int     tEnvelopeDecay(tEnvelope *env, float decay)
{
    uint16_t decayIndex;
    
    if (decay < 0) {
        decayIndex = 0;
    } else if (decay < 8192) {
        decayIndex = ((uint16_t)(decay * 8.0f))-1;
    } else {
        decayIndex = ((uint16_t)(8192 * 8.0f))-1;
    }
    
    env->decayInc = env->inc_buff[decayIndex];

    
    return 0;
}

int     tEnvelopeLoop(tEnvelope *env, int loop)
{
    env->loop = loop;
    
    return 0;
}


int     tEnvelopeOn(tEnvelope *env, float velocity)
{
    if (env->inAttack || env->inDecay) // In case envelope retriggered while it is still happening.
    {
        env->rampPhase = 0;
        env->inRamp = 1;
        env->rampPeak = env->next;
    }
    else // Normal start.
    {
        env->inAttack = 1;
    }
    
    
    env->attackPhase = 0;
    env->decayPhase = 0;
    env->inDecay = 0;
    env->gain = velocity;
    
    return 0;
}

float   tEnvelopeTick(tEnvelope *env)
{
    if (env->inRamp)
    {
        if (env->rampPhase > UINT16_MAX)
        {
            env->inRamp = 0;
            env->inAttack = 1;
            env->next = 0.0f;
        }
        else
        {
            env->next = env->rampPeak * env->exp_buff[(uint32_t)env->rampPhase];
        }
        
        env->rampPhase += env->rampInc;
    }

    if (env->inAttack)
    {
    
        // If attack done, time to turn around.
        if (env->attackPhase > UINT16_MAX)
        {
            env->inDecay = 1;
            env->inAttack = 0;
            env->next = env->gain * 1.0f;
        }
        else
        {
            // do interpolation !
            env->next = env->gain * env->exp_buff[UINT16_MAX - (uint32_t)env->attackPhase]; // inverted and backwards to get proper rising exponential shape/perception
        }
        
        // Increment envelope attack.
        env->attackPhase += env->attackInc;
        
    }
    
    if (env->inDecay)
    {
    
        // If decay done, finish.
        if (env->decayPhase >= UINT16_MAX)
        {
            env->inDecay = 0;
            
            if (env->loop)
            {
                env->attackPhase = 0;
                env->decayPhase = 0;
                env->inAttack = 1;
            }
            else
            {
               env->next = 0.0f;
            }
            
        } else {
            
            env->next = env->gain * (env->exp_buff[(uint32_t)env->decayPhase]); // do interpolation !
        }
        
        // Increment envelope decay;
        env->decayPhase += env->decayInc;
    }

    return env->next;
}

int     tEnvelopeInit(tEnvelope *env, float sr, float attack, float decay, int loop,
                      const float *exponentialTable, const float *attackDecayIncTable)
{
    env->inv_sr = 1.0f/sr;
    env->exp_buff = exponentialTable;
    env->inc_buff = attackDecayIncTable;
    env->buff_size = sizeof(exponentialTable);
    
    env->loop = loop;
    
    if (attack > 8192)
        attack = 8192;
    if (attack < 0)
        attack = 0;
    
    if (decay > 8192)
        decay = 8192;
    if (decay < 0)
        decay = 0;
    
    uint16_t attackIndex = ((uint16_t)(attack * 8.0f))-1;
    uint16_t decayIndex = ((uint16_t)(decay * 8.0f))-1;
    uint16_t rampIndex = ((uint16_t)(2.0f * 8.0f))-1;
    
    if (attackIndex < 0)
        attackIndex = 0;
    if (decayIndex < 0)
        decayIndex = 0;
    if (rampIndex < 0)
        rampIndex = 0;
    
    env->inRamp = 0;
    env->inAttack = 0;
    env->inDecay = 0;
    
    env->attackInc = env->inc_buff[attackIndex];
    env->decayInc = env->inc_buff[decayIndex];
    env->rampInc = env->inc_buff[rampIndex];
    
    return 0;
}

/* Phasor */
int     tPhasorFreq(tPhasor *p, float freq)
{
    p->inc = freq * p->inv_sr;
    
    return 0;
}

float   tPhasorTick(tPhasor *p)
{
    p->phase += p->inc;
    
    if (p->phase >= 1.0f) p->phase -= 1.0f;
    
    return p->phase;
}

int     tPhasorInit(tPhasor *p, float sr)
{
    p->phase = 0.0f;
    p->inc = 0.0f;
    p->inv_sr = 1.0f/sr;
    
    return 0;
}

float   tSVFTick(tSVF *svf, float v0)
{
    float v1,v2,v3;
    float low,high;
    v3 = v0 - svf->ic2eq;
    v1 = (svf->a1 * svf->ic1eq) + (svf->a2 * v3);
    v2 = svf->ic2eq + (svf->a2 * svf->ic1eq) + (svf->a3 * v3);
    svf->ic1eq = (2.0f * v1) - svf->ic1eq;
    svf->ic2eq = (2.0f * v2) - svf->ic2eq;
    
    if (svf->type == SVFTypeLowpass)        return v2;
    else if (svf->type == SVFTypeBandpass)  return v1;
    else if (svf->type == SVFTypeHighpass)  return v0 - (svf->k * v1) - v2;
    else if (svf->type == SVFTypeNotch)     return v0 - (svf->k * v1);
    else if (svf->type == SVFTypePeak)      return v0 - (svf->k * v1) - (2.0f * v2);
    else                                    return 0.0f;
    
}

int     tSVFSetFreq(tSVF *svf, uint16_t cutoffKnob)
{
    svf->g = filtertan[cutoffKnob];
    svf->a1 = 1.0f/(1.0f + svf->g * (svf->g + svf->k));
    svf->a2 = svf->g * svf->a1;
    svf->a3 = svf->g * svf->a2;
    
    return 0;
}

int     tSVFSetQ(tSVF *svf, float Q)
{
    svf->k = 1.0f/clipAU(0.01f,Q,10.0f);
    svf->a1 = 1.0f/(1.0f + svf->g * (svf->g + svf->k));
    svf->a2 = svf->g * svf->a1;
    svf->a3 = svf->g * svf->a2;
    
    return 0;
}

int     tSVFInit(tSVF *svf, float sr, SVFType type, uint16_t cutoffKnob, float Q)
{
    svf->inv_sr = 1.0f/sr;
    svf->type = type;
    
    svf->ic1eq = 0;
    svf->ic2eq = 0;
    
    float a1,a2,a3,g,k;
    g = filtertan[cutoffKnob];
    k = 1.0f/clipAU(0.01f,Q,10.0f);
    a1 = 1.0f/(1.0f+g*(g+k));
    a2 = g*a1;
    a3 = g*a2;
    
    svf->g = g;
    svf->k = k;
    svf->a1 = a1;
    svf->a2 = a2;
    svf->a3 = a3;
    
    return 0;
}



float   tSVFEfficientTick(tSVF *svf, float v0)
{
    float v1,v2,v3;
    float low,high;
    v3 = v0 - svf->ic2eq;
    v1 = (svf->a1 * svf->ic1eq) + (svf->a2 * v3);
    v2 = svf->ic2eq + (svf->a2 * svf->ic1eq) + (svf->a3 * v3);
    svf->ic1eq = (2.0f * v1) - svf->ic1eq;
    svf->ic2eq = (2.0f * v2) - svf->ic2eq;
    
    if (svf->type == SVFTypeLowpass)        return v2;
    else if (svf->type == SVFTypeBandpass)  return v1;
    else if (svf->type == SVFTypeHighpass)  return v0 - (svf->k * v1) - v2;
    else if (svf->type == SVFTypeNotch)     return v0 - (svf->k * v1);
    else if (svf->type == SVFTypePeak)      return v0 - (svf->k * v1) - (2.0f * v2);
    else                                    return 0.0f;
    
}

int     tSVFEfficientSetFreq(tSVF *svf, uint16_t cutoffKnob)
{
    svf->g = filtertan[cutoffKnob];
    svf->a1 = 1.0f/(1.0f + svf->g * (svf->g + svf->k));
    svf->a2 = svf->g * svf->a1;
    svf->a3 = svf->g * svf->a2;
    
    return 0;
}

int     tSVFEfficientSetQ(tSVF *svf, float Q)
{
    svf->k = 1.0f/Q;
    svf->a1 = 1.0f/(1.0f + svf->g * (svf->g + svf->k));
    svf->a2 = svf->g * svf->a1;
    svf->a3 = svf->g * svf->a2;
    
    return 0;
}

int     tSVFEfficientInit(tSVFEfficient *svf, float sr, SVFType type, uint16_t cutoffKnob, float Q)
{
    svf->inv_sr = 1.0f/sr;
    svf->type = type;
    
    svf->ic1eq = 0;
    svf->ic2eq = 0;
    
    float a1,a2,a3,g,k;
    g = filtertan[cutoffKnob];
    k = 1.0f/Q;
    a1 = 1.0f/(1.0f+g*(g+k));
    a2 = g*a1;
    a3 = g*a2;
    
    svf->g = g;
    svf->k = k;
    svf->a1 = a1;
    svf->a2 = a2;
    svf->a3 = a3;
    
    return 0;
}

float   tEnvelopeFollowerTick(tEnvelopeFollower *ef, float x)
{
    if (x < 0.0f ) x = -x;  /* Absolute value. */
    
    if ((x >= ef->y) && (x > ef->a_thresh)) ef->y = x;                      /* If we hit a peak, ride the peak to the top. */
    else                                    ef->y = ef->y * ef->d_coeff;    /* Else, exponential decay of output. */
    
    //ef->y = envelope_pow[(uint16_t)(ef->y * (float)UINT16_MAX)] * ef->d_coeff; //not quite the right behavior - too much loss of precision?
    //ef->y = powf(ef->y, 1.000009f) * ef->d_coeff;  // too expensive
    
    if( ef->y < VERY_SMALL_FLOAT)   ef->y = 0.0f;

    return ef->y;
}

int     tEnvelopeFollowerDecayCoeff(tEnvelopeFollower *ef, float decayCoeff)
{
    return ef->d_coeff = decayCoeff;
}

int     tEnvelopeFollowerAttackThresh(tEnvelopeFollower *ef, float attackThresh)
{
    return ef->a_thresh = attackThresh;
}

int     tEnvelopeFollowerInit(tEnvelopeFollower *ef, float attackThreshold, float decayCoeff)
{
    ef->y = 0.0f;
    ef->a_thresh = attackThreshold;
    ef->d_coeff = decayCoeff;
    
    return 0;
}

/* Highpass */
int     tHighpassFreq(tHighpass *hp, float freq)
{
    
    hp->R = (1.0f-((freq * 2.0f * 3.14f)*hp->inv_sr));
    
    return 0;
}

// From JOS DC Blocker
float   tHighpassTick(tHighpass *hp, float x)
{
    hp->ys = x - hp->xs + hp->R * hp->ys;
    hp->xs = x;
    return hp->ys;
}

int     tHighpassInit(tHighpass *hp, float sr, float freq)
{
    hp->inv_sr = 1.0f/sr;
    hp->R = (1.0f-((freq * 2.0f * 3.14f)*hp->inv_sr));
    hp->ys = 0.0f;
    hp->xs = 0.0f;
    
    return 0;
}

/* Cycle */
int     tCycleSetFreq(tCycle *c, float freq)
{
    c->inc = freq * c->inv_sr;
    
    return 0;
}

float   tCycleTick(tCycle *c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    // Wavetable synthesis
    float temp = SINE_TABLE_SIZE * c->phase;
    int intPart = (int)temp;
    float fracPart = temp - (float)intPart;
    float samp0 = sinewave[intPart];
    if (++intPart >= SINE_TABLE_SIZE) intPart = 0;
    float samp1 = sinewave[intPart];
    return (samp0 + (samp1 - samp0) * fracPart);
}

int     tCycleInit(tCycle *c, float sr)
{
    // Underlying phasor
    c->inc = 0.0f;
    c->phase = 0.0f;
    c->inv_sr = 1.0f/sr;
    
    return 0;
}

/* Sawtooth */
int     tSawtoothSetFreq(tSawtooth *c, float freq)
{
    c->freq = freq;
    c->inc = freq * c->inv_sr;
    
    return 0;
}

float   tSawtoothTick(tSawtooth *c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    float out = 0.0f;
    float w1, w2;
    
    int idx = (int)(c->phase * TRI_TABLE_SIZE);
    
    // Wavetable synthesis
    
    if (c->freq <= 20.0f)
    {
        out = sawtooth[T20][idx];
    }
    else if (c->freq <= 40.0f)
    {
        w1 = (40.0f - c->freq) / 20.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T20][idx] * w1) + (sawtooth[T40][idx] * w2);
    }
    else if (c->freq <= 80.0f)
    {
        w1 = (80.0f - c->freq) / 40.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T40][idx] * w1) + (sawtooth[T80][idx] * w2);
    }
    else if (c->freq <= 160.0f)
    {
        w1 = (160.0f - c->freq) / 80.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T80][idx] * w1) + (sawtooth[T160][idx] * w2);
    }
    else if (c->freq <= 320.0f)
    {
        w1 = (320.0f - c->freq) / 160.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T160][idx] * w1) + (sawtooth[T320][idx] * w2);
    }
    else if (c->freq <= 640.0f)
    {
        w1 = (640.0f - c->freq) / 320.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T320][idx] * w1) + (sawtooth[T640][idx] * w2);
    }
    else if (c->freq <= 1280.0f)
    {
        w1 = (1280.0f - c->freq) / 640.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T640][idx] * w1) + (sawtooth[T1280][idx] * w2);
    }
    else if (c->freq <= 2560.0f)
    {
        w1 = (2560.0f - c->freq) / 1280.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T1280][idx] * w1) + (sawtooth[T2560][idx] * w2);
    }
    else if (c->freq <= 5120.0f)
    {
        w1 = (5120.0f - c->freq) / 2560.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T2560][idx] * w1) + (sawtooth[T5120][idx] * w2);
    }
    else if (c->freq <= 10240.0f)
    {
        w1 = (10240.0 - c->freq) / 5120.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T5120][idx] * w1) + (sawtooth[T10240][idx] * w2);
    }
    else if (c->freq <= 20480.0f)
    {
        w1 = (20480.0f - c->freq) / 10240.0f;
        w2 = 1.0f - w1;
        
        out = (sawtooth[T10240][idx] * w1) + (sawtooth[T20480][idx] * w2);
    }
    else
    {
        out = sawtooth[T20480][idx];
    }

    return out;
}

int     tSawtoothInit(tSawtooth *c, float sr)
{
    // Underlying phasor
    c->inc = 0.0f;
    c->phase = 0.0f;
    c->inv_sr = 1.0f/sr;
    
    return 0;
}

/* Triangle */
int     tTriangleSetFreq(tTriangle *c, float freq)
{
    c->freq = freq;
    c->inc = freq * c->inv_sr;
    
    return 0;
}



float   tTriangleTick(tTriangle *c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    float out = 0.0f;
    float w1, w2;
    
    int idx = (int)(c->phase * TRI_TABLE_SIZE);
    
    // Wavetable synthesis
    
    if (c->freq <= 20.0f)
    {
        out = triangle[T20][idx];
    }
    else if (c->freq <= 40.0f)
    {
        w1 = (40.0f - c->freq) / 20.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T20][idx] * w1) + (triangle[T40][idx] * w2);
    }
    else if (c->freq <= 80.0f)
    {
        w1 = (80.0f - c->freq) / 40.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T40][idx] * w1) + (triangle[T80][idx] * w2);
    }
    else if (c->freq <= 160.0f)
    {
        w1 = (160.0f - c->freq) / 80.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T80][idx] * w1) + (triangle[T160][idx] * w2);
    }
    else if (c->freq <= 320.0f)
    {
        w1 = (320.0f - c->freq) / 160.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T160][idx] * w1) + (triangle[T320][idx] * w2);
    }
    else if (c->freq <= 640.0f)
    {
        w1 = (640.0f - c->freq) / 320.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T320][idx] * w1) + (triangle[T640][idx] * w2);
    }
    else if (c->freq <= 1280.0f)
    {
        w1 = (1280.0f - c->freq) / 640.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T640][idx] * w1) + (triangle[T1280][idx] * w2);
    }
    else if (c->freq <= 2560.0f)
    {
        w1 = (2560.0f - c->freq) / 1280.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T1280][idx] * w1) + (triangle[T2560][idx] * w2);
    }
    else if (c->freq <= 5120.0f)
    {
        w1 = (5120.0f - c->freq) / 2560.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T2560][idx] * w1) + (triangle[T5120][idx] * w2);
    }
    else if (c->freq <= 10240.0f)
    {
        w1 = (10240.0 - c->freq) / 5120.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T5120][idx] * w1) + (triangle[T10240][idx] * w2);
    }
    else if (c->freq <= 20480.0f)
    {
        w1 = (20480.0f - c->freq) / 10240.0f;
        w2 = 1.0f - w1;
        
        out = (triangle[T10240][idx] * w1) + (triangle[T20480][idx] * w2);
    }
    else
    {
        out = triangle[T20480][idx];
    }
    
    return out;
}

int     tTriangleInit(tTriangle *c, float sr)
{
    // Underlying phasor
    c->inc = 0.0f;
    c->phase = 0.0f;
    c->inv_sr = 1.0f/sr;
    
    return 0;
}

/* Square */
int     tSquareSetFreq(tSquare *c, float freq)
{
    c->freq = freq;
    c->inc = freq * c->inv_sr;
    
    return 0;
}



float   tSquareTick(tSquare *c)
{
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    
    float out = 0.0f;
    float w1, w2;
    
    int idx = (int)(c->phase * TRI_TABLE_SIZE);
    
    // Wavetable synthesis
    
    if (c->freq <= 20.0f)
    {
        out = squarewave[T20][idx];
    }
    else if (c->freq <= 40.0f)
    {
        w1 = (40.0f - c->freq) / 20.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T20][idx] * w1) + (squarewave[T40][idx] * w2);
    }
    else if (c->freq <= 80.0f)
    {
        w1 = (80.0f - c->freq) / 40.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T40][idx] * w1) + (squarewave[T80][idx] * w2);
    }
    else if (c->freq <= 160.0f)
    {
        w1 = (160.0f - c->freq) / 80.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T80][idx] * w1) + (squarewave[T160][idx] * w2);
    }
    else if (c->freq <= 320.0f)
    {
        w1 = (320.0f - c->freq) / 160.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T160][idx] * w1) + (squarewave[T320][idx] * w2);
    }
    else if (c->freq <= 640.0f)
    {
        w1 = (640.0f - c->freq) / 320.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T320][idx] * w1) + (squarewave[T640][idx] * w2);
    }
    else if (c->freq <= 1280.0f)
    {
        w1 = (1280.0f - c->freq) / 640.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T640][idx] * w1) + (squarewave[T1280][idx] * w2);
    }
    else if (c->freq <= 2560.0f)
    {
        w1 = (2560.0f - c->freq) / 1280.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T1280][idx] * w1) + (squarewave[T2560][idx] * w2);
    }
    else if (c->freq <= 5120.0f)
    {
        w1 = (5120.0f - c->freq) / 2560.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T2560][idx] * w1) + (squarewave[T5120][idx] * w2);
    }
    else if (c->freq <= 10240.0f)
    {
        w1 = (10240.0 - c->freq) / 5120.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T5120][idx] * w1) + (squarewave[T10240][idx] * w2);
    }
    else if (c->freq <= 20480.0f)
    {
        w1 = (20480.0f - c->freq) / 10240.0f;
        w2 = 1.0f - w1;
        
        out = (squarewave[T10240][idx] * w1) + (squarewave[T20480][idx] * w2);
    }
    else
    {
        out = squarewave[T20480][idx];
    }
    
    return out;
}

int     tSquareInit(tSquare *c, float sr)
{
    // Underlying phasor
    c->inc = 0.0f;
    c->phase = 0.0f;
    c->inv_sr = 1.0f/sr;
    
    return 0;
}


float   tNoiseTick(tNoise *n)
{
    float rand = n->rand();
    
    if (n->type == NoiseTypePink)
    {
        float tmp;
        n->pinkb0 = 0.99765f * n->pinkb0 + rand * 0.0990460f;
        n->pinkb1 = 0.96300f * n->pinkb1 + rand * 0.2965164f;
        n->pinkb2 = 0.57000f * n->pinkb2 + rand * 1.0526913f;
        tmp = n->pinkb0 + n->pinkb1 + n->pinkb2 + rand * 0.1848f;
        return (tmp * 0.05f);
    }
    else // NoiseTypeWhite
    {
        return rand;
    }
}

int     tNoiseInit(tNoise *n, float sr, float (*randomNumberGenerator)(), NoiseType type)
{
    n->type = type;
    n->rand = randomNumberGenerator;
    return 0;
}

int     tRampSetTime(tRamp *r, float time)
{
    r->time = time;
    COMPUTE_INC();
    return 0;
}

int     tRampSetDest(tRamp *r, float dest)
{
    r->dest = dest;
    COMPUTE_INC();
    return 0;
}

float   tRampTick(tRamp *r) {
    
    r->curr += r->inc;
    
    if (((r->curr >= r->dest) && (r->inc > 0.0f)) || ((r->curr <= r->dest) && (r->inc < 0.0f))) r->inc = 0.0f;
    
    return r->curr;
}

int     tRampInit(tRamp *r, float sr, float time, int samples_per_tick)
{
    r->inv_sr_ms = 1.0f/(sr*0.001f);
    r->curr = 0.0f;
    r->dest = 0.0f;
    r->time = time;
    r->samples_per_tick = samples_per_tick;
    COMPUTE_INC();
    return 0;
}
