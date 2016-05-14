#include <stdio.h>
#include <stdlib.h>

unsigned writeToZero, numberOverflow, memoryOverflow, memoryMisalignment;
unsigned iImgLen, dImgLen, iImgLenResult, dImgLenResult, reg[32], PC, cycle;
char *iImgBuffer, *dImgBuffer;
char dDisk[1024], iDisk[1024];
FILE *err, *snap, *iImg, *dImg;
