#pragma once

#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>

// Core auto gain-riding engine: tracks the level of a detector signal (the
// vocal input, or an external sidechain "music" bus), compares it against a
// target level, and produces a smoothed makeup-gain that pulls the tracked
// level back toward the target, clamped to +-range.
class RiderEngine
{
public:
    void prepare (double sampleRate, int /*maxBlockSize*/)
    {
        fs = sampleRate;

        // RMS-style envelope for level detection (~10 ms window).
        levelEnvelopeCoeff = timeConstantToCoeff (0.010);

        updateTimeConstants();

        currentLevelDb = -60.0f;
        smoothedGainDb = 0.0f;
        reset();
    }

    void reset()
    {
        levelEnvelope = 0.0f;
        smoothedGainDb = 0.0f;
        heldPeakDb = -100.0f;
        windowMaxDb = -100.0f;
        holdTimeRemaining = 0.0f;
    }

    void setTargetDb (float db)      { targetDb = db; }
    void setRangeUpDb (float db)     { rangeUpDb = db; }
    void setRangeDownDb (float db)   { rangeDownDb = db; }
    void setEnabled (bool e)         { enabled = e; }

    // Independent attack (rise, gain increasing) and release (fall, gain
    // decreasing) times, in milliseconds.
    void setAttackMs (float ms)
    {
        attackMs = juce::jmax (1.0f, ms);
        updateTimeConstants();
    }

    void setReleaseMs (float ms)
    {
        releaseMs = juce::jmax (1.0f, ms);
        updateTimeConstants();
    }

    // Feed one block of detector samples (mono-summed) purely for envelope
    // tracking; call once per audio block before processGainForSample().
    void updateEnvelopeBlock (const float* detectorMono, int numSamples)
    {
        float blockPeakLinear = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            const float rectified = std::abs (detectorMono[i]);
            const float sq = rectified * rectified;
            levelEnvelope += levelEnvelopeCoeff * (sq - levelEnvelope);
            blockPeakLinear = juce::jmax (blockPeakLinear, rectified);
        }

        const float rms = std::sqrt (juce::jmax (levelEnvelope, 1.0e-12f));
        currentLevelDb = juce::Decibels::gainToDecibels (rms, -100.0f);

        // Peak-hold meter: a new, louder peak is shown instantly. A quieter
        // reading is held for 0.5s (in case it's just a momentary dip)
        // before the display drops to the loudest peak seen during that
        // 0.5s window.
        const float blockPeakDb = juce::Decibels::gainToDecibels (blockPeakLinear, -100.0f);
        const float blockSeconds = (float) numSamples / (float) fs;

        if (blockPeakDb > heldPeakDb)
        {
            heldPeakDb = blockPeakDb;
            windowMaxDb = blockPeakDb;
            holdTimeRemaining = peakHoldSeconds;
        }
        else
        {
            windowMaxDb = juce::jmax (windowMaxDb, blockPeakDb);
            holdTimeRemaining -= blockSeconds;

            if (holdTimeRemaining <= 0.0f)
            {
                heldPeakDb = windowMaxDb;
                windowMaxDb = -100.0f;
                holdTimeRemaining = peakHoldSeconds;
            }
        }

        lastPeakDb.store (heldPeakDb, std::memory_order_relaxed);

        if (! enabled)
        {
            smoothedGainDb += 0.0f; // hold whatever value it decays to below
        }

        // Desired gain correction to pull the tracked level to target,
        // clamped to the allowed up/down range.
        float desiredGainDb = enabled ? (targetDb - currentLevelDb) : 0.0f;
        desiredGainDb = juce::jlimit (-rangeDownDb, rangeUpDb, desiredGainDb);

        const float coeff = (desiredGainDb > smoothedGainDb) ? riseCoeff : fallCoeff;
        smoothedGainDb += coeff * (desiredGainDb - smoothedGainDb);

        lastAppliedGainDb.store (smoothedGainDb, std::memory_order_relaxed);
        lastLevelDb.store (currentLevelDb, std::memory_order_relaxed);
    }

    float getCurrentGainLinear() const
    {
        return juce::Decibels::decibelsToGain (smoothedGainDb);
    }

    // For UI metering (thread-safe reads).
    float getMeterLevelDb() const { return lastLevelDb.load (std::memory_order_relaxed); }
    float getMeterGainDb()  const { return lastAppliedGainDb.load (std::memory_order_relaxed); }
    float getMeterPeakDb()  const { return lastPeakDb.load (std::memory_order_relaxed); }

private:
    float timeConstantToCoeff (double seconds) const
    {
        if (fs <= 0.0) return 1.0f;
        return 1.0f - std::exp (-1.0f / (float) (seconds * fs));
    }

    void updateTimeConstants()
    {
        riseCoeff = timeConstantToCoeff ((double) attackMs / 1000.0);
        fallCoeff = timeConstantToCoeff ((double) releaseMs / 1000.0);
    }

    double fs = 44100.0;

    float levelEnvelopeCoeff = 0.01f;
    float levelEnvelope = 0.0f;
    float currentLevelDb = -60.0f;

    float targetDb = -18.0f;
    float rangeUpDb = 10.0f;
    float rangeDownDb = 10.0f;
    float attackMs = 300.0f;
    float releaseMs = 900.0f;
    bool  enabled = true;

    float riseCoeff = 0.01f;
    float fallCoeff = 0.01f;
    float smoothedGainDb = 0.0f;

    static constexpr float peakHoldSeconds = 0.5f;
    float heldPeakDb = -100.0f;
    float windowMaxDb = -100.0f;
    float holdTimeRemaining = 0.0f;

    std::atomic<float> lastLevelDb { -60.0f };
    std::atomic<float> lastAppliedGainDb { 0.0f };
    std::atomic<float> lastPeakDb { -100.0f };
};
