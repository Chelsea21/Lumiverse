/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_F1E5D9BB3E654E28__
#define __JUCE_HEADER_F1E5D9BB3E654E28__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "../../../LumiverseCore/LumiverseCore.h"
//[/Headers]

using namespace Lumiverse;

//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class GuiComponent  : public Component,
                      public SliderListener,
                      public ButtonListener
{
public:
    //==============================================================================
    GuiComponent (float *buffer, Image::PixelFormat format,
                  int imageWidth, int imageHeight, Rig *rig);
    ~GuiComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void buttonClicked (Button* buttonThatWasClicked);
    
private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Label> m_intensity_label;
    ScopedPointer<Slider> m_intensity_slider;
    ScopedPointer<Button> m_abort_button;
    ScopedPointer<LookAndFeel> m_lookandfeel;
    
    Image m_panel;
    float *m_color_buffer;
    
    int m_width, m_height;
    
    Rig *m_rig;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GuiComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_F1E5D9BB3E654E28__
