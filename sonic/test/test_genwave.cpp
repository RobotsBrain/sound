#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "transmit/RSCodec.h"
#include "sonic/Builder.h"



int gen_wave(const char* txtfile, const char* wavfile)
{
    FILE* fin = fopen(txtfile, "r");
    if (!fin) {
        return -1;
    }

    FILE* fout = fopen(wavfile, "w");
    if (!fout) {
        fclose(fin);
        return -1;
    }

    char* line = NULL;
    size_t len = 0;
    int linebytes = getline(&line, &len, fin);

    printf("linebytes(%d) len(%d) (%s)\n", linebytes, len, line);

    Sonic::CBuilder builder;
    builder.SetContent(line, linebytes);
    free(line);

    std::vector<short> pcmdata(3200);
    int frameIndex = 0;

    while(1) {
        int readBytes = builder.ReadPcm(pcmdata.data(), pcmdata.size() * sizeof(pcmdata[0]));
        if(readBytes <= 0) {
            break;
        }

        int write_bytes = fwrite(pcmdata.data(), 1, readBytes, fout);
        printf("frame(%d) readBytes(%d) write_bytes(%d)\n", frameIndex, readBytes, write_bytes);

        ++frameIndex;
    }

    fclose(fin);
    fclose(fout);

    return 0;
}

int main(int argc, char* argv[])
{
    printf("hello\n");

    if (argc < 3) {
        printf("usage: test_genwave <txtfile> <wavfile>\n");
        return -1;
    }

    gen_wave(argv[1], argv[2]);

    return 0;
}


