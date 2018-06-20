#ifndef __SONIC_LISTENER_H__
#define __SONIC_LISTENER_H__


#include "base/Packet.h"
#include "base/Buffer.h"


namespace Sonic {


class CListener
{
    CListener(CListener const&);
    CListener& operator=(CListener const&);

public:
    CListener();

    ~CListener();

    bool Start();

    bool Stop();

    bool PutFrame(Base::CPacket const& frame);

    bool GetResult(Base::CBuffer& result);

private:
    struct Impl;
    Impl* mImpl;
};

} // namespce Sonic


#endif // __SONIC_LISTENER_H__

