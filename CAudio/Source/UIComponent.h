/*
  ==============================================================================

    UIComponent.h
    Created: 5 Dec 2016 2:23:38pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef UICOMPONENT_H_INCLUDED
#define UICOMPONENT_H_INCLUDED

#include "CAudioUtilities.h"

#include "CAudioLibraryTest.h"


//==============================================================================
/*
*/
class UIComponent    : public Component, public Slider::Listener, public TextButton::Listener, public ComboBox::Listener
{
public:
    UIComponent()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        
        for (int i = 0; i < cSliderNames.size(); i++)
        {
            
            sliders.set(i,  new Slider());
            sliders[i]->setSliderStyle(Slider::SliderStyle::LinearVertical);
            sliders[i]->setRange(0.0, 1.0, 1.0/50000.0);
            sliders[i]->setName(cSliderNames[i]);
            sliders[i]->addListener(this);
            addAndMakeVisible(sliders[i]);
            
            sliderLabels.set(i,   new Label());
            sliderLabels[i]->setName(cSliderNames[i]);
            sliderLabels[i]->setColour(Label::textColourId, Colours::aliceblue);
            sliderLabels[i]->setText(cSliderNames[i], NotificationType::dontSendNotification);
            addAndMakeVisible(sliderLabels[i]);
        }
        
        for (int i = 0; i < cButtonNames.size(); i++)
        {
            buttons.set(i, new TextButton(cButtonNames[i]));
            buttons[i]->changeWidthToFitText();
            buttons[i]->setButtonText(cButtonNames[i]);
            buttons[i]->addListener(this);
            addAndMakeVisible(buttons[i]);
        }
        
        for (int i = 0; i < cComboBoxNames.size(); i++)
        {
            comboBoxes.set(i, new ComboBox());
            comboBoxes[i]->setName(cComboBoxNames[i]);
            comboBoxes[i]->addSeparator();
            comboBoxes[i]->addListener(this);
            addAndMakeVisible(comboBoxes[i]);
            
        }
        
        for (int i = 0; i < cWaveformType.size(); i++)
        {
            comboBoxes[0]->addItem(cWaveformType[i], i+1);
        }
        
        comboBoxes[0]->setSelectedItemIndex(0);
        
    }

    ~UIComponent()
    {
    }

    void paint (Graphics& g) override
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */

        g.fillAll (Colours::slategrey);   // clear the background

        g.setColour (Colours::aliceblue);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
    }

    void resized() override
    {
        for (int i = 0; i < cSliderNames.size(); i++)
        {
            sliders[i]      ->setBounds(
                                        cLeftOffset + ((cSliderWidth + cXSpacing) * i),
                                        cTopOffset,
                                        cSliderWidth,
                                        cSliderHeight);
            
            sliderLabels[i] ->setBounds(
                                        cLeftOffset + ((cSliderWidth + cXSpacing) * i) - (cSliderNames[i].length() * 2.0f),
                                        cTopOffset + cSliderHeight + cYSpacing,
                                        cLabelWidth,
                                        cLabelHeight);
        }
        
        for (int i = 0; i < cButtonNames.size(); i++)
        {
            buttons[i]      ->setBounds(
                                        cLeftOffset + ((cButtonWidth + cXSpacing) * i),
                                        cTopOffset + cSliderHeight + cLabelHeight + 2 * cYSpacing,
                                        cButtonWidth,
                                        cButtonHeight);
        }
        
        for (int i = 0; i < cComboBoxNames.size(); i++)
        {
            comboBoxes[i]   ->setBounds(
                                        cLeftOffset + ((cButtonWidth + cXSpacing) * i),
                                        cTopOffset + cSliderHeight + cLabelHeight + cButtonHeight + 3 * cYSpacing,
                                        cBoxWidth,
                                        cBoxHeight    );
        }
    }
    
    void sliderValueChanged(Slider* s) override
    {
        setSliderValue(s->getName(), s->getValue());
    }
    
    void buttonClicked (Button* b) override
    {
        setButtonState(b->getName(), true);
    }
    
    void comboBoxChanged (ComboBox* cb) override
    {
        setComboBoxState(cb->getName(), cb->getSelectedId() - 1);
    }
    

private:
    OwnedArray<Slider> sliders;
    OwnedArray<Label>  sliderLabels;
    
    OwnedArray<TextButton> buttons;
    
    OwnedArray<ComboBox> comboBoxes;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIComponent)
};


#endif  // UICOMPONENT_H_INCLUDED
