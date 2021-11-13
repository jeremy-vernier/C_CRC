#ifndef _C_CRC_H_
#define _C_CRC_H_


#define SUCCESS 0
#define FAILURE 1

#define MAX_FILENAME_LENGTH 256

const uint32_t CRC32_POLINOMIAL = 0x04C11DB7;
const uint32_t CRC32_INITIAL = 0;

static const int CRC_TABLE_SIZE = 256;


#endif // _C_CRC_H_