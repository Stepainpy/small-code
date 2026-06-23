#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>

typedef struct {
    void*  in; size_t (*read )(      void* dst, size_t size, size_t count, void*  input);
    void* out; size_t (*write)(const void* src, size_t size, size_t count, void* output);
} base64_io_t;

/* Return value
 * 0 on success, nonzero otherwise
 */

/* 'line' argument
 * Count of letters in output line.
 * If set as 0, then not separate by lines.
 */

int base64_encode(const base64_io_t* io, size_t line);
int base64_decode(const base64_io_t* io);

#endif /* BASE64_H */

#ifdef BASE64_IMPLEMENTATION

#if BASE64_URL_SAFE
#  define BASE64I_ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
#else
#  define BASE64I_ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#endif

int base64_encode(const base64_io_t* io, size_t line) {
    size_t rem;

    if (!io || !io->read || !io->write) return 1;

    if (line == 0) line -= 1;
    rem = line;

    while (1) {
        unsigned char bbuf[3] = {0};
        char abuf[4] = {0};
        long ibuf = 0;

        size_t cnt = io->read(bbuf, 1, 3, io->in);
        if (cnt == 0) return 0;

        ibuf =             bbuf[0];
        ibuf = ibuf << 8 | bbuf[1];
        ibuf = ibuf << 8 | bbuf[2];

        abuf[3] = cnt > 2 ? BASE64I_ALPHABET[ibuf & 0x3F] : '='; ibuf >>= 6;
        abuf[2] = cnt > 1 ? BASE64I_ALPHABET[ibuf & 0x3F] : '='; ibuf >>= 6;
        abuf[1] =           BASE64I_ALPHABET[ibuf & 0x3F]      ; ibuf >>= 6;
        abuf[0] =           BASE64I_ALPHABET[ibuf & 0x3F]      ;

        if (rem == 0) {
            if (!io->write("\n", 1, 1, io->out)) return 1;
            rem = line;
        }

        if (rem >= 4) {
            if (!io->write(abuf, 4, 1, io->out)) return 1;
            rem -= 4;
        } else {
            if (!io->write(abuf, rem, 1, io->out)) return 1;
            if (!io->write("\n",   1, 1, io->out)) return 1;
            if (!io->write(abuf + rem, 4 - rem, 1, io->out)) return 1;
            rem = line - 4 + rem;
        }
    }

    return 0;
}

static int base64i_ch2idx(char ch) {
    if ('A' <= ch && ch <= 'Z') return  0 + ch - 'A';
    if ('a' <= ch && ch <= 'z') return 26 + ch - 'a';
    if ('0' <= ch && ch <= '9') return 52 + ch - '0';
    if (ch == BASE64I_ALPHABET[62]) return 62;
    if (ch == BASE64I_ALPHABET[63]) return 63;
    return -1;
}

static size_t base64i_normilize(char buf[4]) {
    size_t i = 0;
    if (base64i_ch2idx(buf[0]) >= 0) buf[i++] = buf[0];
    if (base64i_ch2idx(buf[1]) >= 0) buf[i++] = buf[1];
    if (base64i_ch2idx(buf[2]) >= 0) buf[i++] = buf[2];
    if (base64i_ch2idx(buf[3]) >= 0) buf[i++] = buf[3];
    return i;
}

int base64_decode(const base64_io_t* io) {
    if (!io || !io->read || !io->write) return 1;

    while (1) {
        unsigned char bbuf[3] = {0};
        char abuf[4] = {0};
        size_t fill = 0;
        long ibuf = 0;
        int i;

        do {
            size_t cnt = io->read(abuf + fill, 1, 4 - fill, io->in);
            if (cnt == 0) { if (fill == 0) return 0; else break; }
            fill = base64i_normilize(abuf);
        } while (fill < 4);

        if (fill == 1) return 1;

        ibuf =             ((i = base64i_ch2idx(abuf[0])) < 0 ? 0 : i);
        ibuf = ibuf << 6 | ((i = base64i_ch2idx(abuf[1])) < 0 ? 0 : i);
        ibuf = ibuf << 6 | ((i = base64i_ch2idx(abuf[2])) < 0 ? 0 : i);
        ibuf = ibuf << 6 | ((i = base64i_ch2idx(abuf[3])) < 0 ? 0 : i);

        bbuf[2] = ibuf & 0xFF; ibuf >>= 8;
        bbuf[1] = ibuf & 0xFF; ibuf >>= 8;
        bbuf[0] = ibuf & 0xFF;

        if (!io->write(bbuf, fill - 1, 1, io->out)) return 1;
    }

    return 0;
}

#endif /* BASE64_IMPLEMENTATION */