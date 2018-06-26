#include <iterator>

#include "RSCodec.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif



int getRSCodeLen(int srcLen)
{
    return (srcLen + RS_DATA_LEN - 1) / RS_DATA_LEN * RS_TOTAL_LEN;
}

bool rsDecode(std::vector<int> const& res, std::vector<int> const& rrr, std::string& base32)
{
    int temp[RS_TOTAL_LEN];
    int result[RS_TOTAL_LEN][RS_TOTAL_LEN];
    int counter = 0;

    for (int i=0; i<RS_TOTAL_LEN; i++) {
        for (int j=0; j<RS_TOTAL_LEN; j++) {
            result[i][j] = -1;
        }
    }

    for (int k = 0; k < RS_TOTAL_LEN; k++) {
        for (int i=0, j=0; i<RS_TOTAL_LEN; i++, j++) {

            if (i <= k) {
                temp[j] = res[i];
            } else {
                temp[j] = rrr[i];
            }
        }

        RS *rs = init_rs(RS_SYMSIZE, RS_GFPOLY, RS_FCR, RS_PRIM, RS_NROOTS, RS_PAD);
        int eras_pos[RS_TOTAL_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        unsigned char data1[RS_TOTAL_LEN];

        for (int i=0; i<RS_TOTAL_LEN; i++) {
            data1[i] = temp[i];
        }

        int count = decode_rs_char(rs, data1, eras_pos, 0);

        /////////////////
        if (count >= 0) {
            for (int m = 0; m<RS_TOTAL_LEN; m++) {
                result[m][counter] = data1[m];
            }

            counter++;
        }

#ifdef SONIC_DEBUG
        for (int i=0; i<RS_TOTAL_LEN; i++) {
            printf("%02d ", data1[i]);
        }
        printf("    %d\n", count);
#endif
    }

    int temp_vote[RS_TOTAL_LEN] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int final_result[RS_TOTAL_LEN] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    for (int i=0; i<RS_TOTAL_LEN; i++) {
        for (int j=0; j<RS_TOTAL_LEN; j++) {
            temp_vote[j] = result[i][j];
        }

        vote(temp_vote, RS_TOTAL_LEN, &final_result[i]);
    }

    int ret = false;
    if (counter == 0) {
#ifdef __ANDROID__
        __android_log_print(ANDROID_LOG_WARN, "JNIMsg", "Failed");
#endif
    } else {
        //printf("successes!\n");
        base32.resize(0);
        unsigned int checksum = 0;

        for (int i = 0; i < RS_TOTAL_LEN; i++) {
            printf("%02x ", final_result[i]);
            //__android_log_print(ANDROID_LOG_INFO, "JNIMsg", "%02d", final_result[i]);

            if (i < RS_DATA_LEN) {
                checksum ^= final_result[i];
                if (i < RS_DATA_LEN - 1) {
                    char res_char = ' ';
                    num_to_char(final_result[i], &res_char);
                    base32.push_back(res_char);
                }
            }
        }

        printf("\n");

        // verify checksum
        if ((checksum & 0x1F) == 0) {
#ifdef __ANDROID__
            __android_log_print(ANDROID_LOG_INFO, "JNIMsg", "Successes: %s", base32.c_str());
#else
            printf("success: %s, checksum(%x)\n", base32.c_str(), checksum);
#endif
            ret = true;
        } else {
            printf("checksum failed! checksum(%02x)\n", checksum);
        }
    }
    
    return ret;
}

