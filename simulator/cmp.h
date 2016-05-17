void initTLB();

void initPTE();

void initCache();

void initMemory();

int checkITLBHit(unsigned vm);

int checkIPTEHit(unsigned vm);

int checkICacheHit(unsigned pMemoryAddr);

unsigned findIMemoryReplaceIdx();

void swapIMemory(unsigned diskAddr, unsigned idx);

void deactivateIPTE(unsigned idx);

void updateIPTE(unsigned vm, unsigned idx);

unsigned findITLBReplaceIdx();

void updateITLBWhenPageTableMiss(unsigned vm, unsigned idx);

void updateITLBWhenPageTableHit(unsigned vm);

unsigned findICacheReplaceIdx(unsigned cacheIdx);

void updateICache(unsigned pMemoryAddr);
