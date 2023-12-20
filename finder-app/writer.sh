#!/bin/bash
# Tester script for assignment 1 week 1 question 10)
# Author: Sebastien Lemetter


if [ $# -eq 2 ]
then
	WRITEFILE=$1
	WRITESTR=$2
    echo "Creating new file $WRITEFILE"
    if [ -f "$WRITEFILE" ]
    then
        echo "File exists, being overwriten"
        echo $WRITESTR > $WRITEFILE
    else
        echo "File does not exist, being created"
        install -D /dev/null $WRITEFILE
        echo $WRITESTR >> $WRITEFILE
    fi
else
    echo "Please enter 2 parameters as expected: WRITEFILE and SEARCHSTR"
    exit 1
fi