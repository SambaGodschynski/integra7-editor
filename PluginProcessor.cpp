#include <iostream>
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <algorithm>
#include <sstream>
#include <components/Common.h>
#include <iostream>
#include <Helper.h>
#include <integra7/Model.h>

constexpr size_t MidiBufferReserveBytes = 1024 * 1024 * 4;

PluginProcessor::PluginProcessor()
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
{
}

PluginProcessor::~PluginProcessor()
{
}

void PluginProcessor::releaseResources()
{
}

const juce::String PluginProcessor::getName() const
{
	return "Integra 7 Editor";
}

bool PluginProcessor::acceptsMidi() const
{
	return true;
}

bool PluginProcessor::producesMidi() const
{
	return true;
}

bool PluginProcessor::isMidiEffect() const
{
	return true;
}

double PluginProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int PluginProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
				// so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
	return 0;
}

void PluginProcessor::setCurrentProgram(int)
{
}

const juce::String PluginProcessor::getProgramName(int)
{
	return "";
}

void PluginProcessor::changeProgramName(int, const juce::String&)
{
}

void PluginProcessor::prepareToPlay(double, int)
{
	localMidiBuffer.ensureSize(MidiBufferReserveBytes);
}


bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
	juce::MidiBuffer& inOutMidiBff)
{
	if (inOutMidiBff.getNumEvents() > 0) 
	{
		for (const juce::MidiMessageMetadata &metadata : inOutMidiBff)
		{
			if (!metadata.getMessage().isSysEx())
			{
				continue;
			}
			auto data = metadata.data;
			onSysexRexecived(data, metadata.numBytes);
		}
		inOutMidiBff.clear();
	}
	juce::ScopedNoDenormals noDenormals;
	auto totalNumOutputChannels = getTotalNumOutputChannels();
	for (auto i = 0; i < totalNumOutputChannels; ++i)
	{
		buffer.clear(i, 0, buffer.getNumSamples());
	}
	const std::lock_guard<Mutex> lock(midiBufferMutex);
	auto it = localMidiBuffer.cbegin();
	for(; it != localMidiBuffer.cend(); ++it)
	{
		inOutMidiBff.addEvent((*it).getMessage(), (*it).samplePosition);
	}
	localMidiBuffer.clear();
}

bool PluginProcessor::hasEditor() const
{
	return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
	return new PluginEditor(*this);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock&)
{
}

void PluginProcessor::setStateInformation(const void*, int)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new PluginProcessor();
}

// I7Host ////////////////////////////////////////////////////////////////


void PluginProcessor::sendSysex(const unsigned char* sysexData, size_t numBytes)
{
	DEBUGONLY(std::cout << "S:" << bytesToString(sysexData, sysexData + numBytes) << std::endl);
	const std::lock_guard<Mutex> lock(midiBufferMutex);
	int eventCount = localMidiBuffer.getNumEvents();
	localMidiBuffer.addEvent(sysexData, (int)numBytes, eventCount);
}