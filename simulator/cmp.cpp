#include "global.h"
#include "cmp.h"
#include <vector>

using namespace std;

struct tlb {
	unsigned tag, valid, set, lastused;
	tlb() {}
	tlb(unsigned tag, unsigned valid, unsigned set, unsigned lastused) {
		this->tag = tag;
		this->valid = valid;
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

struct cacheBlock {
	unsigned tag, valid, content;
	cacheBlock() {}
	cacheBlock(unsigned tag, unsigned valid, unsigned content) {
		this->tag = tag;
		this->valid = valid;
		this->content = content;
	}
};

static unsigned iPageTableEntries = 1024 / iMemoryPageSize;
static unsigned dPageTableEntries = 1024 / dMemoryPageSize;
static unsigned iTLBEntries = iPageTableEntries / 4;
static unsigned dTLBEntries = dPageTableEntries / 4;
static unsigned iCacheLength = totalSizeOfICache / blockSizeOfICache / setAssOfICache;
static unsigned dCacheLength = totalSizeOfDCache / blockSizeOfDCache / setAssOfDCache;

vector<tlb> iTLB;
vector<tlb> dTLB;
pte *iPTE;
pte *dPTE;
vector<cacheBlock> *iCache;
vector<cacheBlock> *dCache;
char *iMemory;
char *dMemory;

void initTLB() {
	for (unsigned i = 0; i < iTLBEntries; i++)
		iTLB.push_back(tlb(0, 0, 0, 0));
	for (unsigned i = 0; i < dTLBEntries; i++)
		dTLB.push_back(tlb(0, 0, 0, 0));
}

void initPTE() {
	iPTE = new pte[iPageTableEntries];
	for (unsigned i = 0; i < iPageTableEntries; i++) {
		iPTE[i].valid = 0;
		iPTE[i].pPageNumber = 0;
	}
	dPTE = new pte[dPageTableEntries];
	for (unsigned i = 0; i < iPageTableEntries; i++) {
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
	iMemory = new char[iMemorySize];
	for (unsigned i = 0; i < iMemorySize; i++)
		iMemory[i] = 0;
	dMemory = new char[dMemorySize];
	for (unsigned i = 0; i < dMemorySize; i++)
		dMemory[i] = 0;
}

bool checkITLBHit() {
	
}
