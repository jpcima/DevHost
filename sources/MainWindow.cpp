#include "MainWindow.h"
#include "LookAndFeel.h"
#include <juce_audio_processors/juce_audio_processors.h>

struct MainWindow::Impl : public juce::Button::Listener,
                          public juce::ComponentListener {
    MainWindow *self_ = nullptr;
    std::unique_ptr<juce::Toolbar> toolbar_;

    int showEditorFlags_ = showPluginEditor;
    juce::AudioProcessorEditor *pluginEditor_ = nullptr;
    std::unique_ptr<juce::GenericAudioProcessorEditor> genericEditor_;

    std::unique_ptr<juce::TooltipWindow> tooltipWindow_;

    enum ToolbarIds {
        tb_none,
        tb_open,
        tb_show_plugin_ui,
        tb_show_generic_ui,
    };

    void relayoutContents();

    void buttonClicked(Button *button) override;
    void buttonStateChanged(Button *button) override;

    void componentMovedOrResized(Component &component, bool wasMoved, bool wasResized) override;

    class MainToolbarFactory : public juce::ToolbarItemFactory {
    public:
        explicit MainToolbarFactory(Impl &impl) : impl_(&impl) {}
        void getAllToolbarItemIds(juce::Array<int> &ids) override;
        void getDefaultItemSet(juce::Array<int> &ids) override;
        juce::ToolbarItemComponent *createItem(int itemId) override;

    private:
        juce::ToolbarButton *createFontaudioButton(int itemId, fontaudio::IconName iconName, float iconSize, const juce::String &text);

    private:
        Impl *impl_ = nullptr;
    };
};

MainWindow::MainWindow()
    : juce::DocumentWindow(
        PROJECT_NAME,
        juce::LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
        juce::DocumentWindow::minimiseButton|juce::DocumentWindow::closeButton),
      impl_(new Impl)
{
    Impl &impl = *impl_;
    impl.self_ = this;

    juce::Component *content = new juce::Component;
    content->setSize(800, 600);
    setContentOwned(content, true);

    juce::Toolbar *toolbar = new juce::Toolbar;
    impl.toolbar_.reset(toolbar);
    toolbar->setSize(0, 60);
    toolbar->setStyle(juce::Toolbar::iconsWithText);
    content->addChildComponent(toolbar);

    Impl::MainToolbarFactory toolbarFactory(impl);
    toolbar->addDefaultItems(toolbarFactory);

    impl.tooltipWindow_.reset(new juce::TooltipWindow(this));

    impl.relayoutContents();
}

MainWindow::~MainWindow()
{
}

void MainWindow::installEditor(juce::AudioPluginInstance *processor)
{
    Impl &impl = *impl_;
    juce::Component *content = getContentComponent();

    uninstallEditorEx(false);

    juce::AudioProcessorEditor *newEditor = processor->createEditorIfNeeded();
    if (newEditor) {
        newEditor->addComponentListener(&impl);
        content->addChildComponent(newEditor);
    }
    impl.pluginEditor_ = newEditor;

    juce::GenericAudioProcessorEditor *genericEditor = new juce::GenericAudioProcessorEditor(*processor);
    impl.genericEditor_.reset(genericEditor);
    content->addChildComponent(genericEditor);

    impl.relayoutContents();
}

void MainWindow::uninstallEditor()
{
    uninstallEditorEx(true);
}

void MainWindow::uninstallEditorEx(bool relayout)
{
    Impl &impl = *impl_;
    juce::Component *content = getContentComponent();

    if (juce::AudioProcessorEditor *editor = impl.pluginEditor_) {
        content->removeChildComponent(editor);
        editor->removeComponentListener(&impl);
    }
    impl.pluginEditor_ = nullptr;

    if (juce::GenericAudioProcessorEditor *genericEditor = impl.genericEditor_.get()) {
        content->removeChildComponent(genericEditor);
        impl.genericEditor_.reset();
    }

    if (relayout)
        impl.relayoutContents();
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplicationBase::quit();
}

void MainWindow::setShowEditorFlags(int flags)
{
    Impl &impl = *impl_;

    if (impl.showEditorFlags_ == flags)
        return;

    impl.showEditorFlags_ = flags;
    impl.relayoutContents();
}

void MainWindow::Impl::relayoutContents()
{
    juce::Component *content = self_->getContentComponent();
    juce::Toolbar *toolbar = toolbar_.get();

    const int minw = 640;
    const int minh = 480;

    int flags = showEditorFlags_;
    juce::AudioProcessorEditor *pluginEditor = pluginEditor_;
    juce::GenericAudioProcessorEditor *genericEditor = genericEditor_.get();

    if (!pluginEditor)
        flags &= ~showPluginEditor;
    if (!genericEditor)
        flags &= ~showGenericEditor;

    ///
    juce::Rectangle<int> toolbarPanel;
    toolbarPanel.setHeight(toolbar->getHeight());

    ///
    juce::Rectangle<int> pluginPanel;
    juce::Rectangle<int> genericPanel;
    if (flags & showPluginEditor) {
        int ew = pluginEditor->getWidth();
        int eh = pluginEditor->getHeight();
        pluginPanel.setSize(std::max(ew, minw), std::max(eh, minh));
        pluginPanel.setY(toolbarPanel.getHeight());
        toolbarPanel.setWidth(pluginPanel.getWidth());
    }
    if (flags & showGenericEditor) {
        int gw = genericEditor->getWidth();
        int gh = genericEditor->getHeight();
        if (flags & showPluginEditor)
            genericPanel.setSize(gw, gh);
        else
            genericPanel.setSize(std::max(gw, minw), std::max(gh, minh));
        genericPanel.setX(genericPanel.getX() + pluginPanel.getWidth());
        genericPanel.setY(toolbarPanel.getHeight());
        toolbarPanel.setWidth(genericPanel.getWidth());
    }
    if ((flags & showPluginEditor) && (flags & showGenericEditor)) {
        int h = std::max(pluginPanel.getHeight(), genericPanel.getHeight());
        pluginPanel.setHeight(h);
        genericPanel.setHeight(h);
        genericEditor->setSize(genericEditor->getWidth(), h);
        toolbarPanel.setWidth(pluginPanel.getWidth() + genericPanel.getWidth());
    }

    ///
    toolbar->setVisible(true);

    if (pluginEditor)
        pluginEditor->setVisible((flags & showPluginEditor));
    if (genericEditor)
        genericEditor->setVisible((flags & showGenericEditor));

    toolbar->setBounds(toolbarPanel);

    if (flags & showPluginEditor)
        pluginEditor->setCentrePosition(pluginPanel.getCentre());
    if (flags & showGenericEditor)
        genericEditor->setCentrePosition(genericPanel.getCentre());

    //fprintf(stderr, "plugin: %s\n", pluginPanel.toString().toRawUTF8());
    //fprintf(stderr, "generic: %s\n", genericPanel.toString().toRawUTF8());

    ///
    juce::Rectangle<int> *allPanels[8];
    int numPanels = 0;

    allPanels[numPanels++] = &toolbarPanel;
    if (flags & showPluginEditor)
        allPanels[numPanels++] = &pluginPanel;
    if (flags & showGenericEditor)
        allPanels[numPanels++] = &genericPanel;

    juce::Rectangle<int> totalBounds;
    if (numPanels > 0) {
        totalBounds = *allPanels[0];
        for (int i = 1; i < numPanels; ++i)
            totalBounds = totalBounds.getUnion(*allPanels[i]);
    }

    content->setSize(totalBounds.getWidth(), totalBounds.getHeight());
}

void MainWindow::Impl::buttonClicked(Button *button)
{
    MainWindow *self = self_;
    int itemId = (int)button->getProperties().getWithDefault("X-ItemID", -1);

    switch (itemId) {
    case tb_open:
        #pragma message("TODO")
        juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "Error", "Not implemented yet");
        
        break;
    case tb_show_plugin_ui:
        self->setShowEditorFlags(showEditorFlags_ ^ showPluginEditor);
        break;
    case tb_show_generic_ui:
        self->setShowEditorFlags(showEditorFlags_ ^ showGenericEditor);
        break;
    default:
        jassertfalse;
        break;
    }
}

void MainWindow::Impl::buttonStateChanged(Button *button)
{
    (void)button;
}

void MainWindow::Impl::componentMovedOrResized(Component &component, bool wasMoved, bool wasResized)
{
    (void)component;
    (void)wasMoved;
    if (wasResized && (showEditorFlags_ & showPluginEditor)) {
        relayoutContents();
    }
}

///
void MainWindow::Impl::MainToolbarFactory::getAllToolbarItemIds(juce::Array<int> &ids)
{
    getDefaultItemSet(ids);
}

void MainWindow::Impl::MainToolbarFactory::getDefaultItemSet(juce::Array<int> &ids)
{
    ids = {
        tb_open,
        tb_show_plugin_ui,
        tb_show_generic_ui,
    };
}

juce::ToolbarItemComponent *MainWindow::Impl::MainToolbarFactory::createItem(int itemId)
{
    Impl &impl = *impl_;
    juce::ToolbarItemComponent *item = nullptr;

    const float iconSize = 24.0f;

    switch (itemId) {
    case tb_open:
    {
        item = createFontaudioButton(itemId, fontaudio::Open, iconSize, "Open");
        break;
    }
    case tb_show_plugin_ui:
    {
        item = createFontaudioButton(itemId, fontaudio::Pen, iconSize, "Editor");
        item->setClickingTogglesState(true);
        item->setToggleState(impl.showEditorFlags_ & showPluginEditor, juce::dontSendNotification);
        break;
    }
    case tb_show_generic_ui:
    {
        item = createFontaudioButton(itemId, fontaudio::Sliderhandle1, iconSize, "Generic");
        item->setClickingTogglesState(true);
        item->setToggleState(impl.showEditorFlags_ & showGenericEditor, juce::dontSendNotification);
        break;
    }
    default:
        jassertfalse;
        break;
    }

    if (item) {
        item->getProperties().set("X-ItemID", itemId);
        item->addListener(&impl);
    }

    return item;
}

juce::ToolbarButton *MainWindow::Impl::MainToolbarFactory::createFontaudioButton(int itemId, fontaudio::IconName iconName, float iconSize, const juce::String &text)
{
    Impl &impl = *impl_;
    MyLookAndFeel &lnf = static_cast<MyLookAndFeel &>(impl.self_->getLookAndFeel());
    fontaudio::IconHelper &fontaudio = lnf.getFontAudio();

    juce::Colour colourNormal = lnf.findColour(juce::TextButton::ColourIds::textColourOffId);
    juce::Colour colourOn = lnf.findColour(juce::TextButton::ColourIds::textColourOnId);

    fontaudio::RenderedIcon imageNormal = fontaudio.getIcon(iconName, iconSize, colourNormal);
    fontaudio::RenderedIcon imageOn = fontaudio.getIcon(iconName, iconSize, colourOn);

    std::unique_ptr<juce::DrawableImage> drawableNormal(new juce::DrawableImage);
    std::unique_ptr<juce::DrawableImage> drawableOn(new juce::DrawableImage);
    drawableNormal->setImage(imageNormal);
    drawableOn->setImage(imageOn);

    return new juce::ToolbarButton(itemId, text, std::move(drawableNormal), std::move(drawableOn));
}
