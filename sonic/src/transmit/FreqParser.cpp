#include <vector>
#include <deque>

#include "FFTBufferManager/FFTBufferManager.h"
#include "freq_util/bb_freq_util.h"
#include "FreqParser.h"


#ifndef CLAMP
#define CLAMP(min,x,max) (x < min ? min : (x > max ? max : x))
#endif




struct CFreqParser::Impl
{
    FFTBufferManager     fftbm;
    int                  sampleRate;
    std::vector<int32_t> fftData;

    std::deque<std::vector<double> > queueResult;

    Impl(int sample_num) : fftbm(sample_num) {}
};

CFreqParser::CFreqParser(int sampleRate, int sampleChannels, double duration)
{
    int sample_num = (duration * sampleRate * sampleChannels);
    printf("sample_num(%d)\n", sample_num);
    mImpl = new Impl(sample_num);
    mImpl->sampleRate = sampleRate;
    mImpl->fftData.resize(sample_num/2);
}

CFreqParser::~CFreqParser()
{
    delete mImpl;
}

bool CFreqParser::open()
{
    mImpl->queueResult.clear();
    mImpl->fftbm.Reset();
    return true;
}

bool CFreqParser::close()
{
    mImpl->queueResult.clear();
    return true;
}

void calc_freq_values(int32_t fft_data[], int fft_len, int sample_rate, int freq_count, double values[])
{
    for (int i = 0; i < freq_count; ++i)
    {
        unsigned int freq = 0;
        num_to_freq(i, &freq);
        double fftIdx = 2.0 * freq / sample_rate * fft_len;
        double fftIdx_i = 0;
        double fftIdx_f = modf(fftIdx, &fftIdx_i);

        signed char fft_l = (fft_data[(int)fftIdx_i] & 0xFF000000) >> 24;
        signed char fft_r = (fft_data[(int)fftIdx_i + 1] & 0xFF000000) >> 24;
        double fft_l_fl = 1.0 * (fft_l + 80) / 64.;
        double fft_r_fl = 1.0 * (fft_r + 80) / 64.;
        double interpVal = fft_l_fl * (1.0 - fftIdx_f) + fft_r_fl * fftIdx_f;
        values[i] = sqrt(CLAMP(0., interpVal, 1.));
    }
}

bool CFreqParser::putAudioData(const short* pcmData, int samples)
{
    int remain_samples = samples;
    const short* pdata = pcmData;
    while (remain_samples > 0)
    {
        int write_samples = mImpl->fftbm.GrabAudioData(pdata, remain_samples);
        pdata += write_samples;
        remain_samples -= write_samples;

        if (mImpl->fftbm.ComputeFFT(mImpl->fftData.data()))
        {
            std::vector<double> freqValues(32);
            calc_freq_values(mImpl->fftData.data(), mImpl->fftData.size(), mImpl->sampleRate, freqValues.size(), freqValues.data());
            mImpl->queueResult.push_back(freqValues);
        }
    }

    return true;
}

bool CFreqParser::getResult(std::vector<double>& freqValues)
{
    if (!mImpl->queueResult.empty())
    {
        mImpl->queueResult.front().swap(freqValues);
        mImpl->queueResult.pop_front();
        return true;
    }

    return false;
}
