#pragma once
#include <fontaudio/fontaudio.h>
#include <juce_gui_basics/juce_gui_basics.h>

class MyLookAndFeel : public juce::LookAndFeel_V4 {
public:
    MyLookAndFeel();
    ~MyLookAndFeel();
    fontaudio::IconHelper &getFontAudio() { return *sharedFontAudio_; }

private:
    juce::SharedResourcePointer<fontaudio::IconHelper> sharedFontAudio_;
};
