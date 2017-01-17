/*
  ==============================================================================

    MainComponent.h
    Created: 5 Dec 2016 3:12:27pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "UIComponent.h"

#include "CAudioTest1.h"


class MainContentComponent   :  public AudioAppComponent, public Slider::Listener, public AudioDeviceManager
{
public:
    //==============================================================================
    MainContentComponent();
    
    ~MainContentComponent();
    
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sr) override;
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    
    void releaseResources() override;
    
    //==============================================================================
    void paint (Graphics& g) override;
    
    void resized() override;
    
    void sliderValueChanged(Slider*) override;
    
private:
    //==============================================================================
    
    // Your private member variables go here...
    uint64 timer;
    float gain;
    bool start,ramp;
    double sampleRate;
    
    UIComponent ui;
    AudioDeviceSelectorComponent settings;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent();



#endif  // MAINCOMPONENT_H_INCLUDED
