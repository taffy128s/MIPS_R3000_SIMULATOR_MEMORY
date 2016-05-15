#include <stdio.h>
#include <stdlib.h>

unsigned iCacheHit, iCacheMiss;
unsigned dCacheHit, dCacheMiss;
unsigned iTLBHit, iTLBMiss;
unsigned dTLBHit, dTLBMiss;
unsigned iPageTableHit, iPageTableMiss;
unsigned dPageTableHit, dPageTableMiss;
unsigned iMemorySize, dMemorySize;
unsigned iMemoryPageSize, dMemoryPageSize;
unsigned totalSizeOfICache, blockSizeOfICache, setAssOfICache;
unsigned totalSizeOfDCache, blockSizeOfDCache, setAssOfDCache;
unsigned writeToZero, numberOverflow, memoryOverflow, memoryMisalignment;
unsigned iImgLen, dImgLen, iImgLenResult, dImgLenResult, reg[32], PC, cycle;
char *iImgBuffer, *dImgBuffer;
char dDisk[1024], iDisk[1024];
FILE *err, *snap, *iImg, *dImg;
