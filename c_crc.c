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
static Error_t GetCrcConfigFromUser(Crc_t** crc);
static Error_t ValidateCrcConfigInput(uint8_t value, Crc_t** crc);


int main(int argc, char **argv)
{
    char fileName[MAX_FILENAME_LENGTH];
    char *pFile = NULL;
    Crc_t *selectedCrc;

    // At least 1 argument provided, Argument 1 used for filename parameter
    if(argc >= 2)
    {
        pFile = argv[1];
    }
    else
    {
        // Ask user to select a file if no file was provided as argument
        pFile = fileName;
        GetFileNameFromUser(pFile);
    }

    // Validate filename
    if(FileExists(pFile) != Success)
    {
        printf("File name not valid\r\n");
            return 0;
    } 

    // At least 2 argument provided, Argument 2 used for CRC type parameter
    if(argc >= 3)
    {
        // Validate provided Crc config
        int crcArg = atoi(argv[2]);
        if(ValidateCrcConfigInput((uint8_t)crcArg, &selectedCrc) != Success)
        {
            printf("Specified Crc config not valid\r\n");
            return 0;
        }
    }
    else
    {
        // Ask user to enter a CRC configuration
        if(GetCrcConfigFromUser(&selectedCrc) != Success)
        {
            printf("No valid Crc config provided by user\r\n");
            return 0;
        }
    }


    // Copute CRC
    uint64_t crcResult;
    if(CrcComputationProcess(pFile, &crcResult, selectedCrc) != Success)
    {
        printf("Error computing the CRC value.\r\n");
        return 0;
    }

    printf("CRC = 0x%llX\r\n", crcResult);

    return -1;
}



static Error_t GetFileNameFromUser(char* fileName)
{
    printf("Enter filename: ");
    if(fgets(fileName, MAX_FILENAME_LENGTH, stdin) != NULL)
    {
        int length = strlen(fileName);
        if(length < 0)
        {
            printf("Filename length error\r\n");
            return Failure;
        }

        // Remove newline feed captured by fgets()
        if((fileName[length - 1] == '\n') || (fileName[length - 1] == '\r'))
        {
            fileName[length - 1] = 0;
        }

        // printf("name length: %d\r\n", length);

        // for(int charCnt = 0; charCnt < length; charCnt++)
        // {
        //     printf("[%d] = 0x%X\r\n", charCnt, fileName[charCnt]);
        // }

        // if(FileExists(fileName) == Success)
        return Success;
    }
    
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


static Error_t GetCrcConfigFromUser(Crc_t** crc)
{
    printf("Enter Crc size (8/16/32/64): ");
    uint8_t userSelection;
    if(scanf("%u", &userSelection) != 1)
    {
        printf("crc size input is not a valid number\r\n");
    }
        
    return ValidateCrcConfigInput(userSelection, crc);
}


static Error_t ValidateCrcConfigInput(uint8_t value, Crc_t** crc)
{
    switch(value)
    {
        case 8:
            *crc = &crc8;
            printf("Selected CRC8\r\n");
            break;

        case 16:
            *crc = &crc16;
            printf("Selected CRC16\r\n");
            break;

        case 32:
            *crc = &crc32;
            printf("Selected CRC32\r\n");
            break;

        case 64:
            *crc = &crc64;
            printf("Selected CRC64\r\n");
            break;

        default:
            printf("Incorrect crc size input\r\n");
            return Failure;
    }
        
    return Success;
}


static Error_t CrcComputationProcess(char* fileName, uint64_t *crcValue, Crc_t* crc)
{
    if(fileName == NULL)
        return Failure;

    if((crc->crcBitSize < 8) || (crc->crcBitSize > 64))
        return Failure;

    void* crcTable;

    #ifdef DISPLAY_DEBUG_OUTPUT
        printf("CRC computation process\r\n");
    #endif // DISPLAY_DEBUG_OUTPUT

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
        // Read one byte from the file
        uint8_t byte = fgetc(file);

        // Detect the end of file
        if(feof(file))
        {
            break;
        }

        #ifdef DISPLAY_DEBUG_OUTPUT
            printf("BYTE[%llu] = 0x%X\r\n", bytesComputed, byte);
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

    // Apply final XOR
    crcValue = crcValue ^ crc->finalXor;
    crcValue &= sizeMask;

    #ifdef DISPLAY_DEBUG_OUTPUT
        printf("CRC value=0x%llX computed over %llu bytes\r\n", crcValue, bytesComputed);
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