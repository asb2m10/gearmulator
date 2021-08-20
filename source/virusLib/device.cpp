#include "device.h"
#include "romfile.h"

namespace virusLib
{
	// 128k words beginning at $20000
	constexpr dsp56k::TWord g_externalMemStart	= 0x020000;
	constexpr dsp56k::TWord g_memorySize		= 0x040000;

	Device::Device(const std::string& _romFileName)
		: synthLib::Device(g_memorySize, g_externalMemStart)
		, m_rom(_romFileName)
		, m_syx(getHDI08(), m_rom)
	{
		if(!m_rom.isValid())
			return;

		auto loader = m_rom.bootDSP(getDSP(), getPeriph());

		startDSPThread();

		loader.join();

		dummyProcess(8);

		m_syx.sendInitControlCommands();

		dummyProcess(8);

		m_syx.createDefaultState();
	}

	float Device::getSamplerate() const
	{
		return 12000000.0f / 256.0f;
	}

	bool Device::isValid() const
	{
		return m_rom.isValid();
	}

	void Device::process(float** _inputs, float** _outputs, size_t _size, const std::vector<synthLib::SMidiEvent>& _midiIn, std::vector<synthLib::SMidiEvent>& _midiOut)
	{
		synthLib::Device::process(_inputs, _outputs, _size, _midiIn, _midiOut);
		m_syx.process(_size);

		m_numSamplesProcessed += static_cast<uint32_t>(_size);
	}

	bool Device::getState(std::vector<uint8_t>& _state, synthLib::StateType _type)
	{
		return m_syx.getState(_state, _type);
	}

	bool Device::setState(const std::vector<uint8_t>& _state, synthLib::StateType _type)
	{
		return m_syx.setState(_state, _type);
	}

	uint32_t Device::getInternalLatencySamples() const
	{
		return 300;		// hard to belive but this is what I figured out by measuring with the init patch
	}

	bool Device::sendMidi(const synthLib::SMidiEvent& _ev, std::vector<synthLib::SMidiEvent>& _response)
	{
		if(_ev.sysex.empty())
		{
//			LOG("MIDI: " << std::hex << (int)_ev.a << " " << (int)_ev.b << " " << (int)_ev.c);
			auto ev = _ev;
			ev.offset += m_numSamplesProcessed + getLatencySamples();
			return m_syx.sendMIDI(ev, true);
		}

		std::vector<synthLib::SMidiEvent> responses;

		if(!m_syx.sendSysex(_ev.sysex, true, responses))
			return false;

		for (const auto& response : responses)
			_response.emplace_back(response);

		return true;
	}

	void Device::readMidiOut(std::vector<synthLib::SMidiEvent>& _midiOut)
	{
		while(getHDI08().hasTX())
		{
			if(m_midiOutParser.append(getHDI08().readTX()))
			{
				const auto midi = m_midiOutParser.getMidiData();
				_midiOut.insert(_midiOut.end(), midi.begin(), midi.end());
				m_midiOutParser.clearMidiData();
			}
		}
	}

	void Device::onAudioWritten()
	{
		m_numSamplesWritten += 1;

		m_syx.sendPendingMidiEvents(m_numSamplesWritten >> 1);
	}

	std::vector<std::string> Device::getSingleNames()
	{
		std::vector<std::string> ret;
		ROMFile::TPreset preset;

		for(int i=0;i<255;i++)
		{
			m_rom.getSingle(0, i, preset);
			ret.push_back(m_rom.getSingleName(preset));
		}

		return ret;
	}
}