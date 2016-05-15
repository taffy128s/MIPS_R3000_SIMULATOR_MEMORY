#include "global.h"
#include "cmp.h"

static unsigned iPageTableEntries = 1024 / iMemoryPageSize;
static unsigned dPageTableEntries = 1024 / dMemoryPageSize;
static unsigned iTLBEntries = iPageTableEntries / 4;
static unsigned dTLBEntries = dPageTableEntries / 4;