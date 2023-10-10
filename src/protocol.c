#include <protocol.h>
#include <inttypes.h>

int read_message(FILE *stream, void *buf) {
    uint8_t *y = (uint8_t *) buf;
    int count_correct_read_number = 0;
    int tmp = 0;
    while ((tmp = getc(stream)) != EOF) {
        y[count_correct_read_number++] = tmp;
        printf("%x ", y[count_correct_read_number - 1]);
    }

    return count_correct_read_number;
}

int write_message(FILE *stream, const void *buf, size_t nbyte) {
    int mask = 0x1F; 
    int count_correct_write_number = 1;
    int count_shift = 0;
    uint8_t number_tmp = 0;
    uint8_t number_correct = 0;
    putc(0x77, stream); 

    uint8_t last4 = 0;
    uint8_t *array = (uint8_t *) buf; 
    uint8_t xxx = 0;

    for (int i = 0; i < nbyte; ++i) {
        uint8_t shift_correct = array[i] >> count_shift;
        //printf("%x\n", shift_correct);
        uint8_t check = last4 | (shift_correct >> 4);
        for (int j = 3; j > -1; j--) { // проверка на number_correct + shift_correct
            if ((check >> j) == mask) {
                count_shift++;
                check >>= j;
                uint8_t shift_left =  shift_correct << (8 - j);
                uint8_t shift_right = shift_left >> (8 - j + 1);
                uint8_t shift_return = check << (j + 4);
                number_correct = shift_left;
                //printf("%x\n", shift_right);
                shift_correct = shift_return | shift_right;
                break;
            }
        }
        //printf("%x\n", shift_correct);
        number_tmp = shift_correct | xxx; 
        //printf("%x\n", number_tmp);

        /*for (int j = 3; j > -1; j--) { // проверка на number_correct + shift_correct
            if ((number_tmp >> j) == mask) {
                count_shift++;
                number_tmp >>= j;
                count_cycle = j; // мб без j?
                break;
            }
        }*/

        int count_cycle = 0;
        for (int j = 3; j > -1; j--) {
            if ((number_tmp >> j) == mask) {
                count_shift++;
                number_tmp >>= j;
                count_cycle = j; // мб без j?
                break;
            }
        }

        if (count_shift == 0) {
            uint8_t shift_left = number_tmp << (8 - count_cycle);
            uint8_t shift_right = shift_left >> (8 - count_cycle + 1);
            uint8_t shift_return = number_tmp << count_cycle;
            last4 = (shift_return | shift_right) << 4;
            putc(number_tmp, stream);
            number_correct = shift_left;
        } else if (count_shift == 8) {
            // проверка нужна
            count_shift = 0;
            // очистка нужна
        } else {
            uint8_t shift_left = number_tmp << (8 - count_cycle);
            uint8_t shift_right = shift_left >> (8 - count_cycle + 1);
            uint8_t shift_return = number_tmp << count_cycle;
            last4 = (shift_return | shift_right) << 4;
            putc(shift_return | shift_right, stream);
            number_correct = shift_left;
        }
        if (count_shift != 0) {
            xxx = number_correct << (1 + count_shift); 
        }
        //number_correct = (buf[i] << (8 - count_cycle));
        count_correct_write_number++;

        //number_correct = (array[i] << (8 - count_cycle)) >> (8 - count_cycle + 1); // если 0, то все равно сдвинется
    }
    if (count_shift != 0) {
        putc((0x77 >> count_shift) | xxx, stream);
        xxx = 0x77 << (8 - count_shift);
        count_correct_write_number++;
        putc(xxx | (0xFF >> count_shift), stream);
    } else {
        putc(0x77, stream);
    }
    return count_correct_write_number;
}