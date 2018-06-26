#ifndef __SONIC_LISTENER_H__
#define __SONIC_LISTENER_H__

#include "base/Buffer.h"


namespace Sonic {

class CListener
{
public:
    CListener();
    ~CListener();

    bool Start(int sampleRate, int sampleChannels, double duration);
    bool Stop();

    bool PutFrame(char* pdata, int len);
    bool GetResult(Base::CBuffer& result);

private:
    CListener(CListener const&);
    CListener& operator=(CListener const&);

private:
    struct Impl;
    Impl* mImpl;
};

} // namespce Sonic


#endif // __SONIC_LISTENER_H__

