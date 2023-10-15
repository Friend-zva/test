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
    unsigned int count_write_byte = 0;
    uint8_t byte_tmp = 0;
    uint8_t byte_joint = 0;
    uint8_t byte_shift = 0;
    unsigned int count_shift = 0;
    // printf("putc = %x ", marker);
    putc(marker, stream);

    for (unsigned long index = 0; index < nbyte; ++index) {
        
        // printf("\nindex = %lu  ", index);
        // printf("buffer:%x ", buffer[index]);
        // printf("count_shift = %x ", count_shift);
        // printf("byte_shift = %x ", byte_shift);
        // printf("byte_tmp = %x ", byte_tmp);
        
        byte_joint = byte_tmp | (byte_shift >> 4) | (buffer[index] >> (4 + count_shift));
        byte_tmp = byte_shift | (buffer[index] >> count_shift);
        // printf("byte_joint = %x\n", byte_joint);
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

        // if (search_mask(&byte_joint)) {
        //     printf("byte_joint = %x\n", byte_joint);
        //     byte_tmp = (byte_joint << 4) | ((buffer[index] & 0x1f) >> count_shift);
        //     count_shift++;
        // } else {
        //     byte_tmp = byte_shift | (buffer[index] >> count_shift);
        // }

        // printf("byte_tmp = %x ", byte_tmp);
        byte_shift = buffer[index] << (8 - count_shift);
        // if (search_mask_in_byte_read(&byte_tmp) == 0) {
        //     byte_shift >>= 1;
        // } else {
        //     byte_shift = buffer[index] << (8 - (count_shift + 1));
        // }
        for (int cycle = 3; cycle >= 0; cycle--) {
            if (((byte_tmp >> cycle) & mask) == mask) {
                uint8_t one = byte_tmp >> cycle;
                uint8_t two = byte_tmp << (8 - cycle);
                byte_tmp = (one << cycle) | (two >> (8 - cycle + 1)); // разве one != byte_tmp ? 
                count_shift++;
                //printf("count_shift = %x ", count_shift);
                if (cycle == 0) {
                    byte_shift >>= 1;
                } else {
                    byte_shift = buffer[index] << (8 - count_shift);
                }
                break;
            }
        }
        // printf("putc = %x ", byte_tmp);
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
        
        // printf("byte_shift = %x ", byte_shift);
        // printf("putc = %x\n", byte_tmp);
        byte_tmp <<= 4;
        count_write_byte++;
    }
    int flag = 0;
    for (int cycle = 3; cycle >= 0; cycle--) {
        if ((((byte_tmp | byte_shift >> 4) >> cycle) & mask) == mask) {
            byte_tmp |= (byte_shift >> 4);
            count_shift++;
            // printf("\n\n\n");
            // printf("%x\n", byte_tmp);
            if (count_shift > 4) {
                uint8_t part_one = byte_tmp >> cycle;
                uint8_t part_two = byte_shift << (4 - cycle);
                byte_tmp = part_one << (4 + cycle) | part_two >> (4 - cycle +1) | marker >> (count_shift); // лишнее?
                // printf("\n\n%x\n\n", byte_tmp);
// привет, вова. тут крч частный случай но вот так ^ можно круто считывать и все будет ок. удачи. 
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
            // printf("putc = %x ", byte_shift | marker >> count_shift);
            putc(byte_shift | marker >> count_shift, stream);
        } else {
            // printf("putc = %x ", marker);
            putc(marker, stream);
        }
    }
    if (count_shift != 0) {
        // uint8_t gg = marker << (8 - count_shift) | (spare_units >> count_shift);
        // printf("putc = %x ", gg);
        putc(marker << (8 - count_shift) | (spare_units >> count_shift), stream);
    }

    return count_write_byte;
}

int read_message(FILE *stream, void *buf) {
    uint8_t *buffer = (uint8_t *) buf;
    unsigned int count_read_byte = 0;
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
                    uint8_t two = byte_tmp << (8 - cycle + 1);
                    byte_tmp = (one << cycle) | (two >> (8 - cycle));
                    count_shift++;
                    //printf("count_shift = %x ", count_shift);
                    // if (cycle == 0) {
                    //     byte_shift >>= 1;
                    // } else {
                    //     byte_shift = tmp << (8 - count_shift);
                    // }
                    break;
                }
            }
            count_read_byte++;
        }
    }
    return count_read_byte;
}
