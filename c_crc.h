#ifndef _C_CRC_H_
#define _C_CRC_H_


// #define SUCCESS 0
// #define FAILURE 1

#define DISPLAY_DEBUG_OUTPUT

#define MAX_FILENAME_LENGTH 256

const uint32_t CRC32_POLINOMIAL = 0x04C11DB7;
const uint32_t CRC32_INITIAL_VALUE = 0;

static const int CRC_TABLE_SIZE = 256;

struct Crc_t {
    void* crcTable;
    uint32_t initialValue;
    uint32_t polynomial;
    uint8_t crcBitSize;
};

enum Error_t {
    Success = 0,
    Failure = 1
};

#endif // _C_CRC_H_