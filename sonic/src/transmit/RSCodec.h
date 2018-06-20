#ifndef __AUDIO_TRANSMIT_RSCODEC_H__
#define __AUDIO_TRANSMIT_RSCODEC_H__

#include <string>
#include <vector>

#include "freq_util/bb_freq_util.h"
#include "rscode/rscode.h"


template <class OutIt>
inline int rsEncode(const char* src, int len, OutIt out)
{  
    unsigned char data[RS_TOTAL_LEN] = {0};
    int datalen = len <= RS_DATA_LEN - 1 ? len : RS_DATA_LEN - 1;
    unsigned int checksum = 0;

    for (int i=0; i < datalen; i++) {
        unsigned int n = 0;
        char_to_num(src[i], &n);
        data[i] = n;
        checksum ^= n;
    }

    // add checksum to tail of rsdata
    data[datalen] = (checksum & 0x1F);

    unsigned char* code = &data[RS_DATA_LEN];
    
    RS* rs = init_rs(RS_SYMSIZE, RS_GFPOLY, RS_FCR, RS_PRIM, RS_NROOTS, RS_PAD);
    encode_rs_char(rs, data, code);
    // free_rs_char(rs);  // can not use

    printf("rscode: ");
    for (int i = 0; i < RS_TOTAL_LEN; i++) {
        *out++ = data[i];
        printf("%02x ", (int)data[i]);
    }
    printf("\n");

    return datalen;
}

inline int rsEncode(const char* base32, std::vector<int>& result)
{
    int len = strlen(base32);

    result.resize(0);

    return rsEncode(base32, len, std::back_inserter(result));
}

bool rsDecode(std::vector<int> const& res, std::vector<int> const& rrr, std::string& base32);


#endif // __AUDIO_TRANSMIT_RSCODEC_H__

