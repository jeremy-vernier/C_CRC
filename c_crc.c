#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "c_crc.h"


// Prototypes definitions
static enum Error_t FileExists(char* fileName);
static enum Error_t GetFileNameFromUser(char* fileName);
static enum Error_t CrcComputationProcess(char* fileName, uint32_t *crcValue);
static uint32_t ComputeCrc(char* fileName, struct Crc_t* crc);
static enum Error_t CreateCrcTable(struct Crc_t* crc);
static void DeleteCrcTable(void *table);





int main(int argc, char **argv)
{
    char fileName[MAX_FILENAME_LENGTH];
    char *pFile = NULL;

    printf("*** C CRC Program ***\r\n");

    if(argc < 2)
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
    uint32_t crcResult;
    if(CrcComputationProcess(pFile, &crcResult) != Success)
    {
        printf("Error computing the CRC value.\r\n");
        return 0;
    }

    printf("CRC = %d\r\n", crcResult);

    return -1;
}



static enum Error_t GetFileNameFromUser(char* fileName)
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


static enum Error_t FileExists(char* fileName)
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


static enum Error_t CrcComputationProcess(char* fileName, uint32_t *crcValue)
{
    if(fileName == NULL)
        return Failure;

    // Create CRC32 table
    struct Crc_t crc = {
        .crcBitSize = 32, 
        .polynomial = CRC32_POLINOMIAL,
        .initialValue = 0};

    if(CreateCrcTable(&crc) != Success)
    {
        return Failure;
    }

    // Calculate CRC value
    *crcValue = ComputeCrc(fileName, &crc);

    // Delete CRC table from memory
    DeleteCrcTable((void*)crc.crcTable);

    return Success;
}


static uint32_t ComputeCrc(char* fileName, struct Crc_t* crc)
{
    uint32_t crcValue = 0;
    uint32_t* crcTable = (uint32_t*)crc->crcTable;
    FILE *file;

    file = fopen(fileName, "r");
    printf("File '%s' open\r\n", fileName);

    // Check is the file exists and could be open
    if(file == NULL)
    {
        printf("File could not be open\r\n");
        return crcValue;
    }

    while(1)
    {
        // Detect the end of file
        if(feof(file))
        {
            break;
        }

        uint8_t byte = fgetc(file);

        // Calculate byte CRC
        /* XOR-in next input byte into MSB of crc and get this MSB, that's our new intermediate divident */
        uint8_t pos = (uint8_t)((crcValue ^ (byte << 24)) >> 24);
        /* Shift out the MSB used for division per lookuptable and XOR with the remainder */
        crcValue = (uint32_t)((crcValue << 8) ^ (uint32_t)(crcTable[pos]));
    }

    fclose(file);

    return crcValue;
}



static enum Error_t CreateCrcTable(struct Crc_t* crc)
{
    uint32_t polynomial = crc->polynomial;

    // Get space for table. Table has 256 entries of width "CRC bit size"
    crc->crcTable = malloc(256 * (crc->crcBitSize / 8));

    if(crc->crcTable == NULL)
        return Failure;
        
    #ifdef DISPLAY_DEBUG_OUTPUT
        printf("<CRC TABLE>>\r\n");
        printf("CRC Table Initialization with Polynomial=0x%X and initial value=0x%X\r\n", crc->polynomial, crc->initialValue);
    #endif // DISPLAY_DEBUG_OUTPUT

    // TODO: Make bitsize flexible insted of fixed 32bit
    uint32_t* table = crc->crcTable;

    // Go through all 256 Bytes of the table
    for (uint32_t divident = 0; divident < 256; divident++)
    {
        uint32_t remainder = (uint32_t)(divident << 24); /* move divident byte into MSB of 32Bit CRC */
        for (uint8_t bit = 0; bit < 8; bit++)
        {
            if ((remainder & 0x80000000) != 0)
            {
                remainder <<= 1;
                remainder ^= polynomial;
            }
            else
            {
                remainder <<= 1;
            }
        }

        table[divident] = remainder;
    
        #ifdef DISPLAY_DEBUG_OUTPUT
            printf(" CRC[%u] = %X\r\n", divident, remainder);
        #endif // DISPLAY_DEBUG_OUTPUT
    }

    // uint32_t remainder;
    // uint8_t byte = 0;
    // do {
    //     // Start with the data byte
    //     remainder = byte;
    //     for (uint32_t bit = 8; bit > 0; --bit)
    //     {
    //         if (remainder & 1)
    //             remainder = (remainder >> 1) ^ polynomial;
    //         else
    //             remainder = (remainder >> 1);
    //     }

    //     table[(size_t)byte] = remainder;

    //     #ifdef DISPLAY_DEBUG_OUTPUT
    //         printf(" CRC[%u] = %X\r\n", byte, remainder);
    //     #endif // DISPLAY_DEBUG_OUTPUT
    // } while(++byte != 0);

    #ifdef DISPLAY_DEBUG_OUTPUT
        printf("</CRC TABLE>>\r\n");
    #endif // DISPLAY_DEBUG_OUTPUT

    return Success;
}


static void DeleteCrcTable(void *table)
{
    free(table);
}