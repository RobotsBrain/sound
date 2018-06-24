#ifndef __AUDIO_TRANSMIT_CODE_QUEUE_H__
#define __AUDIO_TRANSMIT_CODE_QUEUE_H__


#include <vector>


class CCodeQueue
{
    CCodeQueue(CCodeQueue const&);
    CCodeQueue& operator=(CCodeQueue const&);

public:
    CCodeQueue();
    ~CCodeQueue();

    bool putFreqValues(std::vector<double> const& freqVaules);
    bool getResult(std::vector<int>& res, std::vector<int>& rrr);
    bool clearQueue();

private:
    struct Impl;
    Impl* mImpl;
};


#endif // __AUDIO_TRANSMIT_CODE_QUEUE_H__

