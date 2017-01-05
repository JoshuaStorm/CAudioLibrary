/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/
#include "MainComponent.h"


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

    //==============================================================================
MainContentComponent::MainContentComponent():
settings(*this,
          0,
          2,
          0,
          2,
          false,
          false,
          true,
          false)
{
    setSize (1100, 800);
    
    addAndMakeVisible(ui);
    addAndMakeVisible(settings);
    
    setAudioChannels (2, 2);
}

MainContentComponent::~MainContentComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainContentComponent::prepareToPlay (int samplesPerBlockExpected, double sr)
{
    gain = 0.0f;
    timer = 0;
    start = false;
    ramp = false;
    
    sampleRate = sr;
    
    init(sampleRate);
}

void MainContentComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) 
{
    AudioIODevice* device = deviceManager.getCurrentAudioDevice();
    
    const BigInteger activeInputChannels = device->getActiveInputChannels();
    const BigInteger activeOutputChannels = device->getActiveOutputChannels();

    // Delay audio by 1 second on startup, then ramp on.
    if (!ramp && (timer < sampleRate * 1.0f))
    {
        timer += bufferToFill.numSamples;
    }
    else
    {
        ramp = true;
    }

    if ((! activeOutputChannels[0]) || ! activeOutputChannels[1])
    {
        bufferToFill.buffer->clear (0, bufferToFill.startSample, bufferToFill.numSamples);
        bufferToFill.buffer->clear (1, bufferToFill.startSample, bufferToFill.numSamples);
    }
    else
    {
        if (! activeInputChannels[0] || ! activeInputChannels[1])
        {
            bufferToFill.buffer->clear (0, bufferToFill.startSample, bufferToFill.numSamples);
            bufferToFill.buffer->clear (1, bufferToFill.startSample, bufferToFill.numSamples);
        }
        else
        {
            const float* inBufferL = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);
            const float* inBufferR = bufferToFill.buffer->getReadPointer (1, bufferToFill.startSample);
            
            float* outBufferL = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
            float* outBufferR = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);
            
            block();
            for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                // Ramp on startup.
                if (!start && ramp)
                {
                    if (gain < 1.0f)
                    {
                        gain += 0.0025f;
                    }
                    else
                    {
                        start = true;
                        gain = 1.0f;
                    }
                }
                
                outBufferL[sample] = tick((inBufferL[sample] + inBufferR[sample]) * 0.5f)   * gain;
                outBufferR[sample] = outBufferL[sample]                                     * gain;
            }
                
        }
    }
}

void MainContentComponent::releaseResources() 
{

}

//==============================================================================
void MainContentComponent::paint (Graphics& g) 
{

}

void MainContentComponent::resized() 
{
    ui.setBounds(getLocalBounds());
    
    settings.setTopLeftPosition(0, 300);
    settings.setSize(500,800);
}

void MainContentComponent::sliderValueChanged(Slider* slider)
{
    
}

// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()
{
    return new MainContentComponent();
}


