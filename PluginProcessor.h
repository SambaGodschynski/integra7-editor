#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <list>
#include <memory>
#include <AI7Host.h>
#include <future>

class PluginProcessor : public juce::AudioProcessor, public AI7Host
{
public:
	typedef std::list<std::string> LogCache;
	PluginProcessor();
	~PluginProcessor() override;
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
	using AudioProcessor::processBlock;
	juce::AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override;
	const juce::String getName() const override;
	bool acceptsMidi() const override;
	bool producesMidi() const override;
	bool isMidiEffect() const override;
	double getTailLengthSeconds() const override;
	int getNumPrograms() override;
	int getCurrentProgram() override;
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;
private:
	typedef std::mutex Mutex;
	std::mutex midiBufferMutex;
	juce::MidiBuffer localMidiBuffer;
	LogCache logCache;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
public:
	virtual void sendSysex(const unsigned char*, size_t) override;
};
