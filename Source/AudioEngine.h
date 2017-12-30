/*
  ==============================================================================

    AudioEngine.h
    Created: 29 Dec 2017 8:33:10pm

  ==============================================================================
*/

#pragma once

#include "Voice.h"

//==============================================================================
class AudioEngine : public juce::MPESynthesiser
{
public:
	static constexpr size_t maxNumVoices = 4;

	//==============================================================================
	AudioEngine()
	{
		for (size_t i = 0; i < maxNumVoices; ++i)
		{
			addVoice(new Voice);
		}
        //
		setVoiceStealingEnabled(true);
	}

	//==============================================================================
	void prepare (const juce::dsp::ProcessSpec& spec) noexcept
	{
		setCurrentPlaybackSampleRate(spec.sampleRate);
        //
		for(auto* v : voices)
		{
			dynamic_cast<Voice*> (v)->prepare(spec);
		}
	}

private:
	//==============================================================================
	void renderNextSubBlock (AudioBuffer<float>& outputAudio, int startSample, int numSamples) override
	{
		MPESynthesiser::renderNextSubBlock(outputAudio, startSample, numSamples);
	}
};
