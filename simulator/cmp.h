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
