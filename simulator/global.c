#include <stdio.h>
#include <stdlib.h>

unsigned iCacheLength;
unsigned dCacheLength;
unsigned iPageTableEntries;
unsigned dPageTableEntries;
unsigned iTLBEntries;
unsigned dTLBEntries;

unsigned dTLBValidSet, dPTEValidPPN;
unsigned iTLBValidSet, iPTEValidPPN;
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
char dRun[1024], iRun[1024];
char dDisk[1024], iDisk[1024];
FILE *err, *snap, *iImg, *dImg, *report;
