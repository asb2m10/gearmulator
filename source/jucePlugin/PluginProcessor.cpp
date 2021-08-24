#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "../synthLib/os.h"


class MessageCCSender
{
	int m_cc;
	synthLib::Plugin *m_plugin;
public:
	MessageCCSender(synthLib::Plugin *plugin, int cc)
	{
		m_cc = cc;
		m_plugin = plugin;
	}

	void sendCC(int newValue) 
	{
		if ( m_cc < 127 ) 
		{
			synthLib::SMidiEvent ev(0xB0, m_cc, newValue);
			m_plugin->addMidiEvent(ev);
		} else {
			synthLib::SMidiEvent ev(0xA0, m_cc-128, newValue);
			m_plugin->addMidiEvent(ev);
		}
	}
};

class AudioParameterCC: public juce::AudioParameterInt, MessageCCSender
{
public:
	AudioParameterCC(int cc, juce::String name, synthLib::Plugin *plugin, int steps = 127) : AudioParameterInt(juce::String(cc), name, 0, steps, 0), MessageCCSender(plugin, cc)
	{
	}

	void valueChanged(int newValue) override
	{
		sendCC(newValue);
	}
};

class AudioParameterChoiceCC: public juce::AudioParameterChoice, MessageCCSender
{
public:
	AudioParameterChoiceCC(int cc, juce::String name, synthLib::Plugin *plugin, juce::StringArray choices) : AudioParameterChoice(juce::String(cc), name, choices, 0), MessageCCSender(plugin, cc)
	{
	}

	void valueChanged(int newValue) override
	{
		sendCC(newValue);
	}
};

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       )
	, m_device(synthLib::findROM())
	, m_plugin(&m_device)
{
	singlePresetNames = m_device.getSingleNames();

	juce::StringArray lfoTypes = {
		"Sine", "Tri", "Saw", "Squr", "S&H", "S&G", "Wave3", "Wave4", "Wave5", "Wave6", "Wave7",
		"Wave8", "Wave9", "Wave10", "Wave11", "Wave12", "Wave13", "Wave14", "Wave15", "Wave16",
		"Wave17", "Wave18", "Wave19", "Wave20", "Wave21", "Wave22","Wave23", "Wave24", "Wave25",
		"Wave26", "Wave27", "Wave28", "Wave29", "Wave30", "Wave31", "Wave32","Wave33", "Wave34",
		"Wave35", "Wave36", "Wave37", "Wave38", "Wave39", "Wave40", "Wave41", "Wave42", "Wave43",
		"Wave44", "Wave45", "Wave46", "Wave47", "Wave48", "Wave49", "Wave50", "Wave51", "Wave52", 
		"Wave53", "Wave54", "Wave55", "Wave56", "Wave57", "Wave58", "Wave59", "Wave60", "Wave61",
		"Wave62",  "Wave63", "Wave64"
	};

	addParameter(new AudioParameterCC(2, "BREATH WHEEL", &m_plugin));
	addParameter(new AudioParameterCC(5, "PORTAMENTO TIME", &m_plugin));
	addParameter(new AudioParameterCC(7, "VOLUME", &m_plugin));
	addParameter(new AudioParameterCC(10, "PAN", &m_plugin));
	addParameter(new AudioParameterCC(17, "OSCILLATORS 1 SHAPE", &m_plugin));
	addParameter(new AudioParameterCC(18, "OSCILLATORS 1 WAVE SEL / PW", &m_plugin));
	addParameter(new AudioParameterCC(19, "OSCILLATORS 1 WAVE", &m_plugin, 63));
	addParameter(new AudioParameterCC(20, "OSCILLATORS 1 SEMITONE", &m_plugin));
	addParameter(new AudioParameterCC(21, "OSCILLATORS 1 KEYFOLLOW", &m_plugin));
	addParameter(new AudioParameterCC(22, "OSCILLATORS 2 SHAPE", &m_plugin));
	addParameter(new AudioParameterCC(23, "OSCILLATORS 2 WAVE SEL / PW", &m_plugin));
	addParameter(new AudioParameterCC(24, "OSCILLATORS 2 WAVE", &m_plugin, 63));
	addParameter(new AudioParameterCC(25, "OSCILLATORS 2 SEMITONE", &m_plugin));
	addParameter(new AudioParameterCC(26, "OSCILLATORS DETUNE 2/3", &m_plugin));
	addParameter(new AudioParameterCC(27, "OSCILLATORS FM AMOUNT", &m_plugin));
	addParameter(new AudioParameterCC(28, "OSCILLATORS SYNC", &m_plugin, 1));
	addParameter(new AudioParameterCC(29, "OSCILLATORS 2 Flt Env-Ptch", &m_plugin));
	addParameter(new AudioParameterCC(30, "OSCILLATORS 2 Flt Env-FM", &m_plugin));
	addParameter(new AudioParameterCC(31, "OSCILLATORS 2 KEYFOLLOW", &m_plugin));
	addParameter(new AudioParameterCC(32, "BANK SELECT", &m_plugin));
	addParameter(new AudioParameterCC(33, "MIX OSC BAL", &m_plugin));
	addParameter(new AudioParameterCC(34, "MIX SUB OSC", &m_plugin));
	addParameter(new AudioParameterCC(35, "SUB OSCILLATOR SHAPE", &m_plugin, 1));
	addParameter(new AudioParameterCC(36, "MIX OSC VOL (SATURATION)", &m_plugin));
	addParameter(new AudioParameterCC(37, "MIX NOISE", &m_plugin));
	addParameter(new AudioParameterCC(38, "MIX RING MOD", &m_plugin));
	addParameter(new AudioParameterCC(39, "NOISE COLOR", &m_plugin));
	addParameter(new AudioParameterCC(40, "FILTER CUT OFF 1", &m_plugin));
	addParameter(new AudioParameterCC(41, "FILTER CUT OFF 2", &m_plugin));
	addParameter(new AudioParameterCC(42, "FILTER RESONANCE 1", &m_plugin));
	addParameter(new AudioParameterCC(43, "FILTER RESONANCE 2", &m_plugin));
	addParameter(new AudioParameterCC(44, "FILTER ENV AMOUNT 1", &m_plugin));
	addParameter(new AudioParameterCC(45, "FILTER ENV AMOUNT 2", &m_plugin));
	addParameter(new AudioParameterCC(46, "FILTER KEYFOLLOW 1", &m_plugin));
	addParameter(new AudioParameterCC(47, "FILTER KEYFOLLOW 2", &m_plugin));
	addParameter(new AudioParameterCC(48, "FILTER BALANCE", &m_plugin));

	addParameter(new AudioParameterChoiceCC(49, "FILTER SATURATION CURVE", &m_plugin, 
		juce::StringArray { "Off", "Light", "Soft", "Middle", "Hard", "Digital", "Shaper", "Rectifiere", "BitReduce",
		"RateReduc", "RateFlw", "LowPass", "LowFtw", "HighPass", "HighFlw" }));

	addParameter(new AudioParameterChoiceCC(51, "FILTER 1 MODE", &m_plugin, 
		juce::StringArray { "LP", "HP", "BP", "BS", "1P Analog", "2P Analog", "3P Analog", "4P Analog" }));

	addParameter(new AudioParameterChoiceCC(52, "FILTER 2 MODE", &m_plugin, 
		juce::StringArray { "LP", "HP", "BP", "BS" }));

	addParameter(new AudioParameterChoiceCC(53, "FILTER ROUTING", &m_plugin, 
		juce::StringArray { "Ser4", "Ser6", "Par4", "Split"}));

	addParameter(new AudioParameterCC(54, "FILTER ENV ATTACK", &m_plugin));
	addParameter(new AudioParameterCC(55, "FILTER ENV DECAY", &m_plugin));
	addParameter(new AudioParameterCC(56, "FILTER ENV SUSTAIN", &m_plugin));
	addParameter(new AudioParameterCC(57, "FILTER ENV TIME", &m_plugin));
	addParameter(new AudioParameterCC(58, "FILTER ENV RELEASE", &m_plugin));
	addParameter(new AudioParameterCC(59, "AMP ENV ATTACK", &m_plugin));
	addParameter(new AudioParameterCC(60, "AMP ENV DECAY", &m_plugin));
	addParameter(new AudioParameterCC(61, "AMP ENV SUSTAIN", &m_plugin));
	addParameter(new AudioParameterCC(62, "AMP ENV TIME", &m_plugin));
	addParameter(new AudioParameterCC(63, "AMP ENV RELEASE", &m_plugin));
	addParameter(new AudioParameterCC(67, "LFO1 RATE", &m_plugin));
	addParameter(new AudioParameterChoiceCC(68, "LFO1 SHAPE", &m_plugin, lfoTypes));
	addParameter(new AudioParameterChoiceCC(69, "LFO1 ENV MODE", &m_plugin,
		juce::StringArray { "Off", "On" }));

	addParameter(new AudioParameterChoiceCC(70, "LFO1 MODE (POLY/MONO)", &m_plugin,
		juce::StringArray { "Poly", "Mono" }));

	addParameter(new AudioParameterCC(71, "LFO1 CONTOUR", &m_plugin));
	addParameter(new AudioParameterCC(72, "LFO1 KEYFOLLOW", &m_plugin));
	addParameter(new AudioParameterCC(73, "LFO1 TRIG PHASE", &m_plugin));
	addParameter(new AudioParameterCC(79, "LFO2 RATE", &m_plugin));
	addParameter(new AudioParameterChoiceCC(80, "LFO2 SHAPE", &m_plugin, lfoTypes));
	addParameter(new AudioParameterChoiceCC(81, "LFO2 ENV MODE", &m_plugin,
		juce::StringArray { "Off", "On" }));

	addParameter(new AudioParameterChoiceCC(82, "LFO2 MODE (POLY/MONO)", &m_plugin,
		juce::StringArray { "Poly", "Mono" }));

	addParameter(new AudioParameterCC(83, "LFO2 CONTOUR", &m_plugin));
	addParameter(new AudioParameterCC(84, "LFO2 KEYFOLLOW", &m_plugin));
	addParameter(new AudioParameterCC(85, "LFO2 TRIG PHASE", &m_plugin));
	addParameter(new AudioParameterCC(91, "PATCH VOLUME", &m_plugin));
	addParameter(new AudioParameterChoiceCC(93, "TRANSPOSE", &m_plugin,
		juce::StringArray { "-2", "-1", "0", "+1", "+2" }));

	addParameter(new AudioParameterChoiceCC(94, "KEY MODE", &m_plugin,
		juce::StringArray { "Poly", "Mono1" , "Mono2", "Mono3", "Mono4" }));

	addParameter(new AudioParameterChoiceCC(97, "UNISON MODE", &m_plugin,
		juce::StringArray { "Off", "Twin2", "Twin3", "Twin4", "Twin5", "Twin6", "Twin7",
							"Twin8", "Twin9", "Twin10", "Twin11", "Twin12","Twin13", "Twin14",
							"Twin15", "Twin16" }));

	addParameter(new AudioParameterCC(98, "UNISON DETUNE", &m_plugin));
	addParameter(new AudioParameterCC(99, "UNISON PAN SPREAD", &m_plugin));
	addParameter(new AudioParameterCC(100, "UNISON LFO PHASE", &m_plugin));
	addParameter(new AudioParameterChoiceCC(101, "EFFECTS VOCODER INPUT MODE", &m_plugin,
		juce::StringArray { "Off", "Dynamic", "Static", "ToEffects" }));

	addParameter(new AudioParameterChoiceCC(102, "EFFECTS VOCODER INPUT SELECT", &m_plugin,
		juce::StringArray { "In1L", "In1LR", "In1R", "Aux1L", "Aux1LR","Aux1R", "Aux2L", 
							"Aux2LR", "Aux2R" }));

	addParameter(new AudioParameterCC(105, "EFFECTS TYPE/MIX", &m_plugin));
	addParameter(new AudioParameterCC(106, "EFFECTS CHORUS RATE", &m_plugin));
	addParameter(new AudioParameterCC(107, "EFFECTS INTENSITY", &m_plugin));
	addParameter(new AudioParameterCC(108, "EFFECTS CHORUS DELAY", &m_plugin));
	addParameter(new AudioParameterCC(109, "EFFECTS CHORUS FEEDBACK", &m_plugin));
	addParameter(new AudioParameterChoiceCC(110, "EFFECTS CHORUS SHAPE", &m_plugin,
		juce::StringArray { "Sine", "Tri", "Saw", "Squr", "S&H", "S&G" }));

	addParameter(new AudioParameterChoiceCC(112, "EFFECTS DELAY/REVERB MODE", &m_plugin, 
		juce::StringArray { "Off",  "Delay", "Reverb", "Rev+Feedb1", "Rev+Feedb2", "Delay2", "Delay4", 
							"Deleay4", "Delay8", "Pattern1+1", "Pattern2+1", "Pattern3+1", "Pattern4+1", 
							"Pattern5+1", "Pattern2+3", "Pattern2+5", "Pattern3+2","Pattern3+3", "Pattern3+4", 
							"Pattern3+5", "Pattern4+3", "Pattern4+5", "Pattern5+2", "Pattern5+3", "Pattern5+4",
							"Pattern5+5" }));

	addParameter(new AudioParameterCC(113, "EFFECTS SEND", &m_plugin));
	addParameter(new AudioParameterCC(115, "EFFECTS FEEDBACK/DAMPING", &m_plugin));
	addParameter(new AudioParameterCC(116, "EFFECTS DELAY RATE", &m_plugin));
	addParameter(new AudioParameterChoiceCC(117, "EFFECTS DELAY DEPTH / REVERB TYPE", &m_plugin,
		juce::StringArray { "Ambience", "SmallRoom", "LargeRoom", "Hall"}));

	addParameter(new AudioParameterCC(118, "EFFECTS DELAY SHAPE", &m_plugin));
	addParameter(new AudioParameterCC(119, "EFFECTS DELAY COLOR / REVERB COLOR", &m_plugin));

	addParameter(new AudioParameterChoiceCC(129, "ARP MODE", &m_plugin,
		juce::StringArray { "Off", "Up", "Down", "Up&Down", "AsPlayed", "Random", "Chord"}));


	addParameter(new AudioParameterCC(130, "ARP PATTERN SELECT", &m_plugin, 31));
	addParameter(new AudioParameterCC(131, "ARP OCTAVE RANGE", &m_plugin, 3));

	addParameter(new AudioParameterChoiceCC(132, "ARP HOLD", &m_plugin,
		juce::StringArray { "Off", "On" }));

	addParameter(new AudioParameterCC(133, "ARP NOTE LENGTH", &m_plugin));
	addParameter(new AudioParameterCC(134, "ARP NOTE SWING", &m_plugin));

	addParameter(new AudioParameterCC(135, "LFO3 RATE", &m_plugin));
	addParameter(new AudioParameterChoiceCC(136, "LFO3 SHAPE", &m_plugin, lfoTypes));
	addParameter(new AudioParameterChoiceCC(137, "LFO3 MODE (POLY/MONO)", &m_plugin,
		juce::StringArray { "Poly", "Mono" }));
	addParameter(new AudioParameterCC(138, "LFO3 KEYFOLLOW", &m_plugin));

	addParameter(new AudioParameterChoiceCC(139, "LFO3 DESTINATION", &m_plugin,
		juce::StringArray { "Osc1", "Osc1+2", "Osc2", "Pw1", "Pw1+2", "PW2"}));

	addParameter(new AudioParameterCC(140, "LFO3 AMOUNT", &m_plugin));
	addParameter(new AudioParameterCC(141, "LFO3 FADETIME", &m_plugin));	

	addParameter(new AudioParameterCC(144, "CLOCK TEMPO", &m_plugin));

	addParameter(new AudioParameterChoiceCC(145, "ARP CLOCK", &m_plugin,
		juce::StringArray { "Off", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "3/64",
							"3/32", "3/16", "3/8", "1/24", "1/12", "1/6", "1/3","2/3",
							"3/4", "1/1" }));

	addParameter(new AudioParameterChoiceCC(145, "ARP CLOCK", &m_plugin,
		juce::StringArray { "Off", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "3/64",
							"3/32", "3/16", "3/8", "1/24", "1/12", "1/6", "1/3","2/3",
							"3/4", "1/1" }));

	addParameter(new AudioParameterChoiceCC(146, "LFO1 CLOCK", &m_plugin,
		juce::StringArray { "Off", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "3/64",
							"3/32", "3/16", "3/8", "1/24", "1/12", "1/6", "1/3","2/3",
							"3/4", "1/1", "2/1", "4/1", "8/1", "16/1" }));

	addParameter(new AudioParameterChoiceCC(147, "LFO2 CLOCK", &m_plugin,
		juce::StringArray { "Off", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "3/64",
							"3/32", "3/16", "3/8", "1/24", "1/12", "1/6", "1/3","2/3",
							"3/4", "1/1", "2/1", "4/1", "8/1", "16/1" }));


	addParameter(new AudioParameterChoiceCC(148, "DELAY CLOCK", &m_plugin,
		juce::StringArray { "Off", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "3/64",
							"3/32", "3/16", "3/8", "1/24", "1/12", "1/6", "1/3","2/3",
							"3/4" }));

	addParameter(new AudioParameterChoiceCC(149, "LFO3 CLOCK", &m_plugin,
		juce::StringArray { "Off", "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "3/64",
							"3/32", "3/16", "3/8", "1/24", "1/12", "1/6", "1/3","2/3",
							"3/4", "1/1", "2/1", "4/1", "8/1", "16/1" }));

	addParameter(new AudioParameterChoiceCC(153, "CONTROL SMOOTH", &m_plugin,
		juce::StringArray { "Off", "On", "Auto", "Note" }));

	addParameter(new AudioParameterCC(154, "BENDER RANGE UP", &m_plugin));
	addParameter(new AudioParameterCC(155, "BENDER RANGE DOWN", &m_plugin));

	addParameter(new AudioParameterChoiceCC(156, "BENDER SCALE", &m_plugin,
		juce::StringArray { "Linear", "Expotential" }));
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 127;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
	currentProgram = index;
	synthLib::SMidiEvent ev(0xC0, index);
	addMidiEvent(ev);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    return singlePresetNames[index];
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

	m_plugin.setSamplerate(static_cast<float>(sampleRate));
	m_plugin.setBlockSize(samplesPerBlock);
	setLatencySamples(m_plugin.getLatencySamples());
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

	float* inputs[8] = {};
	float* outputs[8] = {};

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
	    const float* in = buffer.getReadPointer(channel);
	    float* out = buffer.getWritePointer(channel);

    	inputs[channel] = const_cast<float*>(in);	// TODO: fixme
    	outputs[channel] = out;
    }

	for(const auto metadata : midiMessages)
	{
		const auto message = metadata.getMessage();

		synthLib::SMidiEvent ev{};

		if(message.isSysEx() || message.getRawDataSize() > 3)
		{
			ev.sysex.resize(message.getRawDataSize());
			memcpy( &ev.sysex[0], message.getRawData(), ev.sysex.size());

			// Juce bug? Or VSTHost bug? Juce inserts f0/f7 when converting VST3 midi packet to Juce packet, but its already there
			if(ev.sysex.size() > 1)
			{
				if(ev.sysex.front() == 0xf0 && ev.sysex[1] == 0xf0)
					ev.sysex.erase(ev.sysex.begin());

				if(ev.sysex.size() > 1)
				{
					if(ev.sysex[ev.sysex.size()-1] == 0xf7 && ev.sysex[ev.sysex.size()-2] == 0xf7)
						ev.sysex.erase(ev.sysex.begin());
				}
			}
		}
		else
		{
			ev.a = message.getRawData()[0];
			ev.b = message.getRawDataSize() > 0 ? message.getRawData()[1] : 0;
			ev.c = message.getRawDataSize() > 1 ? message.getRawData()[2] : 0;
		}

		ev.offset = metadata.samplePosition;

		m_plugin.addMidiEvent(ev);
	}

	midiMessages.clear();

    juce::AudioPlayHead::CurrentPositionInfo pos{0};
	
	auto* playHead = getPlayHead();
	if(playHead)
		playHead->getCurrentPosition(pos);

	m_plugin.process(inputs, outputs, buffer.getNumSamples(), static_cast<float>(pos.bpm), static_cast<float>(pos.ppqPosition), pos.isPlaying);

	m_midiOut.clear();
	m_plugin.getMidiOut(m_midiOut);

	for(size_t i=0; i<m_midiOut.size(); ++i)
	{
		const auto& e = m_midiOut[i];
		if(e.sysex.empty())
		{
			midiMessages.addEvent(juce::MidiMessage (e.a, e.b, e.c, 0.0), 0);
		}
		else
		{
			midiMessages.addEvent(juce::MidiMessage (&e.sysex[0], static_cast<int>(e.sysex.size()), 0.0), 0);
		}
	}
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

	std::vector<uint8_t> state;
	m_plugin.getState(state, synthLib::StateTypeGlobal);
	destData.append(&state[0], state.size());
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
	setState(data, sizeInBytes);
}

void AudioPluginAudioProcessor::getCurrentProgramStateInformation(juce::MemoryBlock& destData)
{
	std::vector<uint8_t> state;
	m_plugin.getState(state, synthLib::StateTypeCurrentProgram);
	destData.append(&state[0], state.size());
}

void AudioPluginAudioProcessor::setCurrentProgramStateInformation(const void* data, int sizeInBytes)
{
	setState(data, sizeInBytes);
}

void AudioPluginAudioProcessor::getLastMidiOut(std::vector<synthLib::SMidiEvent>& dst)
{
	juce::ScopedLock lock(getCallbackLock());
	std::swap(dst, m_midiOut);
	m_midiOut.clear();
}

void AudioPluginAudioProcessor::addMidiEvent(const synthLib::SMidiEvent& ev)
{
	m_plugin.addMidiEvent(ev);
}

void AudioPluginAudioProcessor::setState(const void* _data, size_t _sizeInBytes)
{
	if(_sizeInBytes < 1)
		return;

	std::vector<uint8_t> state;
	state.resize(_sizeInBytes);
	memcpy(&state[0], _data, _sizeInBytes);
	m_plugin.setState(state);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
