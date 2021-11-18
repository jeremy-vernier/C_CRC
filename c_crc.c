#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "c_crc.h"


// Prototypes definitions
static Error_t FileExists(char* fileName);
static Error_t GetFileNameFromUser(char* fileName);
static Error_t CrcComputationProcess(char* fileName, uint64_t *crcValue, Crc_t* crc);
static uint64_t ComputeCrc(char* fileName, void* crcTable, Crc_t* crc);
static Error_t CreateCrcTable(Crc_t* crc, void** crcTable);
static void DeleteCrcTable(void *table);



int main(int argc, char **argv)
{
    char fileName[MAX_FILENAME_LENGTH];
    char *pFile = NULL;

    printf("*** C CRC Program ***\r\n");

    if(argc < 2) // TODO support 3rd argument for CRC selection
    {
        // Ask user to enter a filename
        if(GetFileNameFromUser(fileName) != Success)
        {
            printf("No valid file provided by user\r\n");
            return 0;
        } 

        pFile = fileName;
    }
    else if(argc == 2)
    {
        if(FileExists(argv[1]) != Success)
        {
            printf("No valid file provided as argument\r\n");
            return 0;
        } 

        pFile = argv[1];
    }
    else
    {
        printf("Too many arguments provided\r\n");
        return 0;
    }

    // Copute CRC
    uint64_t crcResult;
    if(CrcComputationProcess(pFile, &crcResult, selectedCrc) != Success)
    {
        printf("Error computing the CRC value.\r\n");
        return 0;
    }

    printf("CRC = %llX\r\n", crcResult);

    return -1;
}



static Error_t GetFileNameFromUser(char* fileName)
{
    // do
    // {
        printf("Enter filename: ");
        if(fgets(fileName, MAX_FILENAME_LENGTH, stdin) != NULL)
        {
            int length = strlen(fileName);
            if(length < 0)
            {
                return Failure;
            }

            // Remove newline feed captured by fgets()
            if((fileName[length - 1] == '\n') || (fileName[length - 1] == '\r'))
            {
                fileName[length - 1] == 0;
            }

            if(FileExists(fileName) == Success)
                return Success;
        }

    // } while (FileExists() != SUCCESS);
    
    return Failure;
}


static Error_t FileExists(char* fileName)
{
    FILE *file;

    if(strlen(fileName) > MAX_FILENAME_LENGTH)
    {
        printf("File name too long.\r\n");
        return Failure;
    }

    file = fopen(fileName, "r");
    printf("Opening File '%s' \r\n", fileName);

    // Check is the file exists and could be open
    if(file == NULL)
    {
        printf("File could not be open\r\n");
        return Failure;
    }

    // Close the file and return SUCCESS
    fclose(file);

    return Success;
}


static Error_t CrcComputationProcess(char* fileName, uint64_t *crcValue, Crc_t* crc)
{
    if(fileName == NULL)
        return Failure;

    if((crc->crcBitSize < 8) || (crc->crcBitSize > 64))
        return Failure;


    void* crcTable;

    if(CreateCrcTable(crc, &crcTable) != Success)
    {
        return Failure;
    }

    // Calculate CRC value
    *crcValue = ComputeCrc(fileName, crcTable, crc);

    // Delete CRC table from memory
    DeleteCrcTable((void*)crcTable);

    return Success;
}


static uint64_t ComputeCrc(char* fileName, void* crcTable, Crc_t* crc)
{
    uint64_t crcValue = 0;
    uint64_t* table = (uint64_t*)crcTable;
    FILE *file;

    file = fopen(fileName, "r");
    printf("File '%s' open\r\n", fileName);

    // Check is the file exists and could be open
    if(file == NULL)
    {
        printf("File could not be open\r\n");
        return crcValue;
    }


    uint64_t sizeMask = 0;
    for(uint8_t bitCount = 0; bitCount < crc->crcBitSize; bitCount++)
    {
        sizeMask = sizeMask << 1;
        sizeMask |= 1;
    }

    uint8_t shift = crc->crcBitSize - 8;
    uint64_t bytesComputed = 0;

    while(1)
    {
        // Detect the end of file
        if(feof(file))
        {
            break;
        }

        uint8_t byte = fgetc(file);

        #ifdef DISPLAY_DEBUG_OUTPUT
            printf("BYTE[%llX] = %u\r\n", bytesComputed, byte);
        #endif // DISPLAY_DEBUG_OUTPUT

        // Calculate byte CRC
        /* XOR-in next input byte into MSB of crc and get this MSB, that's our new intermediate divident */
        uint8_t position = (uint8_t)((crcValue ^ (byte << shift)) >> shift);
        /* Shift out the MSB used for division per lookuptable and XOR with the remainder */
        crcValue = (uint64_t)((crcValue << 8) ^ (uint64_t)(table[position]));
        crcValue &= sizeMask;

        bytesComputed++;
    }

    fclose(file);

    #ifdef DISPLAY_DEBUG_OUTPUT
        printf("CRC value=%llX computed over %llu bytes\r\n", crcValue, bytesComputed);
    #endif // DISPLAY_DEBUG_OUTPUT

    return crcValue;
}



static Error_t CreateCrcTable(Crc_t* crc, void** crcTable)
{
    uint64_t polynomial = crc->polynomial;

    // Get space for table. Table has 256 entries of width "CRC bit size"
    *crcTable = malloc(256 * sizeof(uint64_t));//(crc->crcBitSize / 8));

    // Abort if malloc fails to allocate memory
    if(*crcTable == NULL)
        return Failure;
        
    // TODO: Make bitsize flexible insted of fixed 32bit
    uint64_t* table = (uint64_t*)*crcTable;

    uint64_t sizeMask = 0;
    for(uint8_t bitCount = 0; bitCount < crc->crcBitSize; bitCount++)
    {
        sizeMask = sizeMask << 1;
        sizeMask |= 1;
    }

    #ifdef DISPLAY_DEBUG_OUTPUT
        printf("<CRC TABLE>\r\n");
        printf("CRC Table Initialization with Polynomial=0x%llX and initial value=0x%llX\r\n", crc->polynomial, crc->initialValue);
        printf("CRC size=0x%X and mask=0x%llX\r\n", crc->crcBitSize, sizeMask);
    #endif // DISPLAY_DEBUG_OUTPUT

    // Go through all 256 Bytes of the table
    for (uint64_t divident = 0; divident < 256; divident++)
    {
        uint64_t remainder = (uint64_t)(divident << (crc->crcBitSize - 8)); /* move divident byte into MSB of CRC */
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            // Check if the MSB is non-zero
            if ((remainder & (1U << (crc->crcBitSize - 1))) != 0)
            {
                remainder <<= 1;
                remainder ^= polynomial;
            }
            else
            {
                remainder <<= 1;
            }
        }

        table[divident] = remainder & sizeMask;
    
        #ifdef DISPLAY_DEBUG_OUTPUT
            printf(" CRC[%u] = %llX\r\n", divident, table[divident]);
        #endif // DISPLAY_DEBUG_OUTPUT
    }

    #ifdef DISPLAY_DEBUG_OUTPUT
        printf("</CRC TABLE>>\r\n");
    #endif // DISPLAY_DEBUG_OUTPUT

    return Success;
}


static void DeleteCrcTable(void *table)
{
    free(table);
}