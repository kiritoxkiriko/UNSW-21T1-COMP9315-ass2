// bsig.c ... functions on Tuple Signatures (bsig's)
// part of signature indexed files
// Written by John Shepherd, March 2019

#include "defs.h"
#include "reln.h"
#include "query.h"
#include "bsig.h"
#include "psig.h"

void findPagesUsingBitSlices(Query q) {
    assert(q != NULL);
    //TODO

    int bsigPid = -1;
    int temp = 0;
    Offset offset = 0;
    Bits qsig = makePageSig(q->rel, q->qstring);
    Bits bs = newBits(nPages(q->rel));
    Page p = getPage(q->rel->bsigf, 0);
    setAllBits(q->pages);
    for (int i = 0; i < psigBits(q->rel); ++i) {
        if (bitIsSet(qsig, i)) {
            temp = i / maxBsigsPP(q->rel);
            if (temp > bsigPid) {
                q->nsigpages++;
                bsigPid = temp;
                free(p);
                p = getPage(q->rel->bsigf, bsigPid);
            }
            offset = i % maxBsigsPP(q->rel);
            Page p = getPage(q->rel->bsigf, bsigPid);
            getBits(p, offset * (bsigBits(q->rel) / 8), bs);
            andBits(q->pages, bs);
            q->nsigs++;
        }
    }
    free(p);
    freeBits(bs);
}

