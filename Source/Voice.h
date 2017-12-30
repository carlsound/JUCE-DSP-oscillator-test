/*
  ==============================================================================

    Voice.h
    Created: 29 Dec 2017 8:33:35pm

  ==============================================================================
*/

#pragma once

#include "OscillatorTemplate.h"

//==============================================================================
class Voice : public juce::MPESynthesiserVoice
{
public:
	//==============================================================================
	Voice()
    {
        voiceProcessorChain.get<masterGainIndex>().setGainLinear (0.7f);
    }
    
	//==============================================================================
    void prepare (const juce::dsp::ProcessSpec & spec)
    {
        tempBlock = juce::dsp::AudioBlock<float> (heapBlock, spec.numChannels, spec.maximumBlockSize);
        //
        voiceProcessorChain.prepare (spec);
    }
    
    //==============================================================================
    void noteStarted() override
    {
        voiceProcessorChain.get<masterGainIndex>().setGainLinear (getCurrentlyPlayingNote().noteOnVelocity.asUnsignedFloat());
        //
        voiceProcessorChain.get<osc1Index>().setFrequency ((float) getCurrentlyPlayingNote().getFrequencyInHertz());
    }
    
    //==============================================================================
    void notePitchbendChanged() override
    {
        voiceProcessorChain.get<osc1Index>().setFrequency ((float) getCurrentlyPlayingNote().getFrequencyInHertz());
    }
    
    //==============================================================================
    void noteStopped (bool /* allowTailOff */) override
    {
        clearCurrentNote();
    }
    
    //==============================================================================
    void notePressureChanged() override {}
    
    //==============================================================================
    void noteTimbreChanged() override {}
    
    //==============================================================================
    void noteKeyStateChanged() override {}
    
    //==============================================================================
    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        auto block = tempBlock.getSubBlock(0, (size_t) numSamples);
        block.clear();
        //
        juce::dsp::ProcessContextReplacing<float> context (block);
        voiceProcessorChain.process (context);
        //
        juce::dsp::AudioBlock<float> (outputBuffer) .getSubsetChannelBlock((size_t) startSample, (size_t) numSamples) .add (tempBlock);
    }

private:
	//==============================================================================
	juce::HeapBlock<char> heapBlock;
	juce::dsp::AudioBlock<float> tempBlock;
    //
	enum{osc1Index, masterGainIndex};
    //
    juce::dsp::ProcessorChain< OscillatorTemplate<float>, juce::dsp::Gain<float> > voiceProcessorChain;
    //
	static constexpr size_t lfoUpdateRate = 100;
	size_t lfoUpdateCounter = lfoUpdateRate;
};
