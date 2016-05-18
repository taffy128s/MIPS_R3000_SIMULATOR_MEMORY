#include "global.h"
#include "cmp.h"
#include <vector>
#include <limits.h>

using namespace std;

struct tlb {
	unsigned tag, valid, set, lastused;
	tlb() {}
	tlb(unsigned tag, unsigned set, unsigned lastused) {
		this->tag = tag;
		this->set = set;
		this->lastused = lastused;
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

// cache block size can be configured
struct cacheBlock {
	unsigned tag, valid, pseudoLRU;
    char *content;
	cacheBlock() {}
	cacheBlock(unsigned tag, unsigned valid, unsigned pseudoLRU) {
		this->tag = tag;
		this->valid = valid;
        this->pseudoLRU = pseudoLRU;
		this->content = new char[blockSizeOfICache];
	}
};

struct memoryBlock {
	char content;
	unsigned lastused;
    unsigned diskAddr;
};

static vector<tlb> iTLB;
static vector<tlb> dTLB;
static pte *iPTE;
static pte *dPTE;
static vector<cacheBlock> *iCache;
static vector<cacheBlock> *dCache;
static memoryBlock *iMemory;
static memoryBlock *dMemory;
static char *iCacheTemp;

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

void initMemory() {
	iMemory = new memoryBlock[iMemorySize];
	for (unsigned i = 0; i < iMemorySize; i++) {
		iMemory[i].content = 0;
		iMemory[i].lastused = 0;
        iMemory[i].diskAddr = 0;
	}
	dMemory = new memoryBlock[dMemorySize];
	for (unsigned i = 0; i < dMemorySize; i++) {
		dMemory[i].content = 0;
		dMemory[i].lastused = 0;
        dMemory[i].diskAddr = 0;
	}
}

int checkITLBHit(unsigned vm) {
	unsigned tempPageNum = vm / iMemoryPageSize;
	for (unsigned i = 0; i < iTLB.size(); i++) {
		if (iTLB[i].lastused > 0 && iTLB[i].tag == tempPageNum) {
			iTLBValidSet = iTLB[i].set;
			iTLBHit++;
			return 1;
		}
	}
	iTLBMiss++;
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

int checkICacheHit(unsigned pMemoryAddr) {
	unsigned cacheIdx = pMemoryAddr / blockSizeOfICache % iCacheLength;
    unsigned tempTag = pMemoryAddr / blockSizeOfICache / iCacheLength;
	for (unsigned i = 0; i < setAssOfICache; i++) {
		printf("cacheIdx: %u, finding Tag: %u\n", cacheIdx, tempTag);
		if (iCache[cacheIdx][i].valid == 1 && iCache[cacheIdx][i].tag == tempTag) {
			iCachePointer = iCache[cacheIdx][i].content + pMemoryAddr % blockSizeOfICache;
			iCacheHit++;
			puts("hit!");
			return 1;
		}
	}
	iCacheMiss++;
	return 0;
}

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

unsigned findIMemoryReplaceIdx() {
    unsigned smallestN = INT_MAX;
    unsigned smallestIdx;
	for (unsigned i = 0; i < iMemorySize; i++) {
        if (iMemory[i].lastused < smallestN) {
            smallestN = iMemory[i].lastused;
            smallestIdx = i;
        }
        if (iMemory[i].lastused == 0) {
            return i;
        }
    }
    return smallestIdx;
}

void deactivateIPTE(unsigned idx) {
	unsigned virtualPageNum = iMemory[idx].diskAddr / iMemoryPageSize;
	iPTE[virtualPageNum].valid = 0;
	for (unsigned i = idx; i < idx + iMemoryPageSize; i += blockSizeOfICache) {
		printf("yo\n");
		if (checkICache(i) == 1) {
			unsigned j = i;
			for (unsigned k = 0; k < blockSizeOfICache; k++) {
				iMemory[j++].content = iCacheTemp[k];
			}
		}
	}
}

void swapIMemory(unsigned diskAddr, unsigned idx) {
    if (iMemory[idx].lastused > 0) {
		deactivateIPTE(idx);
        for (unsigned i = idx; i < idx + iMemoryPageSize; i++) {
            iDisk[iMemory[i].diskAddr] = iMemory[i].content;
            iMemory[i].diskAddr = diskAddr;
            iMemory[i].content = iDisk[diskAddr++];
            iMemory[i].lastused = cycle;
        }
    } else {
        for (unsigned i = idx; i < idx + iMemoryPageSize; i++) {
            iMemory[i].diskAddr = diskAddr;
            iMemory[i].content = iDisk[diskAddr++];
            iMemory[i].lastused = cycle;
        }
    }
}

void updateIPTE(unsigned vm, unsigned idx) {
	unsigned PPN = idx / iMemoryPageSize;
	unsigned virtualPageNum = vm / iMemoryPageSize;
	iPTE[virtualPageNum].pPageNumber = PPN;
	iPTE[virtualPageNum].valid = 1;
}

unsigned findITLBReplaceIdx() {
	unsigned smallestN = INT_MAX;
    unsigned smallestIdx;
    for (unsigned i = 0; i < iTLB.size(); i++) {
        if (iTLB[i].lastused < smallestN) {
            smallestN = iTLB[i].lastused;
            smallestIdx = i;
        }
        if (iTLB[i].lastused == 0) {
            return i;
        }
    }
    return smallestIdx;
}

void updateITLBWhenPageTableMiss(unsigned vm, unsigned idx) {
    unsigned PPN = idx / iMemoryPageSize;
    unsigned virtualPageNum = vm / iMemoryPageSize;
    unsigned idxToReplace = findITLBReplaceIdx();
    iTLB[idxToReplace].lastused = cycle;
    iTLB[idxToReplace].tag = virtualPageNum;
    iTLB[idxToReplace].set = PPN;
}

void updateITLBWhenPageTableHit(unsigned vm) {
    unsigned idxToReplace = findITLBReplaceIdx();
    unsigned virtualPageNum = vm / iMemoryPageSize;
    iTLB[idxToReplace].set = iPTE[virtualPageNum].pPageNumber;
    iTLB[idxToReplace].lastused = cycle;
    iTLB[idxToReplace].tag = virtualPageNum;
}

unsigned findICacheReplaceIdx(unsigned cacheIdx) {
    for (unsigned i = 0; i < iCache[cacheIdx].size(); i++) {
        if (iCache[cacheIdx][i].pseudoLRU == 0) {
            return i;
        }
    }
    printf("Error occured at findICacheReplaceIdx.\n");
    return INT_MAX;
}

unsigned chkICachePseudoAllOne(unsigned cacheIdx) {
    unsigned allOne = 1;
    for (unsigned i = 0; i < iCache[cacheIdx].size(); i++)
        allOne &= iCache[cacheIdx][i].pseudoLRU;
    return allOne;
}

void clearICachePseudo(unsigned cacheIdx, unsigned thisIdx) {
    for (unsigned i = 0; i < iCache[cacheIdx].size(); i++)
        if (i != thisIdx) iCache[cacheIdx][i].pseudoLRU = 0;
}

void updateICache(unsigned pMemoryAddr) {
    unsigned cacheIdx = pMemoryAddr / blockSizeOfICache % iCacheLength;
    unsigned tempTag = pMemoryAddr / blockSizeOfICache / iCacheLength;
    unsigned setToReplace = findICacheReplaceIdx(cacheIdx);
    if (iCache[cacheIdx][setToReplace].valid == 1) {
        // TODO: swap with iMemory
        unsigned j = 0;
        for (unsigned i = pMemoryAddr; i < pMemoryAddr + blockSizeOfICache; i++)
            iMemory[i].content = iCache[cacheIdx][setToReplace].content[j++];
    }
    iCache[cacheIdx][setToReplace].tag = tempTag;
    iCache[cacheIdx][setToReplace].pseudoLRU = 1;
    iCache[cacheIdx][setToReplace].valid = 1;
    unsigned j = 0;
    for (unsigned i = pMemoryAddr; i < pMemoryAddr + blockSizeOfICache; i++)
        iCache[cacheIdx][setToReplace].content[j++] = iMemory[i].content;
    if (chkICachePseudoAllOne(cacheIdx) == 1)
        clearICachePseudo(cacheIdx, setToReplace);

}
