/* CLEFIA data interpretation
 *
 * 0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | PF | PE | PD | PC | PB | PA | P9 | P8 | P7 | P6 | P5 | P4 | P3 | P2 | P1 | P0 |
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * ^- src
 *
 * 0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | CF | CE | CD | CC | CB | CA | C9 | C8 | C7 | C6 | C5 | C4 | C3 | C2 | C1 | C0 |
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * ^- dest
 *
 * 0    1             15   16                                         128-bit key
 * +----+---- ... ----+----+
 * | KF | KE  ...  K1 | K0 |                          ->  KFKEKDKC KBKAK9K8 K7K6K5K4 K3K2K1K0
 * +----+---- ... ----+----+                                K[0]     K[1]     K[2]     K[3]
 * ^- key
 *
 * 0    1             15   16   17            23   24                 192-bit key
 * +----+---- ... ----+----+----+---- ... ----+----+
 * | KF | KE  ...  K1 | K0 | kF | kE  ...  k9 | k8 |  ->  KFKEKDKC KBKAK9K8 K7K6K5K4 K3K2K1K0
 * +----+---- ... ----+----+----+---- ... ----+----+       KL[0]    KL[1]    KL[2]    KL[3]
 * ^- key                                                 kFkEkDkC kBkAk9k8
 *                                                         KR[0]    KR[1]
 *
 * 0    1             15   16   17            31   31                 256-bit key
 * +----+---- ... ----+----+----+---- ... ----+----+
 * | KF | KE  ...  K1 | K0 | kF | kE  ...  k1 | k0 |  ->  KFKEKDKC KBKAK9K8 K7K6K5K4 K3K2K1K0
 * +----+---- ... ----+----+----+---- ... ----+----+       KL[0]    KL[1]    KL[2]    KL[3]
 * ^- key                                                 kFkEkDkC kBkAk9k8 k7k6k5k4 k3k2k1k0
 *                                                         KR[3]    KR[2]    KR[1]    KR[0]
 *
 *
 * 0            4            8            12           16
 * +------------+------------+------------+------------+
 * | 0xTFTETDTC | 0xTBTAT9T8 | 0xT7T6T5T4 | 0xT3T2T1T0 |
 * +------------+------------+------------+------------+
 *      T[0]         T[1]         T[2]         T[3]
 */

#ifndef CLEFIA_BLOCK_CIPHER_H
#define CLEFIA_BLOCK_CIPHER_H

#include <limits.h>

#   if  UINT_MAX == 0xFFFFFFFFu
typedef unsigned int  clefia_word_t;
# elif ULONG_MAX == 0xFFFFFFFFul
typedef unsigned long clefia_word_t;
# else
#error Not found 32-bit integer
#endif

typedef struct clefia_context_t {
    clefia_word_t RK[52];
    clefia_word_t WK[ 4];
    clefia_word_t rounds;
} clefia_context_t;

int clefia_init_context(clefia_context_t* ctx, const void* key, int key_bits);

void clefia_block_encode(void* dest, const void* src, const clefia_context_t* ctx);
void clefia_block_decode(void* dest, const void* src, const clefia_context_t* ctx);

#endif /* CLEFIA_BLOCK_CIPHER_H */