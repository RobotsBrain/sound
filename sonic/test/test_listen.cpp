#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>

#include "base/Mutex.h"
#include "base/MemoryPool.h"
#include "base/SingletonPool.h"

#include "sonic/Listener.h"


struct AudioPacketPoolTag {};
typedef Base::TSingletonPool<AudioPacketPoolTag, 4096, Base::CMemoryPool, Base::CMutex, 1, 0> SingletonPool;

static void* PoolAlloc(size_t bytes, size_t& alloc_bytes)
{
    assert(bytes <= SingletonPool::requested_size);
    alloc_bytes = SingletonPool::requested_size;
    return SingletonPool::malloc();
}

static void PoolFree(void* ptr, size_t alloc_bytes)
{
    SingletonPool::free(ptr);
}

inline Base::CPacket create_frame(int framesize)
{
    return Base::CPacketFactory::Create(framesize, 0, PoolAlloc, PoolFree);
}


int test_listen(const char* filename, int sampleRate, int channel, double duration)
{
    FILE* fin = fopen(filename, "r");
    if (!fin) {
        return -1;
    }

    int frame_index = 0;
    Sonic::CListener listener;
    listener.Start(sampleRate, channel, duration);

    while (1) {
        if (!feof(fin)) {
            Base::CPacket s16_wave = create_frame(3200);
            s16_wave.SetDataRange(0, 3200);
            int read_bytes = fread(s16_wave.Data(), 1, s16_wave.DataSize(), fin);
            if (read_bytes <= 0) {
                printf("input file end!\n");
                break;
            } else {                
                s16_wave.SetDataRange(0, read_bytes);
                listener.PutFrame(s16_wave);
                printf("frame(%d) read_bytes(%d)\n", frame_index, read_bytes);
            }
        }

        Base::CBuffer result;
        if (listener.GetResult(result)) {
            printf("result success!\n");
            char* data = (char*)result.GetBuffer();
            int len = result.Size();
            std::string str;
            std::copy(data, data + len, std::back_inserter(str));
            printf("result(%s)\n", str.c_str());
            break;
        }

        frame_index++;

        usleep(2000);
    }

    listener.Stop();

    fclose(fin);

    return 0;
}

int main(int argc, char* argv[])
{
    int res = -1;
    int sampleRate = 16000;
    int channel = 1;
    double duration = 0.03;
    char pcm[256] = {0};

    while((res = getopt(argc, argv, "?p:s:d:c:h")) != -1) {
        switch(res) {
        case 'p':
            strcpy(pcm, optarg);
            break;

        case 's':
            sampleRate = atoi(optarg);
            break;

        case 'd':
            duration = atoi(optarg)/1000;
            break;

        case 'c':
            channel = atoi(optarg);
            break;

        case 'h':
        default:
            return -1;
        }
    }

    test_listen(pcm, sampleRate, channel, duration);

    return 0;
}

