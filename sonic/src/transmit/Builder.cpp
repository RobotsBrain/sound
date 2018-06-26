#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <string>
#include <vector>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "freq_util/bb_freq_util.h"

#include "config.h"
#include "RSCodec.h"
#include "sonic/Builder.h"

#define MAX_VOLUME              0.5
#define MAX_SHORT               32767
#define MIN_SHORT               -32768


namespace Sonic {

void makeChirp(short buffer[],int bufferLength,unsigned int freqArray[], int freqArrayLength,
                double duration_secs, long sample_rate, int channels)
{
    int sample_num = duration_secs * sample_rate;
    //printf("sample_num: %d\n", sample_num);

    double theta = 0;
    int idx = 0;
    for (int i=0; i<freqArrayLength; i++) {
        double theta_increment = 2.0 * M_PI * freqArray[i] / sample_rate;
        //printf("theta_increment:%g\n", theta_increment);

        // Generate the samples
        for (int frame = 0; frame < sample_num; frame++) {
#if 1
            double vol = MAX_VOLUME * sqrt(1.0 - (pow(frame - 0.5 * sample_num, 2) / pow(0.5 * sample_num, 2)));
#else
            double vol = 0;
            if (frame < sample_num / 2)
            {
                vol = MAX_VOLUME * sqrt(1.0 - (pow(frame - 0.25 * sample_num, 2) / pow(0.25 * sample_num, 2)));
            }
#endif
            //printf("vol:%g\n", vol);

            double frame_value_float = vol * sin(theta);
            double frame_value_16 = MAX_SHORT * frame_value_float;
            if( frame_value_16 > MAX_SHORT ) frame_value_16 = MAX_SHORT;
            if( frame_value_16 < MIN_SHORT ) frame_value_16 = MIN_SHORT;
            //printf("frame_value_float(%g),frame_value_16(%g)\n", frame_value_float, frame_value_16);

            if (idx < bufferLength) {
                buffer[idx++] = (short)frame_value_16;
                if (channels == 2) {
                    buffer[idx++] = (short)frame_value_16;
                }
            }
            //printf("buf(%d)\n", buffer[idx - 1]);

#if 1
            theta += theta_increment;
#else
            if ((frame < sample_num / 2)) {
                theta += theta_increment;
            } else {
                theta += theta_increment / 4;
            }
#endif
            if (theta > 2.0 * M_PI) {
                theta -= 2.0 * M_PI;
            }
        }
    }

    return;
}

int makeWave(int sampleRate, int sampleChannels, double duration, std::vector<unsigned int>& freqIndexes, std::vector<short>& pcm)
{
    int sample_num = (duration * sampleRate * sampleChannels);
    printf("sample num: %d\n", sample_num);

    int freqCount = freqIndexes.size();
    std::vector<unsigned int> freqValues(freqCount);
    for(int i = 0; i < freqCount; ++i) {
        num_to_freq(freqIndexes[i], &freqValues[i]);
    }

    int pcm_count = sample_num * freqCount;
    pcm.resize(pcm_count);

    makeChirp(pcm.data(), pcm.size(), freqValues.data(), freqValues.size(), duration, sampleRate, sampleChannels);

    return 0;
}

/// 获取指定时间的音频采样点数量
int getSampleCount(int sampleRate, int sampleChannels, double duration)
{
    return duration * sampleRate * sampleChannels;
}

int makeWave(int sampleRate, int sampleChannels, double duration, int freqIndexes[], int freqCount, short* out)
{
    int sample_num = (duration * sampleRate * sampleChannels);
    int pcm_count = sample_num * freqCount;
    //printf("sample_num(%d) pcm_count(%d), freqCount(%d)\n", sample_num, pcm_count, freqCount);

    std::vector<unsigned int> freqValues(freqCount);
    for (int i = 0; i < freqCount; ++i) {
        num_to_freq(freqIndexes[i], &freqValues[i]);
    }

    makeChirp(out, pcm_count, freqValues.data(), freqCount, duration, sampleRate, sampleChannels);

    return 0;
}

struct CBuilder::Impl
{
    int                 mFreqPos;
    int                 mRemainBitCount;
    uint64_t            mBlockBits;
    int                 m_sampleRate;
    int                 m_channel;
    double               m_duration;
    std::vector<int>    mFreqIndexes;
    
    void setSampleParams(int sampleRate, int channel, double duration)
    {
        m_sampleRate = sampleRate;
        m_channel = channel;
        m_duration = duration;

        return;
    }

    void getSampleParams(int& sampleRate, int& channel, double& duration)
    {
        sampleRate = m_sampleRate;
        channel = m_channel;
        duration = m_duration;

        return;
    }

    void initFreqValues(int blockCount)
    {
        mBlockBits = 0;
        mRemainBitCount = BITCOUNT_PER_BYTE * BLOCK_DATA_BYTES;
        mFreqIndexes.reserve(blockCount * BLOCK_FREQ_COUNT);
        mFreqIndexes.resize(0);
        mFreqPos = 0;
    }

    void pushToFreqValues(int block, uint8_t byte)
    {
        int blockBitCount = BITCOUNT_PER_BYTE * BLOCK_DATA_BYTES;

        if (mRemainBitCount >= BITCOUNT_PER_BYTE) {
            mBlockBits |= uint64_t(byte) << (blockBitCount - mRemainBitCount);
            mRemainBitCount -= BITCOUNT_PER_BYTE;
            // printf("data(%02x) mBlockBits(0x%016llx)\n", byte, mBlockBits);
        }

        if (mRemainBitCount <= 0) {
            uint64_t blockBits = mBlockBits;
            mBlockBits >>= blockBitCount;
            mRemainBitCount += blockBitCount;
            //printf("blockBits(0x%016llx)\n", blockBits);

            int freqCount = blockBitCount / BITCOUNT_PER_FREQ;
            std::string freqs;
            freqs.reserve(freqCount + 1);

            // block index to 1th freqency
            char ch = 0;
            num_to_char(block, &ch);
            freqs.push_back(ch);
            printf("block index(%d) freqs(%s)\n", block, freqs.c_str());

            // data to 8 freqency
            for (int i = 0; i < freqCount; ++i) {
                int freq = blockBits & MASK_OF_FREQ;
                blockBits >>= BITCOUNT_PER_FREQ;

                char ch = 0;
                num_to_char(freq, &ch);
                // printf("freq(%02x) base32(%02x)(%c)\n", freq, ch, ch);
                freqs.push_back(ch);
            }

            printf("freqs(%s)\n", freqs.c_str());

#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_INFO, "wavelink", "block index(%d) freqs(%s)\n", block, freqs.c_str());
#endif

            // add sync frequency
            mFreqIndexes.push_back(17);
            mFreqIndexes.push_back(19);
            // rscode the 9 freqency
            rsEncode(freqs.c_str(), freqs.size(), std::back_inserter(mFreqIndexes));

            // add tail
            mFreqIndexes.push_back(-1);
            mFreqIndexes.push_back(-1);
        }
    }
};

CBuilder::CBuilder()
{
    mImpl = new Impl();
}

CBuilder::~CBuilder()
{
    delete mImpl;
}

void CBuilder::SetSampleParams(int sampleRate, int channel, double duration)
{
    return mImpl->setSampleParams(sampleRate, channel, duration);
}

bool CBuilder::SetContent(void const* content, int bytes)
{
    int block_count = (bytes + HEADER_BYTES + BLOCK_DATA_BYTES - 1) / BLOCK_DATA_BYTES;

    mImpl->initFreqValues(block_count);

    for(int block = 0; block < block_count; ++block) {
        int loop_count = 1;
        if(block == 0) { // for head
            loop_count = 2;
        }

        for(int loop = 0; loop < loop_count; ++loop) {
            int offset = block * BLOCK_DATA_BYTES - HEADER_BYTES;
            for (int i = 0; i < BLOCK_DATA_BYTES; ++i) {
                int idx = offset + i;
                if (idx >= 0 && idx < bytes) {
                    uint8_t const* ptr = (uint8_t const*)content;
                    mImpl->pushToFreqValues(block, *(ptr + idx));
                } else if (idx == -1) {
                    mImpl->pushToFreqValues(block, bytes); // HEADER
                } else {
                    mImpl->pushToFreqValues(block, i*4 + 'a');     // TAIL etc.
                }
            }
        }
    }

    std::copy(mImpl->mFreqIndexes.begin(), mImpl->mFreqIndexes.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;

    return true;
}

/// read pcm until no data, return valid bytes
int CBuilder::ReadPcm(void* buf, int bytes)
{
    if (mImpl->mFreqPos >= (int)mImpl->mFreqIndexes.size()) {
        printf("no freqency data\n");
        return 0;
    }

    int sampleRate;
    int channel;
    double duration;

    mImpl->getSampleParams(sampleRate, channel, duration);

    int sample_num = getSampleCount(sampleRate, channel, duration);
    int wantCount = bytes / sizeof(short) / sample_num;
    if (wantCount <= 0) {
        printf("buf not enough!\n");
        return 0;
    }

    int remainCount = mImpl->mFreqIndexes.size() - mImpl->mFreqPos;
    int realCount = remainCount < wantCount ? remainCount : wantCount;

    makeWave(sampleRate, channel, duration, &mImpl->mFreqIndexes[mImpl->mFreqPos], realCount, (short*)buf);
    mImpl->mFreqPos += realCount;

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO, "wavelink", "sample_num(%d) realCount(%d)\n", sample_num, realCount);
#endif

    return sample_num * realCount * sizeof(short);
}

int CBuilder::GetDuration()
{
    int sampleRate;
    int channel;
    double dura;

    mImpl->getSampleParams(sampleRate, channel, dura);

    double duration = dura * mImpl->mFreqIndexes.size();

    return duration * 1000; // to ms
}

} // namespce Sonic

