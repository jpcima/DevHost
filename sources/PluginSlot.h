#pragma once
#include <functional>
#include <memory>
namespace juce { class String; }
namespace juce { class AudioPluginInstance; }
namespace juce { class AudioProcessorEditor; }

class PluginSlot {
public:
    PluginSlot();
    ~PluginSlot();
    void removePlugin();
    bool createPluginFromFile(const juce::String &filePath, float sampleRate, int bufferSize);
    juce::AudioPluginInstance *getProcessor();

    std::function<void()> OnPluginRemoved;
    std::function<void()> OnPluginCreated;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
