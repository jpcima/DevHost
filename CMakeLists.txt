cmake_minimum_required(VERSION 3.12)
project(DevHost VERSION "0.0.0")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory("thirdparty/JUCE")
juce_add_module("thirdparty/fontaudio/wrappers/juce/module/fontaudio" ALIAS_NAMESPACE fontaudio)

juce_add_gui_app(DevHost
  PRODUCT_NAME "DevHost")

target_sources(DevHost
  PRIVATE
    "sources/Application.cpp"
    "sources/LookAndFeel.cpp"
    "sources/MainWindow.cpp"
    "sources/PluginSlot.cpp")

target_compile_definitions(DevHost
  PUBLIC
    "PROJECT_NAME=\"${PROJECT_NAME}\""
    "PROJECT_VERSION=\"${PROJECT_VERSION}\""
    "JUCE_PLUGINHOST_VST=1"
    "JUCE_PLUGINHOST_VST3=1"
    "JUCE_PLUGINHOST_AU=1"
    "JUCE_PLUGINHOST_LADSPA=1"
    "JUCE_WEB_BROWSER=0"
    "JUCE_USE_CURL=0"
    "JUCE_DISPLAY_SPLASH_SCREEN=0")

target_include_directories(DevHost
  PRIVATE "sources")

#juce_add_binary_data(DevHostBinaryData
#  SOURCES
#    )

target_link_libraries(DevHost
  PRIVATE
    #DevHostBinaryData
    fontaudio::fontaudio
    juce::juce_gui_basics
    juce::juce_audio_processors
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
