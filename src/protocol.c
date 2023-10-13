#include <protocol.h>
#include <inttypes.h>

#define marker 0x7e
#define mask 0x1f
#define spare_units 0xff

int write_message(FILE *stream, const void *buf, size_t nbyte) {
    uint8_t *buffer = (uint8_t *) buf;
    unsigned int count_write_number = 0;
    uint8_t number_tmp = 0;
    uint8_t number_correction = 0;
    unsigned int count_shift = 0;
    putc(marker, stream);

    for (unsigned long index = 0; index < nbyte; ++index) {
        //printf("index = %lu ", index);
        //printf("buffer >> (4 + count_shift) = %x\n", buffer[index] >> (4 + count_shift));
        for (int cycle = 3; cycle >= 0; cycle--) {
            if ((((number_tmp | number_correction | (buffer[index] >> (4 + count_shift))) >> cycle) & mask) == mask) {
                //printf("cycle = %x ", cycle);
                //printf("проверка: %x ", (number_tmp | number_correction | (buffer[index] >> (4 + count_shift))) >> cycle);
                uint8_t one = (buffer[index] >> (8 - count_shift - cycle));
                uint8_t two = (buffer[index] << (count_shift + cycle));
                number_tmp = number_correction | (one << (8 - count_shift - cycle)) | (two >> (count_shift + cycle + 1));
                //printf("number_tmp = %x\n", number_tmp);
                count_shift++;
                // number_correction = number_tmp >> count_shift;
                // number_correction <<= count_shift; // dont change
                // //printf("number_correction = %x\n", number_correction);
                break;
            }
        }

        number_correction <<= 4;
        number_tmp = number_correction | (buffer[index] >> count_shift); // test
        number_correction = buffer[index] << (8 - count_shift);
        //printf("number_tmp вне цикла = %x\n", number_tmp);
        for (int cycle = 3; cycle >= 0; cycle--) {
            if (((number_tmp >> cycle) & mask) == mask) {
                uint8_t one = number_tmp >> cycle;
                uint8_t two = number_tmp << (8 - cycle);
                number_tmp = (one << cycle) | (two >> (8 - cycle + 1));
                count_shift++;
                if (cycle == 0) {
                    number_correction <<= 1;
                } else {
                    number_correction = buffer[index] << (8 - count_shift);
                }
                break;
            }
        }
        
        putc(number_tmp, stream);
        count_write_number++;
        //printf("putc = %x ", number_tmp);
        //number_correction = buffer[index] << (8 - count_shift);
        //printf("number_correction = %x ", number_correction);
        number_correction >>= 4;
        //printf("number_correction >> 4 = %x ", number_correction);
        number_tmp <<= 4;
        //printf("putc << 4 = %x\n", number_tmp);
        //printf("\n");
        ////printf("%x\n", number_tmp);
        //number_checked = number_tmp << 4 | buffer[index] << (8 - count_shift);
        //number_correction = buffer[index] << (8 - count_shift);
    }
    putc(number_correction | (marker >> count_shift), stream);
    number_correction = marker << (8 - count_shift);
    if (count_shift != 0) {
        putc(number_correction | (spare_units >> count_shift), stream);
    }

    return count_write_number;
}

int read_message(FILE *stream, void *buf) {
    uint8_t *buffer = (uint8_t *) buf;
    int count_read_number = 0;
    uint8_t number_tmp = 0;
    //uint8_t number_correction = 0;
    int tmp = 0; // char не робит?

    while ((tmp = getc(stream)) != EOF) {
        number_tmp = tmp;
        if (number_tmp != marker) {
            buffer[count_read_number++] = number_tmp;
        }
    }
    return count_read_number;
}
