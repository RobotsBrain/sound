#ifndef __SONIC_LISTENER_H__
#define __SONIC_LISTENER_H__


namespace Sonic {

class CListener
{
public:
    CListener();
    ~CListener();

    bool Start(int sampleRate, int sampleChannels, double duration);
    bool Stop();

    bool PutFrame(char* pdata, int len);
    bool GetResult(char* pdata, int* plen);

private:
    CListener(CListener const&);
    CListener& operator=(CListener const&);

private:
    struct Impl;
    Impl* mImpl;
};

} // namespce Sonic


#endif // __SONIC_LISTENER_H__

