#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
namespace juce { class AudioPluginInstance; }

class MainWindow : public juce::DocumentWindow {
public:
    MainWindow();
    ~MainWindow();
    void installEditor(juce::AudioPluginInstance *processor);
    void uninstallEditor();

    enum ShowEditorFlag {
        showPluginEditor = 1 << 0,
        showGenericEditor = 1 << 1,
    };
    void setShowEditorFlags(int flags);

private:
    void uninstallEditorEx(bool relayout);

protected:
    void closeButtonPressed() override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
