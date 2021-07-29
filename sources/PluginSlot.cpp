#include "PluginSlot.h"
#include <juce_audio_processors/juce_audio_processors.h>

struct PluginSlot::Impl {
    std::unique_ptr<juce::AudioPluginFormatManager> formatManager_;
    std::unique_ptr<juce::AudioPluginInstance> currentInstance_;
    std::unique_ptr<juce::AudioProcessorEditor> currentEditor_;
};

PluginSlot::PluginSlot()
    : impl_(new Impl)
{
    Impl &impl = *impl_;
    juce::AudioPluginFormatManager *formatManager = new juce::AudioPluginFormatManager;
    impl.formatManager_.reset(formatManager);
    formatManager->addDefaultFormats();
}

PluginSlot::~PluginSlot()
{
}

void PluginSlot::removePlugin()
{
    Impl &impl = *impl_;

    if (!impl.currentInstance_)
        return;

    if (OnPluginRemoved)
        OnPluginRemoved();

    impl.currentEditor_.reset();
    impl.currentInstance_.reset();
}

bool PluginSlot::createPluginFromFile(const juce::String &filePath, float sampleRate, int bufferSize)
{
    Impl &impl = *impl_;
    juce::AudioPluginFormatManager &formatManager = *impl.formatManager_;

    std::unique_ptr<juce::PluginDescription> pluginDesc;
    juce::AudioPluginFormat *pluginFormat = nullptr;

    for (juce::AudioPluginFormat *format : formatManager.getFormats()) {
        juce::OwnedArray<juce::PluginDescription> pluginDescList;
        format->findAllTypesForFile(pluginDescList, filePath);

        if (pluginDescList.size() > 0) {
            pluginDesc.reset(pluginDescList.removeAndReturn(0));
            pluginFormat = format;
            break;
        }
    }

    if (!pluginDesc)
        return false;

    std::unique_ptr<juce::AudioPluginInstance> instance = pluginFormat->createInstanceFromDescription(*pluginDesc, sampleRate, bufferSize);

    if (!instance)
        return false;

    removePlugin();

    impl.currentInstance_ = std::move(instance);

    if (OnPluginCreated)
        OnPluginCreated();

    return true;
}

juce::AudioPluginInstance *PluginSlot::getProcessor()
{
    Impl &impl = *impl_;
    return impl.currentInstance_.get();
}
