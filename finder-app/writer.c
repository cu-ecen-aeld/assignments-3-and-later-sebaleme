/* Copyright (c) 2023 Sebastien Lemetter
 * writer.c: Create a file containing a given string
 * ========================================== */
#include <errno.h>
#include <stdio.h>
#include <syslog.h>

int main(int argc, char** argv)
{
    // Use the syslog for non interactive application
    openlog(NULL,0,LOG_USER);

    // Verify arguments
    if(argc != 3)
    {
        syslog(LOG_ERR, "Invalid number of arguments: %d", argc);
        syslog(LOG_INFO, "writer should be started with 2 input parameters, first file full path and then text string\n");
        return 1;
    }

    // Create file and confirm that no problem occured
    // Use w mode to overwrite file if already exists
    FILE *fp = NULL;
    fp = fopen(argv[1] ,"w");
    if(fp == NULL)
    {
        syslog(LOG_ERR, "Value of errno attempting to open file %s: %d\n", argv[1], errno);
        return 1;
    }

    // Write data and close file
    fprintf( fp, argv[2]);
    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
    fclose(fp);

    return 0;
}

