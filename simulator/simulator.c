/***
Name: MIPS_R3000_SIMULATOR_MEMORY
Author: Taffy Cheng
Date: Ongoing
***/

#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "defines.h"
#include "decoder.h"
#include "output.h"
#include "load.h"
#include "cmp.h"

static unsigned opcode, funct, shamt, address, temp1, temp2, temp3, temp4, rs, rt, rd;
static unsigned immediate, signRs, signRt, signRd, signIm, signPos, pos;
static int intRs, intRt, intIm, temp;

void findOpcode() {
    int chkITLBHit = checkITLBHit(PC);
    if (chkITLBHit == 0) { // iTLB misses.
        int chkIPTEHit = checkIPTEHit(PC);
        if (chkIPTEHit == 0) { // iPTE misses.
            unsigned iMemoryReplaceIdx = findIMemoryReplaceIdx();
            swapIMemory(PC, iMemoryReplaceIdx);
            updateIPTE(PC, iMemoryReplaceIdx);
            updateITLBWhenPageTableMiss(PC, iMemoryReplaceIdx);
            // search cache (in fact, cache must miss)
            int chkICacheHit = checkICacheHit(iMemoryReplaceIdx);
            if (chkICacheHit == 0) {
                updateICache(iMemoryReplaceIdx);
            } else printf("Error, it's impossible to have miss miss hit(iCache).\n");
        } else {
            updateITLBWhenPageTableHit(PC);
            // search cache
            unsigned iMemoryIdx = iPTEValidPPN * iMemoryPageSize;
            iMemoryIdx = iMemoryIdx | (PC % iMemoryPageSize);
            int chkICacheHit = checkICacheHit(iMemoryIdx);
            if (chkICacheHit == 0)
                updateICache(iMemoryIdx);
        }
    } else {
        // search cache
        unsigned iMemoryIdx = iTLBValidSet * iMemoryPageSize;
        iMemoryIdx = iMemoryIdx | (PC % iMemoryPageSize);
        int chkICacheHit = checkICacheHit(iMemoryIdx);
        if (chkICacheHit == 0)
            updateICache(iMemoryIdx);
    }
    opcode = iRun[PC];
    opcode = opcode >> 2 << 26 >> 26;
}

int findPosByImmediateWithErrorDetection(unsigned gap) {
    int needToHalt = 0;
    signRs = reg[rs] >> 31, signIm = immediate >> 31;
    pos = reg[rs] + immediate;
    signPos = pos >> 31;
    if (signRs == signIm && signRs != signPos)
        numberOverflow = 1;
    if (pos >= 1024 || pos + gap >= 1024) {
        memoryOverflow = 1;
        needToHalt = 1;
    }
    if (pos % (gap + 1)) {
        memoryMisalignment = 1;
        needToHalt = 1;
    }
    if (needToHalt) return 1;
    else return 0;
}

void run() {
    while (1) {
        dumpSnap();
        findOpcode();
        switch (opcode) {
            case HALT: return;
            case J: {
                temp1 = iRun[PC];
                temp2 = iRun[PC + 1];
                temp3 = iRun[PC + 2];
                temp4 = iRun[PC + 3];
                temp1 = temp1 << 30 >> 6;
                temp2 = temp2 << 24 >> 8;
                temp3 = temp3 << 24 >> 16;
                temp4 = temp4 << 24 >> 24;
                address = temp1 + temp2 + temp3 + temp4;
                PC = ((PC + 4) >> 28 << 28) | (address << 2);
                break;
            }
            case JAL: {
                reg[31] = PC + 4;
                temp1 = iRun[PC];
                temp2 = iRun[PC + 1];
                temp3 = iRun[PC + 2];
                temp4 = iRun[PC + 3];
                temp1 = temp1 << 30 >> 6;
                temp2 = temp2 << 24 >> 8;
                temp3 = temp3 << 24 >> 16;
                temp4 = temp4 << 24 >> 24;
                address = temp1 + temp2 + temp3 + temp4;
                PC = ((PC + 4) >> 28 << 28) | (address << 2);
                break;
            }
            case R: {
                funct = iRun[PC + 3];
                funct = funct << 26 >> 26;
                findRsRtRd(&rs, &rt, &rd);
                switch (funct) {
                    case ADD: {
                        signRs = reg[rs] >> 31, signRt = reg[rt] >> 31;
                        reg[rd] = reg[rs] + reg[rt];
                        signRd = reg[rd] >> 31;
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        if (signRs == signRt && signRs != signRd)
                            numberOverflow = 1;
                        PC += 4;
                        break;
                    }
                    case ADDU: {
                        reg[rd] = reg[rs] + reg[rt];
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case SUB: {
                        signRs = reg[rs] >> 31, signRt = (-reg[rt]) >> 31;
                        reg[rd] = reg[rs] - reg[rt];
                        signRd = reg[rd] >> 31;
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        if (signRs == signRt && signRs != signRd)
                            numberOverflow = 1;
                        PC += 4;
                        break;
                    }
                    case AND: {
                        reg[rd] = reg[rs] & reg[rt];
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case OR: {
                        reg[rd] = reg[rs] | reg[rt];
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case XOR: {
                        reg[rd] = reg[rs] ^ reg[rt];
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case NOR: {
                        reg[rd] = ~(reg[rs] | reg[rt]);
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case NAND: {
                        reg[rd] = ~(reg[rs] & reg[rt]);
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case SLT: {
                        intRs = reg[rs], intRt = reg[rt];
                        reg[rd] = (intRs < intRt) ? 1 : 0;
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case SLL: {
                        findShamt(&shamt);
                        reg[rd] = reg[rt] << shamt;
                        if (rd == 0) {
                            if (!(rt == 0 && shamt == 0)) {
                                writeToZero = 1;
                                reg[rd] = 0;
                            }
                        }
                        PC += 4;
                        break;
                    }
                    case SRL: {
                        findShamt(&shamt);
                        reg[rd] = reg[rt] >> shamt;
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case SRA: {
                        findShamt(&shamt);
                        temp = reg[rt];
                        temp = temp >> shamt;
                        reg[rd] = temp;
                        if (rd == 0) {
                            writeToZero = 1;
                            reg[rd] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    default: PC = reg[rs];
                }
                break;
            }
            default: {
                findRsRtRd(&rs, &rt, NULL);
                switch (opcode) {
                    case ADDI: {
                        findSignedImmediate(&immediate);
                        signRs = reg[rs] >> 31, signIm = immediate >> 31;
                        reg[rt] = reg[rs] + immediate;
                        signRt = reg[rt] >> 31;
                        if (rt == 0) {
                            writeToZero = 1;
                            reg[rt] = 0;
                        }
                        if (signRs == signIm && signRs != signRt)
                            numberOverflow = 1;
                        PC += 4;
                        break;
                    }
                    case ADDIU: {
                        findSignedImmediate(&immediate);
                        reg[rt] = reg[rs] + immediate;
                        if (rt == 0) {
                            writeToZero = 1;
                            reg[rt] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case LW: {
                        if (rt == 0)
                            writeToZero = 1;
                        findSignedImmediate(&immediate);
                        if (findPosByImmediateWithErrorDetection(3)) return;
                        // Misalignment detection is embedded in the findPos function.
                        int chkDTLBHit = checkDTLBHit(pos);
                        if (chkDTLBHit == 0) { // iTLB misses.
                            int chkDPTEHit = checkDPTEHit(pos);
                            if (chkDPTEHit == 0) { // iPTE misses.
                                unsigned dMemoryReplaceIdx = findDMemoryReplaceIdx();
                                swapDMemory(pos, dMemoryReplaceIdx);
                                updateDPTE(pos, dMemoryReplaceIdx);
                                updateDTLBWhenPageTableMiss(pos, dMemoryReplaceIdx);
                                // search cache (in fact, cache must miss)
                                int chkDCacheHit = checkDCacheHit(dMemoryReplaceIdx);
                                if (chkDCacheHit == 0) {
                                    updateDCache(dMemoryReplaceIdx);
                                } else printf("Error, it's impossible to have miss miss hit(dCache).\n");
                            } else {
                                updateDTLBWhenPageTableHit(pos);
                                // search cache
                                unsigned dMemoryIdx = dPTEValidPPN * dMemoryPageSize;
                                dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                                int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                                if (chkDCacheHit == 0)
                                    updateDCache(dMemoryIdx);
                            }
                        } else {
                            // search cache
                            unsigned dMemoryIdx = dTLBValidSet * dMemoryPageSize;
                            dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                            int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                            if (chkDCacheHit == 0)
                                updateDCache(dMemoryIdx);
                        }
                        // ----- //
                        temp1 = dRun[pos], temp1 = temp1 << 24;
                        temp2 = dRun[pos + 1], temp2 = temp2 << 24 >> 8;
                        temp3 = dRun[pos + 2], temp3 = temp3 << 24 >> 16;
                        temp4 = dRun[pos + 3], temp4 = temp4 << 24 >> 24;
                        reg[rt] = temp1 + temp2 + temp3 + temp4;
                        if (rt == 0)
                            reg[rt] = 0;
                        PC += 4;
                        break;
                    }
                    case LH: {
                        if (rt == 0)
                            writeToZero = 1;
                        findSignedImmediate(&immediate);
                        if (findPosByImmediateWithErrorDetection(1)) return;
                        // Misalignment detection is embedded in the findPos function.
                        int chkDTLBHit = checkDTLBHit(pos);
                        if (chkDTLBHit == 0) { // iTLB misses.
                            int chkDPTEHit = checkDPTEHit(pos);
                            if (chkDPTEHit == 0) { // iPTE misses.
                                unsigned dMemoryReplaceIdx = findDMemoryReplaceIdx();
                                swapDMemory(pos, dMemoryReplaceIdx);
                                updateDPTE(pos, dMemoryReplaceIdx);
                                updateDTLBWhenPageTableMiss(pos, dMemoryReplaceIdx);
                                // search cache (in fact, cache must miss)
                                int chkDCacheHit = checkDCacheHit(dMemoryReplaceIdx);
                                if (chkDCacheHit == 0) {
                                    updateDCache(dMemoryReplaceIdx);
                                } else printf("Error, it's impossible to have miss miss hit(dCache).\n");
                            } else {
                                updateDTLBWhenPageTableHit(pos);
                                // search cache
                                unsigned dMemoryIdx = dPTEValidPPN * dMemoryPageSize;
                                dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                                int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                                if (chkDCacheHit == 0)
                                    updateDCache(dMemoryIdx);
                            }
                        } else {
                            // search cache
                            unsigned dMemoryIdx = dTLBValidSet * dMemoryPageSize;
                            dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                            int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                            if (chkDCacheHit == 0)
                                updateDCache(dMemoryIdx);
                        }
                        // ----- //
                        temp1 = dRun[pos], temp1 = temp1 << 24 >> 16;
                        temp2 = dRun[pos + 1], temp2 = temp2 << 24 >> 24;
                        short shortTemp = temp1 + temp2;
                        reg[rt] = shortTemp;
                        if (rt == 0)
                            reg[rt] = 0;
                        PC += 4;
                        break;
                    }
                    case LHU: {
                        if (rt == 0)
                            writeToZero = 1;
                        findSignedImmediate(&immediate);
                        if (findPosByImmediateWithErrorDetection(1)) return;
                        // Misalignment detection is embedded in the findPos function.
                        int chkDTLBHit = checkDTLBHit(pos);
                        if (chkDTLBHit == 0) { // iTLB misses.
                            int chkDPTEHit = checkDPTEHit(pos);
                            if (chkDPTEHit == 0) { // iPTE misses.
                                unsigned dMemoryReplaceIdx = findDMemoryReplaceIdx();
                                swapDMemory(pos, dMemoryReplaceIdx);
                                updateDPTE(pos, dMemoryReplaceIdx);
                                updateDTLBWhenPageTableMiss(pos, dMemoryReplaceIdx);
                                // search cache (in fact, cache must miss)
                                int chkDCacheHit = checkDCacheHit(dMemoryReplaceIdx);
                                if (chkDCacheHit == 0) {
                                    updateDCache(dMemoryReplaceIdx);
                                } else printf("Error, it's impossible to have miss miss hit(dCache).\n");
                            } else {
                                updateDTLBWhenPageTableHit(pos);
                                // search cache
                                unsigned dMemoryIdx = dPTEValidPPN * dMemoryPageSize;
                                dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                                int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                                if (chkDCacheHit == 0)
                                    updateDCache(dMemoryIdx);
                            }
                        } else {
                            // search cache
                            unsigned dMemoryIdx = dTLBValidSet * dMemoryPageSize;
                            dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                            int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                            if (chkDCacheHit == 0)
                                updateDCache(dMemoryIdx);
                        }
                        // ----- //
                        temp1 = dRun[pos], temp1 = temp1 << 24 >> 16;
                        temp2 = dRun[pos + 1], temp2 = temp2 << 24 >> 24;
                        reg[rt] = temp1 + temp2;
                        if (rt == 0)
                            reg[rt] = 0;
                        PC += 4;
                        break;
                    }
                    case LB: {
                        if (rt == 0)
                            writeToZero = 1;
                        findSignedImmediate(&immediate);
                        if (findPosByImmediateWithErrorDetection(0)) return;
                        // No need to detect misalignment.
                        int chkDTLBHit = checkDTLBHit(pos);
                        if (chkDTLBHit == 0) { // iTLB misses.
                            int chkDPTEHit = checkDPTEHit(pos);
                            if (chkDPTEHit == 0) { // iPTE misses.
                                unsigned dMemoryReplaceIdx = findDMemoryReplaceIdx();
                                swapDMemory(pos, dMemoryReplaceIdx);
                                updateDPTE(pos, dMemoryReplaceIdx);
                                updateDTLBWhenPageTableMiss(pos, dMemoryReplaceIdx);
                                // search cache (in fact, cache must miss)
                                int chkDCacheHit = checkDCacheHit(dMemoryReplaceIdx);
                                if (chkDCacheHit == 0) {
                                    updateDCache(dMemoryReplaceIdx);
                                } else printf("Error, it's impossible to have miss miss hit(dCache).\n");
                            } else {
                                updateDTLBWhenPageTableHit(pos);
                                // search cache
                                unsigned dMemoryIdx = dPTEValidPPN * dMemoryPageSize;
                                dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                                int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                                if (chkDCacheHit == 0)
                                    updateDCache(dMemoryIdx);
                            }
                        } else {
                            // search cache
                            unsigned dMemoryIdx = dTLBValidSet * dMemoryPageSize;
                            dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                            int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                            if (chkDCacheHit == 0)
                                updateDCache(dMemoryIdx);
                        }
                        // ----- //
                        reg[rt] = dRun[pos];
                        if (rt == 0)
                            reg[rt] = 0;
                        PC += 4;
                        break;
                    }
                    case LBU: {
                        if (rt == 0)
                            writeToZero = 1;
                        findSignedImmediate(&immediate);
                        if (findPosByImmediateWithErrorDetection(0)) return;
                        // No need to detect misalignment.
                        int chkDTLBHit = checkDTLBHit(pos);
                        if (chkDTLBHit == 0) { // iTLB misses.
                            int chkDPTEHit = checkDPTEHit(pos);
                            if (chkDPTEHit == 0) { // iPTE misses.
                                unsigned dMemoryReplaceIdx = findDMemoryReplaceIdx();
                                swapDMemory(pos, dMemoryReplaceIdx);
                                updateDPTE(pos, dMemoryReplaceIdx);
                                updateDTLBWhenPageTableMiss(pos, dMemoryReplaceIdx);
                                // search cache (in fact, cache must miss)
                                int chkDCacheHit = checkDCacheHit(dMemoryReplaceIdx);
                                if (chkDCacheHit == 0) {
                                    updateDCache(dMemoryReplaceIdx);
                                } else printf("Error, it's impossible to have miss miss hit(dCache).\n");
                            } else {
                                updateDTLBWhenPageTableHit(pos);
                                // search cache
                                unsigned dMemoryIdx = dPTEValidPPN * dMemoryPageSize;
                                dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                                int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                                if (chkDCacheHit == 0)
                                    updateDCache(dMemoryIdx);
                            }
                        } else {
                            // search cache
                            unsigned dMemoryIdx = dTLBValidSet * dMemoryPageSize;
                            dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                            int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                            if (chkDCacheHit == 0)
                                updateDCache(dMemoryIdx);
                        }
                        // ----- //
                        reg[rt] = dRun[pos], reg[rt] = reg[rt] << 24 >> 24;
                        if (rt == 0)
                            reg[rt] = 0;
                        PC += 4;
                        break;
                    }
                    case SW: {
                        findSignedImmediate(&immediate);
                        if (findPosByImmediateWithErrorDetection(3)) return;
                        // Misalignment detection is embedded in the findPos function.
                        int chkDTLBHit = checkDTLBHit(pos);
                        if (chkDTLBHit == 0) { // iTLB misses.
                            int chkDPTEHit = checkDPTEHit(pos);
                            if (chkDPTEHit == 0) { // iPTE misses.
                                unsigned dMemoryReplaceIdx = findDMemoryReplaceIdx();
                                swapDMemory(pos, dMemoryReplaceIdx);
                                updateDPTE(pos, dMemoryReplaceIdx);
                                updateDTLBWhenPageTableMiss(pos, dMemoryReplaceIdx);
                                // search cache (in fact, cache must miss)
                                int chkDCacheHit = checkDCacheHit(dMemoryReplaceIdx);
                                if (chkDCacheHit == 0) {
                                    updateDCache(dMemoryReplaceIdx);
                                } else printf("Error, it's impossible to have miss miss hit(dCache).\n");
                            } else {
                                updateDTLBWhenPageTableHit(pos);
                                // search cache
                                unsigned dMemoryIdx = dPTEValidPPN * dMemoryPageSize;
                                dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                                int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                                if (chkDCacheHit == 0)
                                    updateDCache(dMemoryIdx);
                            }
                        } else {
                            // search cache
                            unsigned dMemoryIdx = dTLBValidSet * dMemoryPageSize;
                            dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                            int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                            if (chkDCacheHit == 0)
                                updateDCache(dMemoryIdx);
                        }
                        // ----- // TODO: write to cache
                        dRun[pos] = reg[rt] >> 24;
                        dRun[pos + 1] = reg[rt] << 8 >> 24;
                        dRun[pos + 2] = reg[rt] << 16 >> 24;
                        dRun[pos + 3] = reg[rt] << 24 >> 24;
                        PC += 4;
                        break;
                    }
                    case SH: {
                        findSignedImmediate(&immediate);
                        if (findPosByImmediateWithErrorDetection(1)) return;
                        // Misalignment detection is embedded in the findPos function.
                        int chkDTLBHit = checkDTLBHit(pos);
                        if (chkDTLBHit == 0) { // iTLB misses.
                            int chkDPTEHit = checkDPTEHit(pos);
                            if (chkDPTEHit == 0) { // iPTE misses.
                                unsigned dMemoryReplaceIdx = findDMemoryReplaceIdx();
                                swapDMemory(pos, dMemoryReplaceIdx);
                                updateDPTE(pos, dMemoryReplaceIdx);
                                updateDTLBWhenPageTableMiss(pos, dMemoryReplaceIdx);
                                // search cache (in fact, cache must miss)
                                int chkDCacheHit = checkDCacheHit(dMemoryReplaceIdx);
                                if (chkDCacheHit == 0) {
                                    updateDCache(dMemoryReplaceIdx);
                                } else printf("Error, it's impossible to have miss miss hit(dCache).\n");
                            } else {
                                updateDTLBWhenPageTableHit(pos);
                                // search cache
                                unsigned dMemoryIdx = dPTEValidPPN * dMemoryPageSize;
                                dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                                int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                                if (chkDCacheHit == 0)
                                    updateDCache(dMemoryIdx);
                            }
                        } else {
                            // search cache
                            unsigned dMemoryIdx = dTLBValidSet * dMemoryPageSize;
                            dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                            int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                            if (chkDCacheHit == 0)
                                updateDCache(dMemoryIdx);
                        }
                        // ----- // TODO: write to cache
                        dRun[pos] = reg[rt] << 16 >> 24;
                        dRun[pos + 1] = reg[rt] << 24 >> 24;
                        PC += 4;
                        break;
                    }
                    case SB: {
                        findSignedImmediate(&immediate);
                        if (findPosByImmediateWithErrorDetection(0)) return;
                        // No need to detect misalignment.
                        int chkDTLBHit = checkDTLBHit(pos);
                        if (chkDTLBHit == 0) { // iTLB misses.
                            int chkDPTEHit = checkDPTEHit(pos);
                            if (chkDPTEHit == 0) { // iPTE misses.
                                unsigned dMemoryReplaceIdx = findDMemoryReplaceIdx();
                                swapDMemory(pos, dMemoryReplaceIdx);
                                updateDPTE(pos, dMemoryReplaceIdx);
                                updateDTLBWhenPageTableMiss(pos, dMemoryReplaceIdx);
                                // search cache (in fact, cache must miss)
                                int chkDCacheHit = checkDCacheHit(dMemoryReplaceIdx);
                                if (chkDCacheHit == 0) {
                                    updateDCache(dMemoryReplaceIdx);
                                } else printf("Error, it's impossible to have miss miss hit(dCache).\n");
                            } else {
                                updateDTLBWhenPageTableHit(pos);
                                // search cache
                                unsigned dMemoryIdx = dPTEValidPPN * dMemoryPageSize;
                                dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                                int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                                if (chkDCacheHit == 0)
                                    updateDCache(dMemoryIdx);
                            }
                        } else {
                            // search cache
                            unsigned dMemoryIdx = dTLBValidSet * dMemoryPageSize;
                            dMemoryIdx = dMemoryIdx | (pos % dMemoryPageSize);
                            int chkDCacheHit = checkDCacheHit(dMemoryIdx);
                            if (chkDCacheHit == 0)
                                updateDCache(dMemoryIdx);
                        }
                        // ----- // TODO: write to cache
                        dRun[pos] = reg[rt] << 24 >> 24;
                        PC += 4;
                        break;
                    }
                    case LUI: {
                        findUnsignedImmediate(&immediate);
                        reg[rt] = immediate << 16;
                        if (rt == 0) {
                            writeToZero = 1;
                            reg[rt] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case ANDI: {
                        findUnsignedImmediate(&immediate);
                        reg[rt] = reg[rs] & immediate;
                        if (rt == 0) {
                            writeToZero = 1;
                            reg[rt] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case ORI: {
                        findUnsignedImmediate(&immediate);
                        reg[rt] = reg[rs] | immediate;
                        if (rt == 0) {
                            writeToZero = 1;
                            reg[rt] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case NORI: {
                        findUnsignedImmediate(&immediate);
                        reg[rt] = ~(reg[rs] | immediate);
                        if (rt == 0) {
                            writeToZero = 1;
                            reg[rt] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case SLTI: {
                        findSignedImmediate(&immediate);
                        intIm = immediate, intRs = reg[rs];
                        reg[rt] = (intRs < intIm) ? 1 : 0;
                        if (rt == 0) {
                            writeToZero = 1;
                            reg[rt] = 0;
                        }
                        PC += 4;
                        break;
                    }
                    case BEQ: {
                        findSignedImmediate(&immediate);
                        if (reg[rs] == reg[rt]) {
                            immediate = immediate << 2;
                            PC = PC + 4 + immediate;
                        } else PC += 4;
                        break;
                    }
                    case BNE: {
                        findSignedImmediate(&immediate);
                        if (reg[rs] != reg[rt]) {
                            immediate = immediate << 2;
                            PC = PC + 4 + immediate;
                        } else PC += 4;
                        break;
                    }
                    default: {
                        findSignedImmediate(&immediate);
                        int temp = reg[rs];
                        if (temp > 0) {
                            immediate = immediate << 2;
                            PC = PC + 4 + immediate;
                        } else PC += 4;
                    }
                }
            }
        }
        errorDump();
    }
}

int main(int argc, char **argv) {
    if (argc == 1) {
        iMemorySize = 64;
        dMemorySize = 32;
        iMemoryPageSize = 8;
        dMemoryPageSize = 16;
        totalSizeOfICache = 16;
        blockSizeOfICache = 4;
        setAssOfICache = 4;
        totalSizeOfDCache = 16;
        blockSizeOfDCache = 4;
        setAssOfDCache = 1;
    } else if (argc == 11) {
        iMemorySize = atoi(argv[1]);
        dMemorySize = atoi(argv[2]);
        iMemoryPageSize = atoi(argv[3]);
        dMemoryPageSize = atoi(argv[4]);
        totalSizeOfICache = atoi(argv[5]);
        blockSizeOfICache = atoi(argv[6]);
        setAssOfICache = atoi(argv[7]);
        totalSizeOfDCache = atoi(argv[8]);
        blockSizeOfDCache = atoi(argv[9]);
        setAssOfDCache = atoi(argv[10]);
    } else {
        printf("Wrong input format.\n");
        exit(0);
    }

    iCacheLength = totalSizeOfICache / blockSizeOfICache / setAssOfICache;
    dCacheLength = totalSizeOfDCache / blockSizeOfDCache / setAssOfDCache;
    iPageTableEntries = 1024 / iMemoryPageSize;
    dPageTableEntries = 1024 / dMemoryPageSize;
    iTLBEntries = iPageTableEntries / 4;
    dTLBEntries = dPageTableEntries / 4;

    initTLB();
    initPTE();
    initCache();
    initMemory();

    openNLoadFiles();
    dealWithDImg();
    dealWithIImg();
    run();
    // Last return may be an error, so it's necessary to run errorDump() again.
    errorDump();
    reportDump();
    /*
    printf("iTLBHit: %u, iPageTableHit: %u, iCacheHit: %u\n", iTLBHit, iPageTableHit, iCacheHit);
    printf("iTLBMiss: %u, iPageTableMiss: %u, iCacheMiss: %u\n", iTLBMiss, iPageTableMiss, iCacheMiss);
    printf("dTLBHit: %u, dPageTableHit: %u, dCacheHit: %u\n", dTLBHit, dPageTableHit, dCacheHit);
    printf("dTLBMiss: %u, dPageTableMiss: %u, dCacheMiss: %u\n", dTLBMiss, dPageTableMiss, dCacheMiss);
    */
    return 0;
}
