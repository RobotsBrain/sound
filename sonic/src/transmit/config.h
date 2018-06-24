#ifndef __WAVE_TRANSMIT_CONFIG_H__
#define __WAVE_TRANSMIT_CONFIG_H__


#define DURATION                0.03    // seconds
#define SAMPLE_RATE             16000   // Hz
#define SAMPLE_CHANNEL          1

enum {HEADER_BYTES = 1};
enum {BLOCK_DATA_BYTES = 5};
enum {BLOCK_FREQ_COUNT = 20};
enum {BITCOUNT_PER_BYTE = 8};
enum {BITCOUNT_PER_FREQ = 5};
enum {MASK_OF_FREQ = 0x1F};
enum {BLOCK_COUNT = 32};


#endif // __WAVE_TRANSMIT_CONFIG_H__

