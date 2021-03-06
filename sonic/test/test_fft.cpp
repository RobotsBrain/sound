#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <libgen.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>

#include "transmit/RSCodec.h"
#include "transmit/FreqParser.h"
#include "transmit/CodeQueue.h"
#include "transmit/config.h"



template <class InIt, class OutIt>
void buf_short_to_float(InIt first, InIt last, OutIt out)
{
    for (InIt it = first; it != last; ++it)
    {
        double v = (double)(*it) / 32768.0;     // normalize
        if (v > 1) v = 1;
        if (v < -1) v = -1;

        *out++ = (float)v;
    }
}

template <class InIt, class OutIt>
void buf_int32_to_short(InIt first, InIt last, OutIt out)
{
    for (InIt it = first; it != last; ++it)
    {
        int32_t v = (*it) / 65536;
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;

        *out++ = (short)v;
    }
}

template <class InIt, class OutIt>
void buf_norm_to_u8(InIt first, InIt last, OutIt out)
{
    for (InIt it = first; it != last; ++it)
    {
        double v = (*it) * 256;
        if (v > 255) v = 255;
        if (v < 0) v = 0;

        *out++ = (uint8_t)v;
    }
}

template <class InIt, class OutIt>
void buf_norm_to_s16(InIt first, InIt last, OutIt out)
{
    for (InIt it = first; it != last; ++it)
    {
        double v = (*it) * 32768;
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;

        *out++ = v;
    }
}

int fft_pcm16(const char* filename, const char* outfile,
                int sampleRate, int channel, double duration)
{
    int sample_num = (duration * sampleRate * channel);
    printf("sample num: %d\n", sample_num);

    CFreqParser parser;
    CCodeQueue codequeue;
    std::vector<short> s16_wave(1600);
    std::vector<double> freq_values(32);
    std::vector<uint8_t> freq_val_u8(32);
    std::vector<short> freq_val_s16(32);

    parser.SetSampleParams(sampleRate, channel, duration);

    int frame_index = 0;
    FILE* fin = fopen(filename, "r");
    if (!fin) {
        return -1;
    }

    FILE* fout = fopen(outfile, "w");
    if (!fout) {
        fclose(fin);
        return -1;
    }

    while (!feof(fin)) {
        int read_bytes = fread(s16_wave.data(), 1, sizeof(short) * s16_wave.size(), fin);
        if (read_bytes <= 0) {
            break;
        }

        parser.putAudioData(s16_wave.data(), s16_wave.size());

        while(parser.getResult(freq_values)) {
            codequeue.putFreqValues(freq_values);

            //buf_norm_to_u8(freq_values.begin(), freq_values.end(), freq_val_u8.begin());
            //int write_bytes = fwrite(freq_val_u8.data(), 1, sizeof(freq_val_u8[0]) * freq_val_u8.size(), fout);
            buf_norm_to_s16(freq_values.begin(), freq_values.end(), freq_val_s16.begin());
            int write_bytes = fwrite(freq_val_s16.data(), 1, sizeof(freq_val_s16[0]) * freq_val_s16.size(), fout);

            printf("frame(%d) read_bytes(%d) write_bytes(%d)\n", frame_index, read_bytes, write_bytes);

            std::vector<int> res, rrr;

            while(codequeue.getResult(res, rrr)) {
                printf("res size(%d) content: ", res.size());
                std::copy(res.begin(), res.end(), std::ostream_iterator<int>(std::cout, " "));
                printf("\n");
                printf("rrr size(%d) content: ", rrr.size());
                std::copy(rrr.begin(), rrr.end(), std::ostream_iterator<int>(std::cout, " "));
                printf("\n");

                std::string base32;
                if(rsDecode(rrr, res, base32) == true) {
                    printf("base32: %s\n", base32.c_str());   
                }
            }
        }

        ++frame_index;
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
    char pcm[256] = {0};
    char fft[256] = {0};

    while((res = getopt(argc, argv, "?f:p:s:d:c:h")) != -1) {
        switch(res) {
        case 'f':
            strcpy(fft, optarg);
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

    fft_pcm16(pcm, fft, sampleRate, channel, duration);

    return 0;
}

