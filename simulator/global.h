#include <stdio.h>
#include <stdlib.h>

extern unsigned iTLBValidSet, iPTEValidPPN, iCacheContent;
extern unsigned iCacheHit, iCacheMiss;
extern unsigned dCacheHit, dCacheMiss;
extern unsigned iTLBHit, iTLBMiss;
extern unsigned dTLBHit, dTLBMiss;
extern unsigned iPageTableHit, iPageTableMiss;
extern unsigned dPageTableHit, dPageTableMiss;
extern unsigned iMemorySize, dMemorySize;
extern unsigned iMemoryPageSize, dMemoryPageSize;
extern unsigned totalSizeOfICache, blockSizeOfICache, setAssOfICache;
extern unsigned totalSizeOfDCache, blockSizeOfDCache, setAssOfDCache;
extern unsigned iMemorySize, dMemorySize;
extern unsigned writeToZero, numberOverflow, memoryOverflow, memoryMisalignment;
extern unsigned iImgLen, dImgLen, iImgLenResult, dImgLenResult, reg[32], PC, cycle;
extern char *iImgBuffer, *dImgBuffer;
extern char dRun[1024], iRun[1024];
extern char dDisk[1024], iDisk[1024];
extern FILE *err, *snap, *iImg, *dImg;
