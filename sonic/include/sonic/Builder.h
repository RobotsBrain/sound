#ifndef __SONIC_BUILDER_H__
#define __SONIC_BUILDER_H__


namespace Sonic {

class CBuilder
{
public:
    CBuilder();
    ~CBuilder();

    void SetSampleParams(int sampleRate, int channel, double duration);
    bool SetContent(void const* content, int bytes);

    int ReadPcm(void* buf, int bytes);
    int GetDuration();

private:
    CBuilder(CBuilder const&);
    CBuilder& operator=(CBuilder const&);

private:
    struct Impl;
    Impl* mImpl;
};

} // namespce Sonic


#endif // __SONIC_BUILDER_H__

