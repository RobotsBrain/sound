#ifndef __AUDIO_TRANSMIT_FREQ_PARSER_H__
#define __AUDIO_TRANSMIT_FREQ_PARSER_H__


class CFreqParser
{
    CFreqParser(CFreqParser const&);
    CFreqParser& operator=(CFreqParser const&);

public:
    CFreqParser();
    ~CFreqParser();

    void SetSampleParams(int sampleRate, int sampleChannels, double duration);

    bool open();
    bool close();

    bool putAudioData(const short* pcmData, int samples);
    bool getResult(std::vector<double>& freqValues);

private:
    struct Impl;
    Impl* mImpl;
};


#endif // __AUDIO_TRANSMIT_FREQ_PARSER_H__

