#include <protocol.h>

int read_message(FILE *stream, u_int8_t *buf) {
    int count_correct_read_number;
    buf[count_correct_read_number++] = 0x7E;
    const unsigned int mask = 0b11111;
    int number_read;
    int count_shift;
    u_int8_t number_tmp;

    while ((number_read = getc(stream)) != EOF) {
        int count_cycle = 0;
        number_tmp = number_read;
        number_tmp >>= count_shift;
        u_int8_t number_correct = count_one(number_read);
        do {
            count_cycle++;
            if (number_tmp == mask) {
                number_tmp <<= 1;
                count_shift++;
                break;
            }    
            number_tmp >>= 1;

        } while (number_tmp != mask && count_cycle < 3);
        buf[count_correct_read_number++] = number_read >> count_shift;
    buf[count_correct_read_number] = 0x7E;
    return count_correct_read_number;
}

int write_message(FILE *stream, u_int8_t *buf, size_t nbyte) {
    int count_correct_write_number;

    for (int i = 0; i < nbyte; ++i) {
        putc(buf[i], stream);
        count_correct_write_number++;
    }

    return count_correct_write_number;
}
