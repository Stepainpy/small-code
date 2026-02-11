/* CLEFIA data interpretation
 *
 * 0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | P0 | P1 | P2 | P3 | P4 | P5 | P6 | P7 | P8 | P9 | PA | PB | PC | PD | PE | PF |
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * ^- src
 *
 * 0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | C0 | C1 | C2 | C3 | C4 | C5 | C6 | C7 | C8 | C9 | CA | CB | CC | CD | CE | CF |
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * ^- dest
 *
 * 0    1             15   16                                         128-bit key
 * +----+---- ... ----+----+
 * | K0 + K1  ...  KE | KF |                          ->  KFKEKDKC KBKAK9K8 K7K6K5K4 K3K2K1K0
 * +----+---- ... ----+----+                                K[0]     K[1]     K[2]     K[3]
 * ^- key
 *
 * 0    1             15   16   17            23   24                 192-bit key
 * +----+---- ... ----+----+----+---- ... ----+----+
 * | K0 + K1  ...  KE | KF | k0 | k1  ...  k6 | k7 |  ->  k7k6k5k4 k3k2k1k0 KFKEKDKC KBKAK9K8
 * +----+---- ... ----+----+----+---- ... ----+----+       KL[0]    KL[1]    KL[2]    KL[3]
 * ^- key                                                 K7K6K5K4 K3K2K1K0
 *                                                         KR[0]    KR[1]
 *
 * 0    1             15   16   17            31   31                 256-bit key
 * +----+---- ... ----+----+----+---- ... ----+----+
 * | K0 + K1  ...  KE | KF | k0 | k1  ...  kE | kF |  ->  kFkEkDkC kBkAk9k8 k7k6k5k4 k3k2k1k0
 * +----+---- ... ----+----+----+---- ... ----+----+       KL[0]    KL[1]    KL[2]    KL[3]
 * ^- key                                                 KFKEKDKC KBKAK9K8 K7K6K5K4 K3K2K1K0
 *                                                         KR[3]    KR[2]    KR[1]    KR[0]
 *
 *
 * 0          4          8          12         16
 * +----------+----------+----------+----------+
 * | XFXEXDXC | XBXAX9X8 | X7X6X5X4 | X3X2X1X0 |
 * +----------+----------+----------+----------+
 *     X[0]       X[1]       X[2]       X[3]
 */

#ifndef CLEFIA_H
#define CLEFIA_H

typedef struct clefia_context_t clefia_context_t;

clefia_context_t* clefia_init_context(const void* key, int key_bits);
void clefia_release_context(clefia_context_t* ctx);

void clefia_block_encode(void* dest, const void* src, const clefia_context_t* ctx);
void clefia_block_decode(void* dest, const void* src, const clefia_context_t* ctx);

#endif /* CLEFIA_H */