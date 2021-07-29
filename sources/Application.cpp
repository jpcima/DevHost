#include "Application.h"
#include "LookAndFeel.h"
#include "MainWindow.h"
#include "PluginSlot.h"

struct Application::Impl {
    std::unique_ptr<PluginSlot> mainPluginSlot_;
    std::unique_ptr<MainWindow> mainWindow_;

    void onPluginCreated(PluginSlot &slot);
    void onPluginRemoved(PluginSlot &slot);

    struct LookAndFeelInitializer {
        LookAndFeelInitializer();
    };
};

Application::Application()
    : impl_(new Impl)
{
    static Impl::LookAndFeelInitializer lookAndFeelInitializer;
}

Application::~Application()
{
}

const juce::String Application::getApplicationName()
{
    return PROJECT_NAME;
}

const juce::String Application::getApplicationVersion()
{
    return PROJECT_VERSION;
}

PluginSlot &Application::getMainPluginSlot()
{
    Impl &impl = *impl_;
    return *impl.mainPluginSlot_;
}

void Application::initialise(const juce::String &commandLineParameters)
{
    Impl &impl = *impl_;

    PluginSlot *mainPluginSlot = new PluginSlot;
    impl.mainPluginSlot_.reset(mainPluginSlot);

    mainPluginSlot->OnPluginCreated = [&impl, mainPluginSlot]() { impl.onPluginCreated(*mainPluginSlot); };
    mainPluginSlot->OnPluginRemoved = [&impl, mainPluginSlot]() { impl.onPluginRemoved(*mainPluginSlot); };

    MainWindow *mainWindow = new MainWindow;
    impl.mainWindow_.reset(new MainWindow);
    mainWindow->setVisible(true);

    ///
    (void)commandLineParameters;
    const juce::StringArray &args = getCommandLineParameterArray();

    if (!args.isEmpty()) {
        #pragma message("TODO")
        if (mainPluginSlot->createPluginFromFile(args[0], 44100, 1024)) {
            mainWindow->installEditor(mainPluginSlot->getProcessor());
        }
    }
}

void Application::shutdown()
{
    Impl &impl = *impl_;
    impl.mainWindow_.reset();
    impl.mainPluginSlot_.reset();
}

void Application::systemRequestedQuit()
{
    quit();
}

void Application::Impl::onPluginCreated(PluginSlot &slot)
{
    if (&slot != mainPluginSlot_.get())
        return;

    mainWindow_->installEditor(slot.getProcessor());
}

void Application::Impl::onPluginRemoved(PluginSlot &slot)
{
    if (&slot != mainPluginSlot_.get())
        return;

    mainWindow_->uninstallEditor();
}

///
Application::Impl::LookAndFeelInitializer::LookAndFeelInitializer()
{
    static MyLookAndFeel lookAndFeel;
    juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel);
}

///
JUCE_CREATE_APPLICATION_DEFINE(Application)
JUCE_MAIN_FUNCTION_DEFINITION
