#ifndef __AUDIO_TRANSMIT_FREQ_PARSER_H__
#define __AUDIO_TRANSMIT_FREQ_PARSER_H__


class CFreqParser
{
    CFreqParser(CFreqParser const&);
    CFreqParser& operator=(CFreqParser const&);

public:
    CFreqParser(int sampleRate, int sampleChannels, double duration);
    ~CFreqParser();

    bool open();
    bool close();

    bool putAudioData(const short* pcmData, int samples);
    bool getResult(std::vector<double>& freqValues); // get frequency db, include 17, 19 and rscode

private:
    struct Impl;
    Impl* mImpl;
};


#endif // __AUDIO_TRANSMIT_FREQ_PARSER_H__

