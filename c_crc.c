#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "c_crc.h"


// Prototypes definitions
static void *CreateCrcTable(uint32_t polynomial);
static uint8_t FileExists(char* fileName);
static uint8_t GetFileNameFromUser(char* fileName);




int main(int argc, char **argv)
{
    char fileName[MAX_FILENAME_LENGTH];

    printf("*** C CRC Program ***\r\n");

    // for(int i =0; i < argc; i++)
    // {
    //     printf("Arg[%d] = '%s'\r\n", i, argv[i]);
    // }

    if(argc < 2)
    {
        // Ask user to enter a filename
        if(GetFileNameFromUser(fileName) != SUCCESS)
        {
            printf("No valid file provided by user\r\n");
            return 0;
        } 
    }
    else if(argc == 2)
    {
        if(FileExists(argv[1]) != SUCCESS)
        {
            printf("No valid file provided as argument\r\n");
            return 0;
        }        
    }
    else
    {
        printf("Too many arguments provided\r\n");
        return 0;
    }

    FILE *file;

    file = fopen(argv[1], "r");
    printf("File '%s' open\r\n", argv[1]);

    // Check is the file exists and could be open
    if(file == NULL)
    {
        printf("File could not be open\r\n");
        return 1;
    }

    uint32_t* crc32_table = CreateCrcTable(CRC32_POLINOMIAL);

    char c;

    while(1)
    {
        // Detect the end of file
        if(feof(file))
        {
            break;
        }

        c = fgetc(file);


        printf("%c", c);
    }

    fclose(file);

    return -1;
}



static uint8_t GetFileNameFromUser(char* fileName)
{
    // do
    // {
        printf("Enter filename: ");
        if(fgets(fileName, MAX_FILENAME_LENGTH, stdin) != NULL)
        {
            int length = strlen(fileName);
            if(length < 0)
            {
                return FAILURE;
            }

            // Remove newline feed captured by fgets()
            if((fileName[length - 1] == '\n') || (fileName[length - 1] == '\r'))
            {
                fileName[length - 1] == 0;
            }

            if(FileExists(fileName) == SUCCESS)
                return SUCCESS;
        }

    // } while (FileExists() != SUCCESS);
    
    return FAILURE;
}


static uint8_t FileExists(char* fileName)
{
    FILE *file;

    if(strlen(fileName) > MAX_FILENAME_LENGTH)
    {
        printf("File name too long.\r\n");
        return FAILURE;
    }

    file = fopen(fileName, "r");
    printf("Opening File '%s' \r\n", fileName);

    // Check is the file exists and could be open
    if(file == NULL)
    {
        printf("File could not be open\r\n");
        return FAILURE;
    }

    // Close the file and return SUCCESS
    fclose(file);

    return SUCCESS;
}





static void *CreateCrcTable(uint32_t polynomial)
{
    uint32_t *table = malloc(256 * CRC_TABLE_SIZE);

    if(table == NULL)
        return NULL;

    uint32_t remainder;
    uint8_t byte = 0;
    do {
        // Start with the data byte
        remainder = byte;
        for (uint32_t bit = 8; bit > 0; --bit)
        {
            if (remainder & 1)
                remainder = (remainder >> 1) ^ polynomial;
            else
                remainder = (remainder >> 1);
        }

        table[(size_t)byte] = remainder;
    } while(++byte != 0);
}