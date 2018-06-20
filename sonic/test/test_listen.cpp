#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
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


int test_listen(const char* filename)
{
    FILE* fin = fopen(filename, "r");
    if (!fin) {
        return -1;
    }

    Sonic::CListener listener;
    listener.Start();

    Base::CPacket s16_wave = create_frame(3200);

    while (1) {
        if (!feof(fin)) {
            s16_wave.SetDataRange(0, 3200);
            int read_bytes = fread(s16_wave.Data(), 1, s16_wave.DataSize(), fin);
            if (read_bytes <= 0) {
                printf("input file end!\n");
                break;
            } else {                
                s16_wave.SetDataRange(0, read_bytes);
                listener.PutFrame(s16_wave);
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

        usleep(200);
    }

    listener.Stop();

    fclose(fin);

    return 0;
}

int main(int argc, char* argv[])
{
    printf("hello\n");

    if (argc < 2) {
        printf("usage: test_fft <pcm filename>\n");
        return -1;
    }

    test_listen(argv[1]);

    return 0;
}

