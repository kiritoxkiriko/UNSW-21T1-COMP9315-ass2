// tsig.c ... functions on Tuple Signatures (tsig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "tsig.h"
#include "reln.h"
#include "hash.h"
#include "bits.h"

// calculate corresponding PID
PageID calPid(Reln r, Count nsigs){
    return nsigs/ maxTupsPP(r);
}

// make a tuple signature for SIMC
Bits codeword(char *attr_value, int m, int k) {
    int nbits = 0; // count of set bits
    Bits cword = newBits(m); // assuming m <= 32 bits
    srandom(hash_any(attr_value, strlen(attr_value)));
    while (nbits < k) {
        int i = (int) random() % m;
        if (!bitIsSet(cword, i)) {
            setBit(cword, i);
            nbits++;
        }
    }
    return cword;
// m-bits with k 1-bits and m-k 0-bits
}

// make a tuple signature for CATC
Bits codeword_c(char *attr_value, int m, int u, int k) {
    int nbits = 0; // count of set bits
    Bits cword = newBits(m); // assuming m <= 32 bits
    srandom(hash_any(attr_value, strlen(attr_value)));
    while (nbits < k) {
        int i = (int) random() % u;
        if (!bitIsSet(cword, i)) {
            setBit(cword, i);
            nbits++;
        }
    }
    return cword;
// m-bits with k 1-bits and m-k 0-bits with in u-bits long
}

Bits makeTupleSig(Reln r, Tuple t) {
    assert(r != NULL && t != NULL);
    //TODO
    char **attr = tupleVals(r, t);
    Bits cw = NULL;
    Bits temp = NULL;
    switch (sigType(r)) {
        case 'c': // CATC
            cw = newBits(tsigBits(r));
            int u = tsigBits(r) / nAttrs(r);
            int first = u+ tsigBits(r) % nAttrs(r);
            int k = (u / 2) ? (u / 2) : 1;
            for (int i = 0; i < nAttrs(r); ++i) {
                if (attr[i][0] == '?' || attr[i][0] == '\0') // ignore the ? attr
                    continue;
                if (i == 0)
                    temp = codeword_c(attr[i], tsigBits(r), first, k);
                else{
                    temp = codeword_c(attr[i], tsigBits(r), u, k);
                    shiftBits(temp, first + (i-1)*u);
                }
                orBits(cw, temp);
                freeBits(temp); // release memory
            }
            break;
        case 's': // SIMC
            cw = newBits(tsigBits(r));
            for (int i = 0; i < nAttrs(r); ++i) {
                if (attr[i][0] == '?' || attr[i][0] == '\0') // ignore the ? attr
                    continue;
                temp = codeword(attr[i], tsigBits(r),codeBits(r));
                orBits(cw, temp);
                freeBits(temp); // release memory
            }
            break;
        default:break;
    }
    freeVals(attr, nAttrs(r));
    return cw;
}

// find "matching" pages using tuple signatures

void findPagesUsingTupSigs(Query q) {
    assert(q != NULL);
    //TODO

    Bits querySig = makeTupleSig(q->rel,q->qstring);
    unsetAllBits(q->pages);
    for (PageID i = 0; i < nTsigPages(q->rel); ++i) {
        Page p = getPage(q->rel->tsigf,i);
        for (Offset j = 0; j < pageNitems(p); ++j) {
            Bits tsig = newBits(tsigBits(q->rel));
            // get correct pos
            getBits(p, j* (tsigBits(q->rel)/8),tsig);
            // check if sig match
            if (isSubset(querySig,tsig)){
                PageID pid = calPid(q->rel,q->nsigs);
                setBit(q->pages, pid);
            }
            q->nsigs++;
            freeBits(tsig);
        }
        q->nsigpages++;
        free(p);
    }
    // The printf below is primarily for debugging
    // Remove it before submitting this function
//    printf("Matched Pages:");
//    showBits(q->pages);
//    putchar('\n');
}