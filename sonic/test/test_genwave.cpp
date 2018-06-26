#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <libgen.h>
#include <vector>

#include "sonic/Builder.h"



int gen_wave(const char* txtfile, const char* wavfile,
            int sampleRate, int channel, double duration)
{
    FILE* fin = fopen(txtfile, "r");
    if (!fin) {
        return -1;
    }

    FILE* fout = fopen(wavfile, "w");
    if (!fout) {
        fclose(fin);
        return -1;
    }

    char* line = NULL;
    size_t len = 0;
    int linebytes = getline(&line, &len, fin);

    printf("linebytes(%d) len(%d) (%s)\n", linebytes, len, line);

    Sonic::CBuilder builder;
    builder.SetSampleParams(sampleRate, channel, duration);
    builder.SetContent(line, linebytes);
    free(line);

    std::vector<short> pcmdata(3200);
    int frameIndex = 0;

    while(1) {
        int readBytes = builder.ReadPcm(pcmdata.data(), pcmdata.size() * sizeof(pcmdata[0]));
        if(readBytes <= 0) {
            break;
        }

        int write_bytes = fwrite(pcmdata.data(), 1, readBytes, fout);
        // printf("frame(%d) readBytes(%d) write_bytes(%d)\n", frameIndex, readBytes, write_bytes);

        ++frameIndex;
    }

    fclose(fin);
    fclose(fout);

    return 0;
}

int main(int argc, char* argv[])
{
    int res = -1;
    int sampleRate = 16000;
    int channel = 1;
    double duration = 0.03;
    char text[256] = {0};
    char pcm[256] = {0};

    while((res = getopt(argc, argv, "?t:p:s:d:c:h")) != -1) {
        switch(res) {
        case 't':
            strcpy(text, optarg);
            break;

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

    gen_wave(text, pcm, sampleRate, channel, duration);

    return 0;
}


