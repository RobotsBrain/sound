#include <stdint.h>
#include <string>
#include <vector>
#include <deque>

#include "base/Mutex.h"
#include "base/LoopThread.h"
#include "base/Packet.h"

#include "config.h"
#include "FreqParser.h"
#include "CodeQueue.h"
#include "RSCodec.h"
#include "sonic/Listener.h"


namespace Sonic {

class CListenerImpl : public Base::CLoopThread
{
    CFreqParser mParser;
    CCodeQueue  mCodequeue;

    Base::CMutex              mFramesMutex;
    std::deque<Base::CPacket> mFrames;

    Base::CMutex          mBlocksMutex;
    std::vector<uint64_t> mBlocks;
    uint32_t              mBlockMask;

public:
    CListenerImpl() 
        : Base::CLoopThread("WaveTransmit")
        , mParser(SAMPLE_RATE, SAMPLE_CHANNEL, DURATION)
    {
        mBlockMask = 0xFFFFFFFF;
        mBlocks.resize(BLOCK_COUNT);
    }

    ~CListenerImpl()
    {
        Stop();
    }

    bool Start()
    {
        mBlockMask = 0xFFFFFFFF;
        return StartThread();
    }

    bool Stop()
    {
        StopThread();
        mFrames.clear();
        return true;
    }

    bool PutFrame(Base::CPacket const& frame)
    {
        Base::CMutex::ScopedLock lock(mFramesMutex);
        if (mFrames.size() < 32) {
            mFrames.push_back(frame);
            return true;
        }

        printf("frame queue full! queue count(%d)\n", mFrames.size());
        return false;
    }

    /// read pcm until no data, return valid bytes
    bool GetResult(Base::CBuffer& result)
    {
        Base::CMutex::ScopedLock lock(mBlocksMutex);

        if ((mBlockMask & 0x1) != 0) {
            //printf("no header!\n");
            return false;
        }

        int dataBytes = mBlocks[0] & MASK_OF_FREQ;
        int blockCount = (dataBytes + HEADER_BYTES + BLOCK_DATA_BYTES - 1) / BLOCK_DATA_BYTES;
        uint32_t blockMask = ~(0xFFFFFFFF << blockCount);
        //printf("dataBytes(%d) block count(%d) mask(0x%08x)\n", dataBytes, blockCount, blockMask);
        if (dataBytes == 0 || (mBlockMask & blockMask) != 0) {
            //printf("block not finished! dataBytes(%d) mask(0x%08x)\n", dataBytes, mBlockMask);
            return false;
        }

        result.Resize(dataBytes);
        char* dataptr = (char*)result.GetBuffer();
        for (int block = 0; block < blockCount; ++block) {
            uint64_t blockBits = mBlocks[block];
            int offset = block * BLOCK_DATA_BYTES - HEADER_BYTES;
            printf("block index(%02d) bits(0x%010llx)\n", block, blockBits);
            for (int i = 0; i < BLOCK_DATA_BYTES; ++i) {
                int idx = offset + i;
                if (idx >= 0 && idx < dataBytes) {
                    uint8_t data = (blockBits >> (BITCOUNT_PER_BYTE * i)) & 0xFF;
                    dataptr[idx] = data;
                    //printf("%02x ", data);
                }
            }
        }

        return true;
    }

private:
    bool PushToBlocks(std::string const& base32)
    {
        int blockBitCount = BITCOUNT_PER_BYTE * BLOCK_DATA_BYTES;
        int freqCount = blockBitCount / BITCOUNT_PER_FREQ;

        if ((int)base32.size() < freqCount + 1) {
            printf("base32 too short! size(%d)\n", base32.size());
            return false;
        }

        unsigned int blockIndex = 0;
        if (char_to_num(base32[0], &blockIndex) != 0) {
            printf("get block index failed!\n");
            return false;
        }

        uint64_t blockBits = 0;
        for (int i = 0; i < freqCount; ++i) {
            unsigned int num = 0;
            char_to_num(base32[i + 1], &num);
            blockBits |= uint64_t(num) << (i * BITCOUNT_PER_FREQ);
        }

        Base::CMutex::ScopedLock lock(mBlocksMutex);

        mBlocks[blockIndex] = blockBits;
        mBlockMask &= ~(1 << blockIndex);   // clear block index mask
        //printf("block index(%02d) bits(0x%010llx)\n", blockIndex, mBlocks[blockIndex]);

        return true;
    }

    void ThreadProc()
    {
        printf("Thread __begin\n");

        mParser.open();
        mCodequeue.clearQueue();

        bool exitflag = false;

        while (!exitflag) {
            int sig = WaitSignal(40);
            if (sig == -1) {
                exitflag = true;
                break;
            }

            ProcessFrames(exitflag);
            ProcessFreqValues(exitflag);
        }

        mParser.close();

        printf("Thread __end\n");
    }

    void ProcessFrames(bool& exitflag)
    {
        while (1) {
            //TRACEPOINT();
            Base::CPacket packet;
            {
                Base::CMutex::ScopedLock lock(mFramesMutex);
                if (!mFrames.empty())
                {
                    packet = mFrames.front();
                    mFrames.pop_front();
                }
            }

            if (packet.Empty()) {
                break;
            }

            short const* s16pcm = (short const*)packet.Data();
            int s16count = packet.DataSize() / sizeof(short);
            mParser.putAudioData(s16pcm, s16count);

            int sig = WaitSignal(0);
            if (sig == -1) {
                exitflag = true;
                break;
            }
        }
    }

    void ProcessFreqValues(bool& exitflag)
    {
        std::vector<double> freq_values;

        while (mParser.getResult(freq_values)) {
            //TRACEPOINT();
            mCodequeue.putFreqValues(freq_values);

            std::vector<int> res, rrr;
            while (mCodequeue.getResult(res, rrr)) {
                //TRACEPOINT();
                std::string base32;
                if (rsDecode(rrr, res, base32)) {
                    mCodequeue.clearQueue();
                    printf("base32 result: %s\n", base32.c_str());
                    PushToBlocks(base32);
                }
                //TRACEPOINT();
            }
            //TRACEPOINT();

            int sig = WaitSignal(0);
            if (sig == -1) {
                exitflag = true;
                break;
            }
        }
    }
};


struct CListener::Impl
{
    CListenerImpl impl;
};

CListener::CListener()
{
    mImpl = new Impl();
}

CListener::~CListener()
{
    delete mImpl;
}

bool CListener::Start()
{
    return mImpl->impl.Start();
}

bool CListener::Stop()
{
    return mImpl->impl.Stop();
}

bool CListener::PutFrame(Base::CPacket const& frame)
{
    return mImpl->impl.PutFrame(frame);
}

bool CListener::GetResult(Base::CBuffer& result)
{
    return mImpl->impl.GetResult(result);
}


} // namespce Sonic

