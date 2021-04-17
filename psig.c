// psig.c ... functions on page signatures (psig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "psig.h"
#include "hash.h"


// make a tuple signature for SIMC
Bits codeword1(char *attr_value, int m, int k) {
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
Bits codeword1_c(char *attr_value, int m, int u, int k) {
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


Bits makePageSig(Reln r, Tuple t) {
    assert(r != NULL && t != NULL);
    //TODO
    char **attr = tupleVals(r, t);
    Bits cw = NULL;
    Bits temp = NULL;
    switch (sigType(r)) {
        case 'c': // CATC
            cw = newBits(psigBits(r));
            int u = psigBits(r) / nAttrs(r);
            int first = u + psigBits(r) % nAttrs(r);
            // use floor unless it's 0
            int k = (u / 2 / maxTupsPP(r)) ? (u / 2 / maxTupsPP(r)) : 1;
            for (int i = 0; i < nAttrs(r); ++i) {
                if (attr[i][0] == '?' || attr[i][0] == '\0') // ignore the ? attr
                    continue;
                if (i == 0)
                    temp = codeword1_c(attr[i], psigBits(r), first, k);
                else {
                    temp = codeword1_c(attr[i], psigBits(r), u, k);
                    shiftBits(temp, first + (i - 1) * u);
                }
                orBits(cw, temp);
                freeBits(temp); // release memory
            }
            break;
        case 's': // SIMC
            cw = newBits(psigBits(r));
            for (int i = 0; i < nAttrs(r); ++i) {
                if (attr[i][0] == '?' || attr[i][0] == '\0') // ignore the ? attr
                    continue;
                temp = codeword1(attr[i], psigBits(r),codeBits(r));
                orBits(cw, temp);
                freeBits(temp); // release memory
            }
            break;
        default:
            break;
    }
    freeVals(attr, nAttrs(r));
    return cw;
}

void findPagesUsingPageSigs(Query q) {
    assert(q != NULL);
    //TODO
    Bits querySig = makePageSig(q->rel, q->qstring);
    unsetAllBits(q->pages);
    PageID pid = 0;
    for (PageID i = 0; i < nPsigPages(q->rel); ++i) {
        Page p = getPage(q->rel->psigf, i);
        for (Offset j = 0; j < pageNitems(p); ++j) {
            Bits psig = newBits(psigBits(q->rel));
            // get correct pos
            getBits(p, j * (psigBits(q->rel) / 8), psig);
            // check if sig match
            if (isSubset(querySig, psig)) {
                pid = q->nsigs;
                setBit(q->pages, pid);
            }
            q->nsigs++;
            freeBits(psig);
        }
        q->nsigpages++;
        free(p);
    }
//    printf("Matched Pages:");
//    showBits(q->pages);
//    putchar('\n');
}

