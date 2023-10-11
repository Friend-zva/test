#include <protocol.h>
#include <inttypes.h>

int read_message(FILE *stream, void *buf) {
    uint8_t *y = (uint8_t *) buf;
    uint8_t mask = 0x1F;
    int count_correct_read_number = 0;
    int tmp = 0;
    int count_shift = 0;
    while ((tmp = getc(stream)) != EOF) {
        uint8_t number_tmp = tmp;
        
        if (number_tmp != 0x7E && number_tmp != 0xff) {
            for (int cycle = 3; cycle > -1; cycle--) {
                if ((number_tmp >> cycle) == mask) {
                    count_shift++;
                    break;
                }
            }
            y[count_correct_read_number++] = number_tmp;
        } else {
            count_correct_read_number++;
        }
    }

    return count_correct_read_number;
}


int write_message(FILE *stream, const void *buf, size_t nbyte) {
    uint8_t *array = (uint8_t *) buf;
    uint8_t mask = 0x1F;
    uint8_t number_tmp = 0;
    uint8_t correction = 0;
    int count_shift = 0;

    putc(0x7E, stream);
    int count_correct_write_number = 1;

    for (unsigned long ind = 0; ind < nbyte; ++ind) {
        number_tmp = (correction << (4 - count_shift)) | (array[ind] >> count_shift);

        for (int cycle = 3; cycle >= -1; cycle--) {
            if ((correction | (array[ind] >> 4)) >> cycle == mask) {
                number_tmp = (correction << (4 - count_shift)) | ((array[ind] >> (4 - cycle)) << (4 - cycle)) | ((array[ind] << (8 - cycle)) >> (8 - cycle + 1 + count_shift));
                count_shift++;
                break;
            }
        }

        //count_cycle = 0;
        for (int cycle = 3; cycle > -1; cycle--) {
            if ((number_tmp >> cycle) == mask) {
                number_tmp = ((number_tmp >> cycle) << cycle) | ((array[ind] << (8 - cycle)) >> (8 - cycle + 1 + count_shift));
                count_shift++;
                break;
            }
        }

        correction = array[ind] << 4;
        putc(number_tmp, stream);
        count_correct_write_number++;
    }
    putc((0x7E >> count_shift) | (correction << (8 - count_shift)), stream); // почему (8 - count_shift)
    count_correct_write_number++;
    if (count_shift % 8 != 0) {
        putc(0xFF >> count_shift | (0x7E << (8 - count_shift)), stream); // & ? |
    }
    return count_correct_write_number;
}
