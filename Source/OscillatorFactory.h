/*
  ==============================================================================

    OscillatorFactory.h
    Created: 30 Dec 2017 3:18:35pm

  ==============================================================================
*/

#pragma once

template<typename SampleType>
class OscillatorFactory //: public juce::dsp::Oscillator<SampleType>
{
public:
    using NumericType = typename juce::dsp::SampleTypeHelpers::ElementType<SampleType>::Type;
    
    //==============================================================================
    OscillatorFactory()
    {
        //
    }
    
    //==============================================================================
    OscillatorFactory(const std::function<NumericType (NumericType)>& function, size_t lookupTableNumPoints = 0)
    {
        initialise (function, lookupTableNumPoints);
    }
    
    //==============================================================================
    void initialise (const std::function<NumericType (NumericType)>& function, size_t lookupTableNumPoints = 0)
    {
        if (lookupTableNumPoints != 0)
        {
            auto* table = new juce::dsp::LookupTableTransform<NumericType> (function, static_cast <NumericType> (-1.0 * double_Pi), static_cast<NumericType> (double_Pi), lookupTableNumPoints);
            //
            lookupTable = table;
            generator = [table] (NumericType x) { return (*table) (x); };
        }
        else
        {
            generator = function;
        }
    }
    
    //==============================================================================
    void setFrequency (NumericType newGain, bool force = false) noexcept
    {
        frequency.setValue (newGain);
    }
    
    //==============================================================================
    NumericType getFrequency() const noexcept
    {
        return frequency.getTargetValue();
    }
    
    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec) noexcept
    {
        sampleRate = static_cast<NumericType> (spec.sampleRate);
        rampBuffer.resize ((int) spec.maximumBlockSize);
        //
        reset();
    }
    
    //==============================================================================
    void reset() noexcept
    {
        pos = 0.0;
        
        if (sampleRate > 0)
            frequency.reset (sampleRate, 0.05);
    }
    
    //==============================================================================
    SampleType JUCE_VECTOR_CALLTYPE processSample (SampleType) noexcept
    {
        auto increment = static_cast<NumericType> (2.0 * double_Pi) * frequency.getNextValue() / sampleRate;
        auto value = generator (pos - static_cast<NumericType> (double_Pi));
        pos = std::fmod (pos + increment, static_cast<NumericType> (2.0 * double_Pi));
        //
        return value;
    }
    
    //==============================================================================
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto&& outBlock = context.getOutputBlock();
        //
        jassert (context.getInputBlock().getNumChannels() == 0 || (! context.usesSeparateInputAndOutputBlocks()));
        jassert (outBlock.getNumSamples() <= static_cast<size_t> (rampBuffer.size()));
        //
        auto len           = outBlock.getNumSamples();
        auto numChannels   = outBlock.getNumChannels();
        auto baseIncrement = static_cast<NumericType> (2.0 * double_Pi) / sampleRate;
        //
        if (frequency.isSmoothing())
        {
            auto* buffer = rampBuffer.getRawDataPointer();
            
            for (size_t i = 0; i < len; ++i)
            {
                buffer[i] = pos - static_cast<NumericType> (double_Pi);
                
                pos = std::fmod (pos + (baseIncrement * frequency.getNextValue()), static_cast<NumericType> (2.0 * double_Pi));
            }
            //
            for (size_t ch = 0; ch < numChannels; ++ch)
            {
                auto* dst = outBlock.getChannelPointer (ch);
                //
                for (size_t i = 0; i < len; ++i)
                    dst[i] = generator (buffer[i]);
                    }
        }
        else
        {
            auto freq = baseIncrement * frequency.getNextValue();
            //
            for (size_t ch = 0; ch < numChannels; ++ch)
            {
                auto p = pos;
                auto* dst = outBlock.getChannelPointer (ch);
                //
                for (size_t i = 0; i < len; ++i)
                {
                    dst[i] = generator (p - static_cast<NumericType> (double_Pi));
                    p = std::fmod (p + freq, static_cast<NumericType> (2.0 * double_Pi));
                }
            }
            //
            pos = std::fmod (pos + freq * static_cast<NumericType> (len), static_cast<NumericType> (2.0 * double_Pi));
        }
    }
    
private:
    //==============================================================================
    std::function<NumericType (NumericType)> generator;
    ScopedPointer<juce::dsp::LookupTableTransform<NumericType>> lookupTable;
    Array<NumericType> rampBuffer;
    LinearSmoothedValue<NumericType> frequency {static_cast<NumericType> (440.0)};
    NumericType sampleRate = 48000.0, pos = 0.0;
};
