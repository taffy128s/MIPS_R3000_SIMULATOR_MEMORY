#include "global.h"
#include "cmp.h"
#include <vector>
#include <limits.h>

using namespace std;

// TLB contains tag, set, last cycle for LRU(also for valid bit).
struct tlb {
	unsigned tag, set, lastcycle;
	tlb() {}
	tlb(unsigned tag, unsigned set, unsigned lastcycle) {
		this->tag = tag;
		this->set = set;
		this->lastcycle = lastcycle;
	}
};

// PTE contains valid bit, PPN.
struct pte {
	unsigned valid, pPageNumber;
	pte() {}
	pte(unsigned valid, unsigned pPageNumber) {
		this->valid = valid;
		this->pPageNumber = pPageNumber;
	}
};

// A cache block contains tag, valid bit, MRU.
struct cacheBlock {
	unsigned tag, valid, MRU;
    char *content;
	cacheBlock() {}
	cacheBlock(unsigned tag, unsigned valid, unsigned MRU) {
		this->tag = tag;
		this->valid = valid;
        this->MRU = MRU;
		this->content = new char[blockSizeOfICache];
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
static char *iCacheTemp;

// Initialize the TLBs.
void initTLB() {
	for (unsigned i = 0; i < iTLBEntries; i++)
		iTLB.push_back(tlb(0, 0, 0));
	for (unsigned i = 0; i < dTLBEntries; i++)
		dTLB.push_back(tlb(0, 0, 0));
}

// Initialize the PTEs.
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

// Initialize the caches.
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

// Check iTLB hits or not. If yes, update the values of lastcycle and iTLBValidSet.
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

// Check iPTE hits or not. If yes, update the value of iPTEValidPPN.
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

// Check iCache hits or not. If yes, update the values of MRU(important!!), iCachePointer.
int checkICacheHit(unsigned pMemoryAddr) {
	unsigned cacheIdx = pMemoryAddr / blockSizeOfICache % iCacheLength;
    unsigned tempTag = pMemoryAddr / blockSizeOfICache / iCacheLength;
	for (unsigned i = 0; i < setAssOfICache; i++) {
		if (iCache[cacheIdx][i].valid == 1 && iCache[cacheIdx][i].tag == tempTag) {
            // important!!
            iCache[cacheIdx][i].MRU = 1;
            if (chkICacheMRUAllOne(cacheIdx) == 1)
                clearICacheMRU(cacheIdx, i);
			iCachePointer = iCache[cacheIdx][i].content + pMemoryAddr % blockSizeOfICache;
			iCacheHit++;
			return 1;
		}
	}
	iCacheMiss++;
	return 0;
}

// Check iCache and turn it to not valid when need to swap.
int checkICache(unsigned pMemoryAddr) {
	unsigned cacheIdx = pMemoryAddr / blockSizeOfICache % iCacheLength;
	unsigned tempTag = pMemoryAddr / blockSizeOfICache / iCacheLength;
	for (unsigned i = 0; i < setAssOfICache; i++) {
		if (iCache[cacheIdx][i].valid == 1 && iCache[cacheIdx][i].tag == tempTag) {
			iCacheTemp = iCache[cacheIdx][i].content;
			iCache[cacheIdx][i].valid = 0;
			return 1;
		}
	}
	return 0;
}

// When need to swap, find iMemory swap addr.
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

// Deactivate the iPTE when something is swapped.
void deactivateIPTE(unsigned idx) {
	unsigned virtualPageNum = iMemory[idx].diskAddr / iMemoryPageSize;
	iPTE[virtualPageNum].valid = 0;
    // Make sure to move the cache content to the memory.
	for (unsigned i = idx; i < idx + iMemoryPageSize; i += blockSizeOfICache) {
        //printf("%u\n", i);
		if (checkICache(i) == 1) {
			unsigned j = i;
			for (unsigned k = 0; k < blockSizeOfICache; k++) {
				iMemory[j++].content = iCacheTemp[k];
			}
		}
	}
}

// Swap iMemory when the target is not present.
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

// Update iPTE.
void updateIPTE(unsigned vm, unsigned idx) {
	unsigned PPN = idx / iMemoryPageSize;
	unsigned virtualPageNum = vm / iMemoryPageSize;
	iPTE[virtualPageNum].pPageNumber = PPN;
	iPTE[virtualPageNum].valid = 1;
}

// Find the idx of the TLB for changing.
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

// As function name.
void updateITLBWhenPageTableMiss(unsigned vm, unsigned idx) {
    unsigned idxToReplace = findITLBReplaceIdx();
    unsigned virtualPageNum = vm / iMemoryPageSize;
    unsigned PPN = idx / iMemoryPageSize;
    iTLB[idxToReplace].lastcycle = cycle;
    iTLB[idxToReplace].tag = virtualPageNum;
    iTLB[idxToReplace].set = PPN;
}

// As function name.
void updateITLBWhenPageTableHit(unsigned vm) {
    unsigned idxToReplace = findITLBReplaceIdx();
    unsigned virtualPageNum = vm / iMemoryPageSize;
    iTLB[idxToReplace].set = iPTE[virtualPageNum].pPageNumber;
    iTLB[idxToReplace].lastcycle = cycle;
    iTLB[idxToReplace].tag = virtualPageNum;
}

// [important] Find cache replacing idx. 1->least index not valid, 2->least index MRU == 0.
unsigned findICacheReplaceIdx(unsigned cacheIdx) {
    for (unsigned i = 0; i < iCache[cacheIdx].size(); i++) {
        if (iCache[cacheIdx][i].valid == 0) {
            return i;
        }
    }
    for (unsigned i = 0; i < iCache[cacheIdx].size(); i++) {
        if (iCache[cacheIdx][i].MRU == 0) {
            return i;
        }
    }
    printf("Error occured at findICacheReplaceIdx.\n");
    return INT_MAX;
}

// Check the cache MRU all one or not.
unsigned chkICacheMRUAllOne(unsigned cacheIdx) {
    unsigned allOne = 1;
    for (unsigned i = 0; i < iCache[cacheIdx].size(); i++)
        allOne &= iCache[cacheIdx][i].MRU;
    return allOne;
}

// Clear all the MRUs except the one which was just changed.
void clearICacheMRU(unsigned cacheIdx, unsigned thisIdx) {
    for (unsigned i = 0; i < iCache[cacheIdx].size(); i++)
        if (i != thisIdx) iCache[cacheIdx][i].MRU = 0;
}

// [important] Update iCache.
void updateICache(unsigned pMemoryAddr) {
    unsigned cacheIdx = pMemoryAddr / blockSizeOfICache % iCacheLength;
    unsigned tempTag = pMemoryAddr / blockSizeOfICache / iCacheLength;
    unsigned setToReplace = findICacheReplaceIdx(cacheIdx);
    // Swap with memory when valid.
    if (iCache[cacheIdx][setToReplace].valid == 1) {
        unsigned j = 0;
        for (unsigned i = pMemoryAddr; i < pMemoryAddr + blockSizeOfICache; i++)
            iMemory[i].content = iCache[cacheIdx][setToReplace].content[j++];
    }
    iCache[cacheIdx][setToReplace].tag = tempTag;
    iCache[cacheIdx][setToReplace].MRU = 1;
    iCache[cacheIdx][setToReplace].valid = 1;
    unsigned j = 0;
    for (unsigned i = pMemoryAddr; i < pMemoryAddr + blockSizeOfICache; i++)
        iCache[cacheIdx][setToReplace].content[j++] = iMemory[i].content;
    if (chkICacheMRUAllOne(cacheIdx) == 1)
        clearICacheMRU(cacheIdx, setToReplace);
}
