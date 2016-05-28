#include "global.h"
#include "cmp.h"
#include <vector>
#include <climits>

using namespace std;

struct tlb {
    unsigned tag, set, lastcycle;
    tlb() {}
    tlb(unsigned tag, unsigned set, unsigned lastcycle) {
        this->tag = tag;
        this->set = set;
        this->lastcycle = lastcycle;
    }
};

struct pte {
    unsigned valid, pPageNumber;
    pte() {}
    pte(unsigned valid, unsigned pPageNumber) {
        this->valid = valid;
        this->pPageNumber = pPageNumber;
    }
};

struct cacheBlock {
    unsigned tag, valid, MRU;
    cacheBlock() {}
    cacheBlock(unsigned tag, unsigned valid, unsigned MRU) {
        this->tag = tag;
        this->valid = valid;
        this->MRU = MRU;
    }
};

// A memory block contains content, last cycle for LRU, diskAddr.
struct memoryBlock {
    char content;
    unsigned lastcycle;
    unsigned diskAddr;
};

// Declare all the variables.
static vector<tlb> iTLB;
static vector<tlb> dTLB;
static pte *iPTE;
static pte *dPTE;
static vector<cacheBlock> *iCache;
static vector<cacheBlock> *dCache;
static memoryBlock *iMemory;
static memoryBlock *dMemory;

void initTLB() {
    for (unsigned i = 0; i < iTLBEntries; i++)
        iTLB.push_back(tlb(0, 0, 0));
    for (unsigned i = 0; i < dTLBEntries; i++)
        dTLB.push_back(tlb(0, 0, 0));
}

void initPTE() {
    iPTE = new pte[iPageTableEntries];
    for (unsigned i = 0; i < iPageTableEntries; i++) {
        iPTE[i].valid = 0;
        iPTE[i].pPageNumber = 0;
    }
    dPTE = new pte[dPageTableEntries];
    for (unsigned i = 0; i < dPageTableEntries; i++) {
        dPTE[i].valid = 0;
        dPTE[i].pPageNumber = 0;
    }
}

void initCache() {
    iCache = new vector<cacheBlock>[iCacheLength];
    for (unsigned i = 0; i < iCacheLength; i++)
        for (unsigned j = 0; j < setAssOfICache; j++)
            iCache[i].push_back(cacheBlock(0, 0, 0));
    dCache = new vector<cacheBlock>[dCacheLength];
    for (unsigned i = 0; i < dCacheLength; i++)
        for (unsigned j = 0; j < setAssOfDCache; j++)
            dCache[i].push_back(cacheBlock(0, 0, 0));
}

// Initialize the memories.
void initMemory() {
    iMemory = new memoryBlock[iMemorySize];
    for (unsigned i = 0; i < iMemorySize; i++) {
        iMemory[i].content = 0;
        iMemory[i].lastcycle = 0;
        iMemory[i].diskAddr = 0;
    }
    dMemory = new memoryBlock[dMemorySize];
    for (unsigned i = 0; i < dMemorySize; i++) {
        dMemory[i].content = 0;
        dMemory[i].lastcycle = 0;
        dMemory[i].diskAddr = 0;
    }
}

int checkITLBHit(unsigned vm) {
    unsigned tempPageNum = vm / iMemoryPageSize;
    for (unsigned i = 0; i < iTLB.size(); i++) {
        if (iTLB[i].lastcycle > 0 && iTLB[i].tag == tempPageNum) {
            iTLB[i].lastcycle = cycle;
            iTLBValidSet = iTLB[i].set;
            iTLBHit++;
            return 1;
        }
    }
    iTLBMiss++;
    return 0;
}

int checkDTLBHit(unsigned vm) {
    unsigned tempPageNum = vm / dMemoryPageSize;
    for (unsigned i = 0; i < dTLB.size(); i++) {
        if (dTLB[i].lastcycle > 0 && dTLB[i].tag == tempPageNum) {
            dTLB[i].lastcycle = cycle;
            dTLBValidSet = dTLB[i].set;
            dTLBHit++;
            return 1;
        }
    }
    dTLBMiss++;
    return 0;
}

int checkIPTEHit(unsigned vm) {
    unsigned tempPageNum = vm / iMemoryPageSize;
    if (iPTE[tempPageNum].valid == 1) {
        iPTEValidPPN = iPTE[tempPageNum].pPageNumber;
        iPageTableHit++;
        return 1;
    }
    iPageTableMiss++;
    return 0;
}

int checkDPTEHit(unsigned vm) {
    unsigned tempPageNum = vm / dMemoryPageSize;
    if (dPTE[tempPageNum].valid == 1) {
        dPTEValidPPN = dPTE[tempPageNum].pPageNumber;
        dPageTableHit++;
        return 1;
    }
    dPageTableMiss++;
    return 0;
}

int checkICacheHit(unsigned pMemoryAddr) {
    unsigned cacheIdx = pMemoryAddr / blockSizeOfICache % iCacheLength;
    unsigned tempTag = pMemoryAddr / blockSizeOfICache / iCacheLength;
    /*for (unsigned i = 0; i < iCacheLength; i++) {
        for (unsigned j = 0; j < setAssOfICache; j++) {
            printf("[%3u %3u %3u] ", iCache[i][j].tag, iCache[i][j].valid, iCache[i][j].MRU);
        }
        puts("");
    }*/
    for (unsigned i = 0; i < setAssOfICache; i++) {
        if (iCache[cacheIdx][i].valid == 1 && iCache[cacheIdx][i].tag == tempTag) {
            iCache[cacheIdx][i].MRU = 1;
            if (chkICacheMRUAllOne(cacheIdx) == 1)
                clearICacheMRU(cacheIdx, i);
            iCacheHit++;
            //puts("hit!");
            return 1;
        }
    }
    iCacheMiss++;
    //puts("miss!");
    return 0;
}

int checkDCacheHit(unsigned pMemoryAddr) {
    unsigned cacheIdx = pMemoryAddr / blockSizeOfDCache % dCacheLength;
    unsigned tempTag = pMemoryAddr / blockSizeOfDCache / dCacheLength;
    for (unsigned i = 0; i < setAssOfDCache; i++) {
        if (dCache[cacheIdx][i].valid == 1 && dCache[cacheIdx][i].tag == tempTag) {
            dCache[cacheIdx][i].MRU = 1;
            if (chkDCacheMRUAllOne(cacheIdx) == 1)
                clearDCacheMRU(cacheIdx, i);
            dCacheHit++;
            return 1;
        }
    }
    dCacheMiss++;
    return 0;
}

int checkICache(unsigned pMemoryAddr) {
    unsigned cacheIdx = pMemoryAddr / blockSizeOfICache % iCacheLength;
    unsigned tempTag = pMemoryAddr / blockSizeOfICache / iCacheLength;
    for (unsigned i = 0; i < setAssOfICache; i++) {
        if (iCache[cacheIdx][i].valid == 1 && iCache[cacheIdx][i].tag == tempTag) {
            iCache[cacheIdx][i].valid = 0;
            return 1;
        }
    }
    return 0;
}

int checkDCache(unsigned pMemoryAddr) {
    unsigned cacheIdx = pMemoryAddr / blockSizeOfDCache % dCacheLength;
    unsigned tempTag = pMemoryAddr / blockSizeOfDCache / dCacheLength;
    for (unsigned i = 0; i < setAssOfDCache; i++) {
        if (dCache[cacheIdx][i].valid == 1 && dCache[cacheIdx][i].tag == tempTag) {
            dCache[cacheIdx][i].valid = 0;
            return 1;
        }
    }
    return 0;
}

unsigned findIMemoryReplaceIdx() {
    unsigned smallestN = INT_MAX;
    unsigned smallestIdx;
    for (unsigned i = 0; i < iMemorySize; i++) {
        if (iMemory[i].lastcycle < smallestN) {
            smallestN = iMemory[i].lastcycle;
            smallestIdx = i;
        }
        if (iMemory[i].lastcycle == 0) {
            return i;
        }
    }
    return smallestIdx;
}

unsigned findDMemoryReplaceIdx() {
    unsigned smallestN = INT_MAX;
    unsigned smallestIdx;
    for (unsigned i = 0; i < dMemorySize; i++) {
        if (dMemory[i].lastcycle < smallestN) {
            smallestN = dMemory[i].lastcycle;
            smallestIdx = i;
        }
        if (dMemory[i].lastcycle == 0) {
            return i;
        }
    }
    return smallestIdx;
}

void deactivateITLB(unsigned vpn) {
    unsigned tempPageNum = vpn;
    for (unsigned i= 0; i < iTLB.size(); i++) {
        if (iTLB[i].lastcycle > 0 && iTLB[i].tag == tempPageNum) {
            iTLB[i].lastcycle = 0;
        }
    }
}

void deactivateDTLB(unsigned vpn) {
    unsigned tempPageNum = vpn;
    for (unsigned i= 0; i < dTLB.size(); i++) {
        if (dTLB[i].lastcycle > 0 && dTLB[i].tag == tempPageNum) {
            dTLB[i].lastcycle = 0;
        }
    }
}

void deactivateIPTE(unsigned idx) {
    unsigned virtualPageNum = iMemory[idx].diskAddr / iMemoryPageSize;
    deactivateITLB(virtualPageNum);
    iPTE[virtualPageNum].valid = 0;
    for (unsigned i = idx; i < idx + iMemoryPageSize; i += blockSizeOfICache) {
        checkICache(i);
    }
}

void deactivateDPTE(unsigned idx) {
    unsigned virtualPageNum = dMemory[idx].diskAddr / dMemoryPageSize;
    deactivateDTLB(virtualPageNum);
    dPTE[virtualPageNum].valid = 0;
    for (unsigned i = idx; i < idx + dMemoryPageSize; i += blockSizeOfDCache) {
        checkDCache(i);
    }
}

void swapIMemory(unsigned diskAddr, unsigned idx) {
    if (iMemory[idx].lastcycle > 0) {
        deactivateIPTE(idx);
        for (unsigned i = idx; i < idx + iMemoryPageSize; i++) {
            iDisk[iMemory[i].diskAddr] = iMemory[i].content;
            iMemory[i].diskAddr = diskAddr;
            iMemory[i].content = iDisk[diskAddr++];
            iMemory[i].lastcycle = cycle;
        }
    } else {
        for (unsigned i = idx; i < idx + iMemoryPageSize; i++) {
            iMemory[i].diskAddr = diskAddr;
            iMemory[i].content = iDisk[diskAddr++];
            iMemory[i].lastcycle = cycle;
        }
    }
}

void swapDMemory(unsigned diskAddr, unsigned idx) {
    if (dMemory[idx].lastcycle > 0) {
        deactivateDPTE(idx);
        for (unsigned i = idx; i < idx + dMemoryPageSize; i++) {
            dDisk[dMemory[i].diskAddr] = dMemory[i].content;
            dMemory[i].diskAddr = diskAddr;
            dMemory[i].content = dDisk[diskAddr++];
            dMemory[i].lastcycle = cycle;
        }
    } else {
        for (unsigned i = idx; i < idx + dMemoryPageSize; i++) {
            dMemory[i].diskAddr = diskAddr;
            dMemory[i].content = dDisk[diskAddr++];
            dMemory[i].lastcycle = cycle;
        }
    }
}

void updateIPTE(unsigned vm, unsigned idx) {
    unsigned PPN = idx / iMemoryPageSize;
    unsigned virtualPageNum = vm / iMemoryPageSize;
    iPTE[virtualPageNum].pPageNumber = PPN;
    iPTE[virtualPageNum].valid = 1;
}

void updateDPTE(unsigned vm, unsigned idx) {
    unsigned PPN = idx / dMemoryPageSize;
    unsigned virtualPageNum = vm / dMemoryPageSize;
    dPTE[virtualPageNum].pPageNumber = PPN;
    dPTE[virtualPageNum].valid = 1;
}

unsigned findITLBReplaceIdx() {
    unsigned smallestN = INT_MAX;
    unsigned smallestIdx;
    for (unsigned i = 0; i < iTLB.size(); i++) {
        if (iTLB[i].lastcycle < smallestN) {
            smallestN = iTLB[i].lastcycle;
            smallestIdx = i;
        }
        if (iTLB[i].lastcycle == 0) {
            return i;
        }
    }
    return smallestIdx;
}

unsigned findDTLBReplaceIdx() {
    unsigned smallestN = INT_MAX;
    unsigned smallestIdx;
    for (unsigned i = 0; i < dTLB.size(); i++) {
        if (dTLB[i].lastcycle < smallestN) {
            smallestN = dTLB[i].lastcycle;
            smallestIdx = i;
        }
        if (dTLB[i].lastcycle == 0) {
            return i;
        }
    }
    return smallestIdx;
}

void updateITLBWhenPageTableMiss(unsigned vm, unsigned idx) {
    unsigned idxToReplace = findITLBReplaceIdx();
    unsigned virtualPageNum = vm / iMemoryPageSize;
    unsigned PPN = idx / iMemoryPageSize;
    iTLB[idxToReplace].lastcycle = cycle;
    iTLB[idxToReplace].tag = virtualPageNum;
    iTLB[idxToReplace].set = PPN;
}

void updateDTLBWhenPageTableMiss(unsigned vm, unsigned idx) {
    unsigned idxToReplace = findDTLBReplaceIdx();
    unsigned virtualPageNum = vm / dMemoryPageSize;
    unsigned PPN = idx / dMemoryPageSize;
    dTLB[idxToReplace].lastcycle = cycle;
    dTLB[idxToReplace].tag = virtualPageNum;
    dTLB[idxToReplace].set = PPN;
}

void updateITLBWhenPageTableHit(unsigned vm) {
    unsigned idxToReplace = findITLBReplaceIdx();
    unsigned virtualPageNum = vm / iMemoryPageSize;
    iTLB[idxToReplace].set = iPTE[virtualPageNum].pPageNumber;
    iTLB[idxToReplace].lastcycle = cycle;
    iTLB[idxToReplace].tag = virtualPageNum;
}

void updateDTLBWhenPageTableHit(unsigned vm) {
    unsigned idxToReplace = findDTLBReplaceIdx();
    unsigned virtualPageNum = vm / dMemoryPageSize;
    dTLB[idxToReplace].set = dPTE[virtualPageNum].pPageNumber;
    dTLB[idxToReplace].lastcycle = cycle;
    dTLB[idxToReplace].tag = virtualPageNum;
}

unsigned findICacheReplaceIdx(unsigned cacheIdx) {
    /*for (unsigned i = 0; i < setAssOfICache; i++) {
        if (iCache[cacheIdx][i].valid == 0) {
            return i;
        }
    }*/
    for (unsigned i = 0; i < setAssOfICache; i++) {
        if (iCache[cacheIdx][i].MRU == 0) {
            return i;
        }
    }
    printf("Error occured at findICacheReplaceIdx.\n");
    return INT_MAX;
}

unsigned findDCacheReplaceIdx(unsigned cacheIdx) {
    /*for (unsigned i = 0; i < setAssOfDCache; i++) {
        if (dCache[cacheIdx][i].valid == 0) {
            return i;
        }
    }*/
    for (unsigned i = 0; i < setAssOfDCache; i++) {
        if (dCache[cacheIdx][i].MRU == 0) {
            return i;
        }
    }
    printf("Error occured at findDCacheReplaceIdx.\n");
    return INT_MAX;
}

unsigned chkICacheMRUAllOne(unsigned cacheIdx) {
    unsigned allOne = 1;
    for (unsigned i = 0; i < setAssOfICache; i++)
        allOne &= iCache[cacheIdx][i].MRU;
    return allOne;
}

unsigned chkDCacheMRUAllOne(unsigned cacheIdx) {
    unsigned allOne = 1;
    for (unsigned i = 0; i < setAssOfDCache; i++)
        allOne &= dCache[cacheIdx][i].MRU;
    return allOne;
}

void clearICacheMRU(unsigned cacheIdx, unsigned thisIdx) {
    if (setAssOfICache == 1) iCache[cacheIdx][thisIdx].MRU = 0;
    for (unsigned i = 0; i < iCache[cacheIdx].size(); i++)
        if (i != thisIdx) iCache[cacheIdx][i].MRU = 0;
}

void clearDCacheMRU(unsigned cacheIdx, unsigned thisIdx) {
    if (setAssOfDCache == 1) dCache[cacheIdx][thisIdx].MRU = 0;
    for (unsigned i = 0; i < dCache[cacheIdx].size(); i++)
        if (i != thisIdx) dCache[cacheIdx][i].MRU = 0;
}

void updateICache(unsigned pMemoryAddr) {
    unsigned cacheIdx = pMemoryAddr / blockSizeOfICache % iCacheLength;
    unsigned tempTag = pMemoryAddr / blockSizeOfICache / iCacheLength;
    unsigned setToReplace = findICacheReplaceIdx(cacheIdx);
    iCache[cacheIdx][setToReplace].tag = tempTag;
    iCache[cacheIdx][setToReplace].MRU = 1;
    iCache[cacheIdx][setToReplace].valid = 1;
    if (chkICacheMRUAllOne(cacheIdx) == 1)
        clearICacheMRU(cacheIdx, setToReplace);
}

void updateDCache(unsigned pMemoryAddr) {
    unsigned cacheIdx = pMemoryAddr / blockSizeOfDCache % dCacheLength;
    unsigned tempTag = pMemoryAddr / blockSizeOfDCache / dCacheLength;
    unsigned setToReplace = findDCacheReplaceIdx(cacheIdx);
    dCache[cacheIdx][setToReplace].tag = tempTag;
    dCache[cacheIdx][setToReplace].MRU = 1;
    dCache[cacheIdx][setToReplace].valid = 1;
    if (chkDCacheMRUAllOne(cacheIdx) == 1)
        clearDCacheMRU(cacheIdx, setToReplace);
}
