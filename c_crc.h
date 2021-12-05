#ifndef _C_CRC_H_
#define _C_CRC_H_


// #define SUCCESS 0
// #define FAILURE 1

#define DISPLAY_DEBUG_OUTPUT

#define MAX_FILENAME_LENGTH 256

const uint32_t CRC32_POLINOMIAL = 0x04C11DB7;
const uint32_t CRC32_INITIAL_VALUE = 0xFFFFFFFF;

static const int CRC_TABLE_SIZE = 256;

typedef struct {
    uint8_t crcBitSize;     // 8, 16, 32 or 64bits
    uint64_t initialValue;
    uint64_t polynomial;
    uint64_t finalXor;      // XOR applied to the computed CRC before returning its value
    
}Crc_t;

typedef enum {
    Success = 0,
    Failure = 1
}Error_t;


// CRC8 table
Crc_t crc8 = {
    .crcBitSize = 8, 
    .polynomial = 0x07,
    .initialValue = 0x00,
    .finalXor = 0x00};


// CRC32 table
Crc_t crc16 = {
    .crcBitSize = 16, 
    .polynomial = 0x1021,
    .initialValue = 0x0000,
    .finalXor = 0x0000};


// CRC32 table
Crc_t crc32 = {
    .crcBitSize = 32, 
    .polynomial = 0x04C11DB7,
    .initialValue = 0xFFFFFFFF,
    .finalXor = 0xFFFFFFFF};


// CRC64 table
Crc_t crc64 = {
    .crcBitSize = 64, 
    .polynomial = 0x42F0E1EBA9EA3693,
    .initialValue = 0x0000000000000000,
    .finalXor = 0x0000000000000000};

static Crc_t* selectedCrc = &crc32;


#endif // _C_CRC_H_