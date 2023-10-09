#include <protocol.h>
#include <inttypes.h>

int read_message(FILE *stream, void *buf) {
    uint8_t *x = (uint8_t *) buf;
    int count_correct_write_number;

    for (int i = 0; i < 10; ++i) {
        putc(x[i], stream);
        count_correct_write_number++;
    }

    return count_correct_write_number;
}

int write_message(FILE *stream, const void *buf, size_t nbyte) {
    uint8_t *x = (uint8_t *) buf; // только новую переменную создавать?
    putc(0x77, stream); //проверка ?
    int mask = 0b11111;
    int count_correct_write_number = 1;
    int count_shift = 0;
    uint8_t number_tmp = 0;
    uint8_t number_correct = 0;
    int count_cycle = 0;

    for (int i = 0; i < nbyte; ++i) {
        number_tmp = (x[i] >> count_shift) | number_correct; // проверка?
        
        for (int i = 3; i > -1; i--) {
            number_tmp >>= i;
            if (number_tmp == mask) {
                count_shift++;
                count_cycle = i;
                break;
            }
        }

        //number_correct = (buf[i] << (8 - count_cycle));
        //printf("%x\n", number_tmp); ???????
        putc((number_tmp << count_cycle) | ((x[i] << (8 - count_cycle)) >> (8 - count_cycle + 1)), stream);
        count_correct_write_number++;
        count_cycle = 0;

        number_correct = (x[i] << (8 - count_cycle)) >> (8 - count_cycle + 1); // если 0, то все равно сдвинется
    }
    if (count_shift != 0) {
        putc((0x77 >> count_shift) | number_correct, stream);
        count_correct_write_number++;
        putc((0x77 << (8 - count_shift)) | (0xFF >> count_shift), stream);
    } else {
        putc(0x77, stream);
    }
    return count_correct_write_number;
}
