#include <stdint.h>
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
#include "WaveBuilder.h"
#include "sonic/Builder.h"


namespace Sonic {


enum {HEADER_BYTES = 1};
enum {BLOCK_DATA_BYTES = 5};
enum {BLOCK_FREQ_COUNT = 20};
enum {BITCOUNT_PER_BYTE = 8};
enum {BITCOUNT_PER_FREQ = 5};
enum {MASK_OF_FREQ = 0x1F};


struct CBuilder::Impl
{
    uint64_t mBlockBits;
    int mRemainBitCount;
    std::vector<int> mFreqIndexes;
    int mFreqPos;

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

    int sample_num = getSampleCount(SAMPLE_RATE, SAMPLE_CHANNEL, DURATION);
    int wantCount = bytes / sizeof(short) / sample_num;
    if (wantCount <= 0) {
        printf("buf not enough!\n");
        return 0;
    }

    int remainCount = mImpl->mFreqIndexes.size() - mImpl->mFreqPos;
    int realCount = remainCount < wantCount ? remainCount : wantCount;

    makeWave(SAMPLE_RATE, SAMPLE_CHANNEL, DURATION, &mImpl->mFreqIndexes[mImpl->mFreqPos], realCount, (short*)buf);
    mImpl->mFreqPos += realCount;

#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_INFO, "wavelink", "sample_num(%d) realCount(%d)\n", sample_num, realCount);
#endif

    return sample_num * realCount * sizeof(short);
}

int CBuilder::GetDuration()
{
    double duration = DURATION * mImpl->mFreqIndexes.size();

    return duration * 1000; // to ms
}

} // namespce Sonic

