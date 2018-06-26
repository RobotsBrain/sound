#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>

#include "sonic/Listener.h"


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
            char buffer[3200] = {0};
            int read_bytes = fread(buffer, 1, sizeof(buffer), fin);
            if (read_bytes <= 0) {
                printf("input file end!\n");
                break;
            } else {
                listener.PutFrame(buffer, read_bytes);
                printf("frame(%d) read_bytes(%d)\n", frame_index, read_bytes);
            }
        }

        char result[32] = {0};
        int len = 0;

        if (listener.GetResult(result, &len)) {
            std::string str;
            std::copy(result, result + len, std::back_inserter(str));
            printf("get data success, result(%s)\n", str.c_str());
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

