#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
class PluginSlot;

class Application : public juce::JUCEApplication {
public:
    Application();
    ~Application();
    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;

    PluginSlot &getMainPluginSlot();

protected:
    void initialise(const juce::String &commandLineParameters) override;
    void shutdown() override;
    void systemRequestedQuit() override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
