/*
  ==============================================================================

    Oscillator.h
    Created: 29 Dec 2017 8:34:24pm

  ==============================================================================
*/

#pragma once

#include "OscillatorFactory.h"

template <typename Type>
class OscillatorTemplate
{
public:
	//==============================================================================
    OscillatorTemplate()
	{
        auto& osc = oscillatorProcessorChain.template get<oscIndex>();
		osc.initialise ([] (Type x) { return std::sin (x); }, 128);
	}

	//==============================================================================
	void setFrequency(Type newValue, bool force = false)
	{
        ignoreUnused(newValue);
        ignoreUnused(force);
        //
        auto& osc = oscillatorProcessorChain.template get<oscIndex>();
        //osc.setFrequency (newValue, force);
		osc.setFrequency(newValue);
	}

	//==============================================================================
	void setLevel(Type newValue)
	{
		ignoreUnused(newValue);
        //
        auto& gain = oscillatorProcessorChain.template get<gainIndex>();
        gain.setGainLinear (newValue);
	}

	//==============================================================================
	void reset() noexcept
    {
        oscillatorProcessorChain.reset();
    }

	//==============================================================================
	template <typename ProcessContext>
	void process(const ProcessContext& context) noexcept
	{
        auto&& outBlock = context.getOutputBlock();
        const auto numSamples = outBlock.getNumSamples();
        //
        auto blockToUse = tempBlock.getSubBlock(0, numSamples);
        auto contextToUse = juce::dsp::ProcessContextReplacing<Type> (blockToUse);
        //
        oscillatorProcessorChain.process (contextToUse);
        outBlock.add (blockToUse);
	}

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec)
	{
        ignoreUnused(spec);
        //
        oscillatorProcessorChain.prepare (spec);
        tempBlock = juce::dsp::AudioBlock<Type> (heapBlock, spec.numChannels, spec.maximumBlockSize);
	}

private:
	//==============================================================================
	enum{oscIndex, gainIndex};
    //
	juce::dsp::ProcessorChain< OscillatorFactory<Type>, juce::dsp::Gain<Type> > oscillatorProcessorChain;
    //
	juce::HeapBlock<char> heapBlock;
	juce::dsp::AudioBlock<Type> tempBlock;
};
