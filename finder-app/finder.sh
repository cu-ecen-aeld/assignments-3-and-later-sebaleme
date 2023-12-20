#!/bin/bash
# Tester script for assignment 1 week 1 question 9)
# Author: Sebastien Lemetter


if [ $# -eq 2 ]
then
	FILEDIR=$1
	SEARCHSTR=$2
    echo "Searching for $SEARCHSTR in $FILEDIR"
    if [ -d "$FILEDIR" ]
    then
        NUMFILES=$(find $FILEDIR -type f | wc -l)
        MATCHINGLINES=$(grep -Rnw $FILEDIR -e $SEARCHSTR | wc -l)
        echo "The number of files are $NUMFILES and the number of matching lines are $MATCHINGLINES"
    else
        echo "$FILEDIR does not exist"
        exit 1
    fi
else
    echo "Please enter 2 parameters as expected: FILEDIR and SEARCHSTR"
    exit 1
fi

