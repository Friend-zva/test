#include <protocol.h>

#define marker 0x7e
#define mask 0x1f
#define spare_units 0xff

int search_mask(uint8_t *byte_check) {
    for (int cycle = 3; cycle >= 0; cycle--) {
        if (((*byte_check >> cycle) & mask) == mask) {
            uint8_t part_one = *byte_check >> cycle;
            uint8_t part_two = *byte_check << (8 - cycle);
            *byte_check = (part_one << cycle) | (part_two >> (8 - cycle + 1));
            return 1;
        }
    }
    return 0;
}

int write_message(FILE *stream, const void *buf, size_t nbyte) {
    uint8_t *buffer = (uint8_t *) buf;
    int count_write_byte = 0;
    uint8_t byte_tmp = 0;
    uint8_t byte_joint = 0;
    uint8_t byte_shift = 0;
    unsigned int count_shift = 0;

    putc(marker, stream);

    for (unsigned long index = 0; index < nbyte; ++index) {    
        byte_joint = byte_tmp | (byte_shift >> 4) | (buffer[index] >> (4 + count_shift));
        byte_tmp = byte_shift | (buffer[index] >> count_shift);
        for (int cycle = 3; cycle >= 0; cycle--) {
            if (((byte_joint >> cycle) & mask) == mask) {
                uint8_t part_one = byte_joint >> cycle;
                uint8_t part_two = byte_joint << (8 - cycle);
                byte_joint = (part_one << cycle) | (part_two >> (8 - cycle + 1));
                if (count_shift > 4) {
                    byte_tmp = (byte_joint << 4) | (byte_shift & 0x0f) | (buffer[index] >> count_shift); // что???
                } else {
                    byte_tmp = (byte_joint << 4) | (buffer[index] >> (4 - cycle + count_shift)); // подобрал
                }
                count_shift++;
                break;
            }
        }

        if (count_shift == 8) {
            byte_tmp = byte_shift;
            if (search_mask(&byte_tmp)) {
                putc(byte_tmp, stream);
                count_shift = 1;
                byte_shift <<= (8 - count_shift);
                byte_tmp = byte_shift | (buffer[index] >> count_shift);
            } else {
                putc(byte_tmp << 4, stream);
                count_shift = 0;
                byte_shift = 0;
                byte_tmp = buffer[index];
            }
        }

        byte_shift = buffer[index] << (8 - count_shift);

        for (int cycle = 3; cycle >= 0; cycle--) {
            if (((byte_tmp >> cycle) & mask) == mask) {
                uint8_t one = byte_tmp >> cycle;
                uint8_t two = byte_tmp << (8 - cycle);
                byte_tmp = (one << cycle) | (two >> (8 - cycle + 1)); 
                count_shift++;
                if (cycle == 0) {
                    byte_shift >>= 1;
                } else {
                    byte_shift = buffer[index] << (8 - count_shift);
                }
                break;
            }
        }

        putc(byte_tmp, stream);

        if (count_shift == 8) {
            byte_tmp = byte_shift;
            if (search_mask(&byte_tmp)) {
                putc(byte_tmp, stream);
                count_shift = 1;
                byte_shift <<= (8 - count_shift);
            } else {
                putc(byte_tmp << 4, stream);
                count_shift = 0;
                byte_shift = 0;
            }
        }
        
        byte_tmp <<= 4;
        count_write_byte++;
    }
    int flag = 0;

    for (int cycle = 3; cycle >= 0; cycle--) {
        if ((((byte_tmp | byte_shift >> 4) >> cycle) & mask) == mask) {
            byte_tmp |= (byte_shift >> 4);
            count_shift++;
            if (count_shift > 4) {
                uint8_t part_one = byte_tmp >> cycle;
                uint8_t part_two = byte_shift << (4 - cycle);
                byte_tmp = part_one << (4 + cycle) | part_two >> (4 - cycle +1) | marker >> (count_shift); // лишнее?
            
                putc(byte_tmp, stream);
            } else {
                putc(byte_tmp << 4 | marker >> (count_shift), stream); // byte_tmp << (4 + cycle)?
            }
            flag = 1;
            break;
        }
    }

    if (flag == 0) {
        if (count_shift != 0) {
            putc(byte_shift | marker >> count_shift, stream);
        } else {
            putc(marker, stream);
        }
    }

    if (count_shift != 0) {
        putc(marker << (8 - count_shift) | (spare_units >> count_shift), stream);
    }

    return count_write_byte;
}

int read_message(FILE *stream, void *buf) {
    uint8_t *buffer = (uint8_t *) buf;
    int count_read_byte = 0;
    uint8_t byte_tmp = 0;
    uint8_t byte_shift = 0;
    unsigned int count_shift = 0;
    int tmp = 0;

    for (unsigned int index = 0; (tmp = getc(stream)) != EOF; ++index) {
        tmp = (uint8_t) tmp;
        if (tmp != marker) {
            if (index == 0) {
                for (int cycle = 7; cycle >= 0; cycle--) {
                    if (((tmp >> cycle) & 0x01) == 0x00) {
                        count_shift = (7 - cycle);
                        break;
                    }
                }
                continue;
            }

            byte_shift = tmp >> (8 - count_shift);
            if (count_read_byte != 0) {
                buffer[count_read_byte - 1] = byte_tmp | byte_shift;
            }

            byte_tmp = tmp << count_shift;
            if (byte_tmp == marker) {
                break;
            }

            for (int cycle = 3; cycle >= 0; cycle--) {
                if (((byte_tmp >> cycle) & mask) == mask) {
                    uint8_t one = byte_tmp >> cycle;
                    one <<= cycle;
                    uint8_t two = byte_tmp << (8 - cycle + 1);
                    two >>= (8 - cycle);
                    byte_tmp = one | two;
                    count_shift++;
                    break;
                }
            }

            count_read_byte++;
        }
    }

    return count_read_byte;
}
