#ifndef __WAVEBUILDER_H__
#define __WAVEBUILDER_H__

int getSampleCount(int sampleRate, int sampleChannels, double duration);

int makeWave(int sampleRate, int sampleChannels, double duration,
				int freqValues[], int freqCount, short* out);


#endif
