#include "global.h"
#include "output.h"

void dumpSnap() {
    fprintf(snap, "cycle %u\n", cycle++);
    unsigned i;
    for (i = 0; i < 32; ++i) {
        fprintf(snap, "$%02u: 0x", i);
        fprintf(snap, "%08X\n", reg[i]);
    }
    fprintf(snap, "PC: 0x");
    fprintf(snap, "%08X\n\n\n", PC);
}

void errorDump() {
    if (writeToZero) {
        writeToZero = 0;
        fprintf(err, "In cycle %d: Write $0 Error\n", cycle);
    }
    if (numberOverflow) {
        numberOverflow = 0;
        fprintf(err, "In cycle %d: Number Overflow\n", cycle);
    }
    if (memoryOverflow) {
        memoryOverflow = 0;
        fprintf(err, "In cycle %d: Address Overflow\n", cycle);
    }
    if (memoryMisalignment) {
        memoryMisalignment = 0;
        fprintf(err, "In cycle %d: Misalignment Error\n", cycle);
    }
}

void reportDump() {
    fprintf(report, "ICache :\n");
    fprintf(report, "# hits: %u\n", iCacheHit);
    fprintf(report, "# misses: %u\n", iCacheMiss);
    fprintf(report, "\n");
    fprintf(report, "DCache :\n");
    fprintf(report, "# hits: %u\n", dCacheHit);
    fprintf(report, "# misses: %u\n", dCacheMiss);
    fprintf(report, "\n");
    fprintf(report, "ITLB :\n");
    fprintf(report, "# hits: %u\n", iTLBHit);
    fprintf(report, "# misses: %u\n", iTLBMiss);
    fprintf(report, "\n");
    fprintf(report, "DTLB :\n");
    fprintf(report, "# hits: %u\n", dTLBHit);
    fprintf(report, "# misses: %u\n", dTLBMiss);
    fprintf(report, "\n");
    fprintf(report, "IPageTable :\n");
    fprintf(report, "# hits: %u\n", iPageTableHit);
    fprintf(report, "# misses: %u\n", iPageTableMiss);
    fprintf(report, "\n");
    fprintf(report, "DPageTable :\n");
    fprintf(report, "# hits: %u\n", dPageTableHit);
    fprintf(report, "# misses: %u\n", dPageTableMiss);
    fprintf(report, "\n");
}
