/* Copyright (c) 2023 Sebastien Lemetter
 * writer.c: Create a file containing a given string
 * ========================================== */
#include <stdio.h>
#include <errno.h>

int main(int argc, char** argv)
{
    // Verify arguments
    if(argc != 3)
    {
        printf("Error, wrong number of input parameters.\n");
        printf("writer should be started with 2 input parameters, first file full path and then text string\n");
        return 1;
    }

    // Create file and confirm that no problem occured
    // Use w mode to overwrite file if already exists
    FILE *fp = NULL;
    fp = fopen(argv[1] ,"w");
    if(fp == NULL)
    {
        //printf(stderr, "Value of errno attempting to open file %s: %d\n", argv[1], errno);
        perror("perror returned");
        return 1;
    }

    // Write data and close file
    fprintf( fp, argv[2]);
    fclose(fp);

    return 0;
}

