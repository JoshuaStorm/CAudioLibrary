/*
  ==============================================================================

    Globals.h
    Created: 17 Jan 2017 12:16:12pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#define TEST 1


String floatArrayToString(Array<float> arr)
{
    String s = "";
    for (auto key : arr)
    {
        s.append(String(key), 6);
        s.append(" ", 1);
    }
    return s;
}

String intArrayToString(Array<int> arr)
{
    String s = "";
    for (auto key : arr)
    {
        s.append(String(key), 6);
        s.append(" ", 1);
    }
    return s;
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

static const int cBoxHeight = 20;
static const int cBoxWidth  =  100;

#endif  // GLOBALS_H_INCLUDED
