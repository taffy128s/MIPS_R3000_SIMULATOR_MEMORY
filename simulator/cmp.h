void initTLB();

void initPTE();

void initCache();

void initMemory();

/***/

int checkITLBHit(unsigned vm);

int checkDTLBHit(unsigned vm);

/***/

int checkIPTEHit(unsigned vm);

int checkDPTEHit(unsigned vm);

/***/

int checkICacheHit(unsigned pMemoryAddr);

int checkDCacheHit(unsigned pMemoryAddr);

/***/

int checkICache(unsigned pMemoryAddr);

int checkDCache(unsigned pMemoryAddr);

/***/

unsigned findIMemoryReplaceIdx();

unsigned findDMemoryReplaceIdx();

/***/

void deactivateITLB(unsigned vpn);

void deactivateDTLB(unsigned vpn);

/***/

void deactivateIPTE(unsigned idx);

void deactivateDPTE(unsigned idx);

/***/

void swapIMemory(unsigned diskAddr, unsigned idx);

void swapDMemory(unsigned diskAddr, unsigned idx);

/***/

void updateIPTE(unsigned vm, unsigned idx);

void updateDPTE(unsigned vm, unsigned idx);

/***/

unsigned findITLBReplaceIdx();

unsigned findDTLBReplaceIdx();

/***/

void updateITLBWhenPageTableMiss(unsigned vm, unsigned idx);

void updateDTLBWhenPageTableMiss(unsigned vm, unsigned idx);

/***/

void updateITLBWhenPageTableHit(unsigned vm);

void updateDTLBWhenPageTableHit(unsigned vm);

/***/

unsigned findICacheReplaceIdx(unsigned cacheIdx);

unsigned findDCacheReplaceIdx(unsigned cacheIdx);

/***/

unsigned chkICacheMRUAllOne(unsigned cacheIdx);

unsigned chkDCacheMRUAllOne(unsigned cacheIdx);

/***/

void clearICacheMRU(unsigned cacheIdx, unsigned thisIdx);

void clearDCacheMRU(unsigned cacheIdx, unsigned thisIdx);

/***/

void updateICache(unsigned pMemoryAddr);

void updateDCache(unsigned pMemoryAddr);
