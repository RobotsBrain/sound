#ifndef __SONIC_BUILDER_H__
#define __SONIC_BUILDER_H__


namespace Sonic {


class CBuilder
{
    CBuilder(CBuilder const&);
    CBuilder& operator=(CBuilder const&);

public:
    CBuilder();
    ~CBuilder();

    void SetSampleParams(int sampleRate, int channel, float duration);
    bool SetContent(void const* content, int bytes);

    /// read pcm until no data, return valid bytes
    int ReadPcm(void* buf, int bytes);

    /// get audio all milliseconds
    int GetDuration();

private:
    struct Impl;
    Impl* mImpl;
};

} // namespce Sonic


#endif // __SONIC_BUILDER_H__

