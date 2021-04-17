// bits.c ... functions on bit-strings
// part of signature indexed files
// Bit-strings are arbitrarily long byte arrays
// Least significant bits (LSB) are in array[0]
// Most significant bits (MSB) are in array[nbytes-1]

// Written by John Shepherd, March 2019

#include <assert.h>
#include "defs.h"
#include "bits.h"
#include "page.h"

typedef struct _BitsRep {
    Count nbits;          // how many bits
    Count nbytes;          // how many bytes in array
    Byte bitstring[1];  // array of bytes to hold bits
    // actual array size is nbytes
} BitsRep;

// create a new Bits object

Bits newBits(int nbits) {
    Count nbytes = iceil(nbits, 8);
    Bits new = malloc(2 * sizeof(Count) + nbytes);
    new->nbits = nbits;
    new->nbytes = nbytes;
    memset(&(new->bitstring[0]), 0, nbytes);
    return new;
}

// release memory associated with a Bits object

void freeBits(Bits b) {
    free(b);
}

// check if the bit at position is 1

Bool bitIsSet(Bits b, int position) {
    assert(b != NULL);
    assert(0 <= position && position < b->nbits);
    Offset index = position / 8;
    Offset offset = position % 8;
    Byte mask = (1 << offset);
    return b->bitstring[index] & mask;
}

// check whether one Bits b1 is a subset of Bits b2

Bool isSubset(Bits b1, Bits b2) {
    assert(b1 != NULL && b2 != NULL);
    assert(b1->nbytes == b2->nbytes);
    //TODO
    for (int i = 0; i < b1->nbytes; ++i) {
        Byte checker = b1->bitstring[i] | b2->bitstring[i];
        if (checker != b2->bitstring[i])
            return FALSE;
    }
    return TRUE;
}

// set the bit at position to 1

void setBit(Bits b, int position) {
    assert(b != NULL);
    assert(0 <= position && position < b->nbits);
    Offset index = position / 8;
    Offset offset = position % 8;
    Byte mask = (1 << offset);
    b->bitstring[index] |= mask;
}

// set all bits to 1

void setAllBits(Bits b) {
    assert(b != NULL);
    memset(&(b->bitstring[0]), 0xff, b->nbytes);
    Offset total_offset = b->nbits % 8 ? (8 - b->nbits % 8) : 0;
    // reset shift bits to 0
    if (total_offset) {
        (b->bitstring[b->nbytes - 1]) >>= total_offset;
    }
}

// set the bit at position to 0

void unsetBit(Bits b, int position) {
    assert(b != NULL);
    assert(0 <= position && position < b->nbits);
    Offset index = position / 8;
    Offset offset = position % 8;
    Byte mask = (1 << offset);
    b->bitstring[index] &= ~mask;
}

// set all bits to 0

void unsetAllBits(Bits b) {
    assert(b != NULL);
    memset(&(b->bitstring[0]), 0, b->nbytes);
}

// bitwise AND ... b1 = b1 & b2

void andBits(Bits b1, Bits b2) {
    assert(b1 != NULL && b2 != NULL);
    assert(b1->nbytes == b2->nbytes);
    //TODO
    for (int i=0; i < b1->nbytes; i++) {
        b1->bitstring[i] &= b2->bitstring[i];
    }
}

// bitwise OR ... b1 = b1 | b2

void orBits(Bits b1, Bits b2) {
    assert(b1 != NULL && b2 != NULL);
    assert(b1->nbytes == b2->nbytes);
    //TODO
    for (int i=0; i < b1->nbytes; i++) {
        b1->bitstring[i] |= b2->bitstring[i];
    }
}

// left-shift ... b1 = b1 << n
// negative n gives right shift

void shiftBits(Bits b, int n) {
    // TODO
    assert(b != NULL);
    int shift_bytes = n / 8;
    int shift_bits = n % 8;
    if (shift_bytes * shift_bytes >= b->nbytes* b->nbytes) {
        unsetAllBits(b);
    } else {
        if (shift_bytes > 0) {
            int flag = 0;
            for (int i = b->nbytes - 1; i > 0; --i) {
                if (i - shift_bytes >= 0) {
                    b->bitstring[i] = b->bitstring[i - shift_bytes];
                } else {
                    flag = i + 1;
                    while (i >-1) {
                        b->bitstring[i] = 0;
                        i--;
                    }
                    break;
                }
            }
            if (shift_bits>0) {
                Byte shift_temp = 0x00;
                for (int i = flag; i < b->nbytes ; ++i) {
                    Byte new = b->bitstring[i] << shift_bits;
                    Byte temp = b->bitstring[i] >> (8 - shift_bits);
                    b->bitstring[i] = new | shift_temp;
                    shift_temp = temp;
                }
            }
        } else if (shift_bytes < 0) {
            int flag = b->nbytes-1;
            for (int i = 0; i < b->nbytes - 1; ++i) {
                // use - because shift_bytes is negative
                if (i - shift_bytes < b->nbytes) {
                    b->bitstring[i] = b->bitstring[i - shift_bytes];
                } else {
                    flag = i - 1;
                    while ( i< b->nbytes) {
                        b->bitstring[i] = 0;
                        i++;
                    }
                    break;
                }
            }
            if (shift_bits<0) {
                Byte shift_temp = 0x00;
                for (int i = flag; i >= 0; --i) {
                    Byte new = b->bitstring[i] >> -1 * shift_bits;
                    Byte temp = b->bitstring[i] << (8 + shift_bits);
                    b->bitstring[i] = new | shift_temp;
                    shift_temp = temp;
                }
            }
        } else {
            Byte shift_temp = 0x00;
            if (shift_bits>0){
                for (int i = 0; i < b->nbytes ; ++i) {
                    Byte new = b->bitstring[i] << shift_bits;
                    Byte temp = b->bitstring[i] >> (8 - shift_bits);
                    b->bitstring[i] = new | shift_temp;
                    shift_temp = temp;
                }
            } else if (shift_bits<0){
                for (int i = b->nbytes-1; i >= 0; --i) {
                    Byte new = b->bitstring[i] >> -1*shift_bits;
                    Byte temp = b->bitstring[i] << (8 + shift_bits);
                    b->bitstring[i] = new | shift_temp;
                    shift_temp = temp;
                }
            }
        }
    }
}

// get a bit-string (of length b->nbytes)
// from specified position in Page buffer
// and place it in a BitsRep structure
// NOTE: position offset is in byte
void getBits(Page p, Offset pos, Bits b) {
    //TODO
    Byte *start = addrInPage(p, pos, 1);
    memcpy(&b->bitstring[0], start, b->nbytes);
}

// copy the bit-string array in a BitsRep
// structure to specified position in Page buffer
// NOTE: position offset is in byte
void putBits(Page p, Offset pos, Bits b) {
    //TODO
    Byte *start = addrInPage(p, pos, 1);
    memcpy(start, &b->bitstring[0], b->nbytes);
}

// show Bits on stdout
// display in order MSB to LSB
// do not append '\n'

void showBits(Bits b) {
    assert(b != NULL);
    //printf("(%d,%d)",b->nbits,b->nbytes);
    for (int i = b->nbytes - 1; i >= 0; i--) {
        for (int j = 7; j >= 0; j--) {
            Byte mask = (1 << j);
            if (b->bitstring[i] & mask)
                putchar('1');
            else
                putchar('0');
        }
    }
}
