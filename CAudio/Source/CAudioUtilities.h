/*
  ==============================================================================

    Utilities.h
    Created: 5 Dec 2016 2:37:03pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef UTILITIES_H_INCLUDED
#define UTILITIES_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "stdlib.h"

static const std::vector<std::string> cSliderNames =
{
    "OscPitch",
    "Noise",
    "Attack",
    "Decay",
    "TZFrequency",
    "TZRadius",
    "Gain"
};



static const std::vector<std::string> cButtonNames =
{
    "Tap"
};

std::vector<float> cSliderValues(cSliderNames.size());

std::vector<bool> cButtonStates(cButtonNames.size());

void printSliderValues(void)
{
    for (int i = 0; i < cSliderNames.size(); i++)
    {
        DBG(cSliderNames[i] + ": " + String(cSliderValues[i]));
    }
}

bool getButtonState(String name)
{
    for (int i = 0; i < cButtonNames.size(); i++)
    {
        if (name == cButtonNames[i])
        {
            return cButtonStates[i];
        }
    }
}

void setButtonState(String name, bool on)
{
    for (int i = 0; i < cButtonNames.size(); i++)
    {
        if (name == cButtonNames[i])
        {
            cButtonStates[i] = on;
        }
    }
}

void setSliderValue(String name, float val)
{
    for (int i = 0; i < cSliderNames.size(); i++)
    {
        if (name == cSliderNames[i])
        {
            cSliderValues[i] = val;
        }
    }
    printSliderValues();
}

float getSliderValue(String name)
{
    float value = 0.0f;
    
    for (int i = 0; i < cSliderNames.size(); i++)
    {
        if (name == cSliderNames[i])
        {
            value = cSliderValues[i];
        }
    }
    
    return value;
}

float randomNumberGenerator(void)
{
    return ((float)rand()/RAND_MAX);
}


static const int cLeftOffset = 30;
static const int cTopOffset = 30;

static const int cXSpacing = 60;
static const int cYSpacing = 5;

static const int cSliderHeight = 200;
static const int cSliderWidth = 20;

static const int cLabelHeight = 20;
static const int cLabelWidth  = cSliderWidth + cXSpacing;

static const int cButtonHeight = 30;
static const int cButtonWidth  = 60;



#endif  // UTILITIES_H_INCLUDED
