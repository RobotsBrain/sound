#include <stdio.h>
#include <vector>

#include "freq_util/bb_freq_util.h"


#define MAX_VOLUME              0.5
#define MAX_SHORT               32767
#define MIN_SHORT               -32768


void makeChirp(short buffer[],int bufferLength,unsigned int freqArray[], int freqArrayLength,
                double duration_secs, long sample_rate, int channels) 
{
    int sample_num = duration_secs * sample_rate;
    //printf("sample_num: %d\n", sample_num);

    double theta = 0;
    int idx = 0;
    for (int i=0; i<freqArrayLength; i++) 
    {
        double theta_increment = 2.0 * M_PI * freqArray[i] / sample_rate;
        //printf("theta_increment:%g\n", theta_increment);
        
        // Generate the samples
        for (int frame = 0; frame < sample_num; frame++)
        {
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
            
            if (idx < bufferLength)
            {
                buffer[idx++] = (short)frame_value_16;
                if (channels == 2)
                {
                    buffer[idx++] = (short)frame_value_16;
                }
            }
            //printf("buf(%d)\n", buffer[idx - 1]);

#if 1
            theta += theta_increment;
#else
            if ((frame < sample_num / 2))
            {
                theta += theta_increment;
            }
            else
            {
                theta += theta_increment / 4;
            }
#endif
            if (theta > 2.0 * M_PI)
            {
                theta -= 2.0 * M_PI;
            }
        }
    }
}

int makeWave(int sampleRate, int sampleChannels, double duration, std::vector<unsigned int>& freqIndexes, std::vector<short>& pcm)
{
    int sample_num = (duration * sampleRate * sampleChannels);
    printf("sample num: %d\n", sample_num);

    int freqCount = freqIndexes.size();
    std::vector<unsigned int> freqValues(freqCount);
    for (int i = 0; i < freqCount; ++i)
    {
        num_to_freq(freqIndexes[i], &freqValues[i]);
        //printf("v:%d\n", freqValues[i]);
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
    for (int i = 0; i < freqCount; ++i)
    {
        num_to_freq(freqIndexes[i], &freqValues[i]);
        //printf("v:%d\n", freqValues[i]);
    }

    makeChirp(out, pcm_count, freqValues.data(), freqCount, duration, sampleRate, sampleChannels);
    return 0;
}

