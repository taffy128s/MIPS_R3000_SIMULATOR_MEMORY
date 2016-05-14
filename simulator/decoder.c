#include "global.h"
#include "decoder.h"

void findRsRtRd(unsigned *rs, unsigned *rt, unsigned *rd) {
    // Deal with rs.
    unsigned temp1 = iDisk[PC], temp2 = iDisk[PC + 1];
    temp1 = temp1 << 30 >> 27;
    temp2 = temp2 << 24 >> 29;
    *rs = temp1 + temp2;
    // Deal with rt.
    *rt = iDisk[PC + 1];
    *rt = (*rt) << 27 >> 27;
    // Deal with rd.
    if (rd == NULL) return;
    *rd = iDisk[PC + 2];
    *rd = (*rd) << 24 >> 27;
}

void findShamt(unsigned *shamt) {
    unsigned temp1 = iDisk[PC + 2], temp2 = iDisk[PC + 3];
    temp1 = temp1 << 29 >> 27;
    temp2 = temp2 >> 6 << 30 >> 30;
    *shamt = temp1 + temp2;
}

void findUnsignedImmediate(unsigned *immediate) {
    unsigned temp1 = iDisk[PC + 2], temp2 = iDisk[PC + 3];
    temp1 = temp1 << 24 >> 16;
    temp2 = temp2 << 24 >> 24;
    *immediate = temp1 + temp2;
}

void findSignedImmediate(unsigned *immediate) {
    unsigned temp1 = iDisk[PC + 2], temp2 = iDisk[PC + 3];
    temp1 = temp1 << 24 >> 16;
    temp2 = temp2 << 24 >> 24;
    int temp = temp1 + temp2;
    temp = temp << 16 >> 16;
    *immediate = temp;
}
