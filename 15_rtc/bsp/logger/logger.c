#include "logger.h"

extern void putc(unsigned char c);

void log_dump(const uint8_t *data, uint16_t len) {
    uint8_t ch, cl;
    for (int i = 0; i < len; i++) {
        ch = data[i] >> 4;
        cl = data[i] & 0x0F;
        if (ch < 10) ch += '0';
        else
            ch += 'A' - 10;
        if (cl < 10) cl += '0';
        else
            cl += 'A' - 10;
        putc(ch);
        putc(cl);
        printf(" ");
        if ((i + 1) % 16 == 0) {
            printf("\r\n");
        } else if ((i + 1) % 8 == 0) {
            ch = ' ';
            putc(ch);
            putc(ch);
            putc(ch);
            putc(ch);
        } else {
            ch = ' ';
            putc(ch);
        }
    }
    //    printf("\r\n");
}
