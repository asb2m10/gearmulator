#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "version.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
	, m_btSingleMode("Single Mode")
	, m_btMultiMode("Multi Mode")
{
    ignoreUnused (processorRef);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

	addAndMakeVisible(m_mode);
	addAndMakeVisible(m_presets);

	m_mode.setTopLeftPosition(0,0);
	m_mode.addItem("Single Mode", 1);
	m_mode.addItem("Multi Mode", 2);
	m_mode.setSelectedId(1);
	m_mode.setSize(120,30);

	m_mode.onChange = [this]()
	{
		if (m_mode.getSelectedId() == 1)
			switchPlayMode(0);
		else
			switchPlayMode(2);
	};

	m_presets.setTopRightPosition(400-250,0);
	m_presets.setSize(250,30);

	for (int i=0; i<127; ++i)
	{
		m_presets.addItem(processorRef.singlePresetNames[i], i+1);
	}
	m_presets.setSelectedId(processorRef.getCurrentProgram()+1);

	m_presets.onChange = [this]()
	{
		processorRef.setCurrentProgram(m_presets.getSelectedId()-1);
	};
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);

	std::string message = "DSP 56300 Emulator\nVersion " + std::string(g_pluginVersionString) + "\n" __DATE__ " " __TIME__;

	if(!processorRef.isPluginValid())
		message += "\n\nNo ROM, no sound!\nCopy ROM next to plugin, must end with .bin";

	g.drawFittedText(message, getLocalBounds(), juce::Justification::centred, 2);
    g.drawFittedText("To donate: paypal.me/dsp56300", getLocalBounds(), juce::Justification::centredBottom, 2);
}

void AudioPluginAudioProcessorEditor::resized()
{
	// This is generally where you'll want to lay out the positions of any
	// subcomponents in your editor..
}

void AudioPluginAudioProcessorEditor::switchPlayMode(uint8_t _playMode) const
{
	synthLib::SMidiEvent ev;
	ev.sysex = { 0xf0, 0x00, 0x20, 0x33, 0x01, 0x00, 0x72, 0x0, 0x7a, _playMode, 0xf7};
	processorRef.addMidiEvent(ev);
}
