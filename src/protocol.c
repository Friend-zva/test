#include <protocol.h>

int write_message(FILE *stream, const void *buf, size_t nbyte) {
    uint8_t *buffer = (uint8_t *) buf;
    int count_write_byte = 0;
    uint8_t byte_write = 0;
    uint8_t byte_joint = 0;
    uint8_t byte_shift = 0;
    int count_shift = 0;
    
    if (nbyte > MAX_MESSAGE_LEN) {
        error("Number of bytes cannot be more than MAX_MESSAGE_LEN\n");
        return EOF;
    }
    putc(marker, stream);

    for (size_t index = 0; index < nbyte; ++index) {
        byte_joint = byte_write | (byte_shift >> (len_byte / 2)) | (buffer[index] >> ((len_byte / 2) + count_shift));
        if (search_mask_byte_joint(&byte_joint, byte_shift)) {
            count_shift++;
            byte_write = byte_joint | (buffer[index] >> count_shift);

            if (check_count_shift(stream, &count_shift, &byte_shift)) {
                byte_write = buffer[index];
            }
        } else {
            byte_write = byte_shift | (buffer[index] >> count_shift);
        }
        if (search_mask_byte_write(&byte_write)) {
            if ((byte_write & 0x1f) == mask) {
                count_shift++;
                byte_shift = (buffer[index] << (len_byte - (count_shift - 1)));
                byte_shift >>= 1;
            } else {
                count_shift++;
                byte_shift = (buffer[index] << (len_byte - count_shift));
            }
        } else {
            byte_shift = (buffer[index] << (len_byte - count_shift));
        }
        putc(byte_write, stream);
        count_write_byte++;

        if (check_count_shift(stream, &count_shift, &byte_shift)) {
            byte_write = byte_shift;
        }

        byte_write <<= (len_byte / 2);
    }
    byte_joint = byte_write | (byte_shift >> (len_byte / 2));
    
    if (search_mask_byte_joint(&byte_joint, byte_shift)) {
        count_shift++;
        byte_write = byte_joint | (marker >> count_shift);
    } else {
        byte_write = (byte_joint << (len_byte / 2)) | (marker >> count_shift);
    }

    if (count_shift == 0) {
        putc(marker, stream);
    } else if (count_shift == len_byte) {
        putc(byte_write, stream);
        putc(marker, stream);
    } else {
        putc(byte_shift | marker >> count_shift, stream);
        putc(marker << (len_byte - count_shift) | (spare_units >> count_shift), stream);
    }

    return count_write_byte;
}

int read_message(FILE *stream, void *buf) { // 111111 -> eof + не удалось прочитать + не начинать пока не встретил маркер иначе eof + в конце не может быть более 1 нуля + read_twice
    uint8_t *buffer = (uint8_t *) buf;
    int symbol_read = 0;
    int count_read_byte = 0;
    uint8_t byte_read = 0;
    uint8_t byte_shift = 0;
    int count_shift = 0;

    if (read_marker(stream, &byte_read, &count_shift)) {
        error("Cannot read message\n");
        return EOF;
    }
    if (byte_read == 0) {
        if ((symbol_read = getc(stream)) != EOF || symbol_read != marker) {
            byte_read = (uint8_t) symbol_read;
        }
    }
    
    for (unsigned int i = 0; (symbol_read = getc(stream)) != EOF; ++i) {
        byte_shift = ((uint8_t) symbol_read) >> (len_byte - count_shift);
        if ((byte_read | byte_shift) == marker) {
            if (((uint8_t) symbol_read & (spare_units >> (len_byte - count_shift))) == (spare_units >> (len_byte - count_shift))) {
                break;
            } else {
                error("Uncorrect message\n");
                return EOF;
            }
        }

        byte_shift = (uint8_t) symbol_read >> (len_byte - count_shift);
        byte_read |= byte_shift;
        for (int cycle = 3; cycle >= 0; cycle--) {
            if (((byte_read >> cycle) & mask) == mask) {
                uint8_t one = byte_read >> cycle;
                uint8_t two = byte_read << (len_byte - cycle + 1);
                byte_read = (one << cycle) | (two >> (len_byte - cycle));
                count_shift++;
                break;
            }
        }

        buffer[count_read_byte++] = byte_read;
        byte_read = (uint8_t) symbol_read << count_shift;
    }
    if (ferror(stream)) {
        error("Cannot read symbol\n");
        return -1;
    }

    return count_read_byte;
}

int search_mask_byte(const uint8_t byte_check) {
    for (int cycle = 3; cycle >= 0; cycle--) {
        if (((byte_check >> cycle) & mask) == mask) {
            return cycle;
        }
    }
    return -1;
}

int search_mask_byte_joint(uint8_t *byte_joint, const uint8_t byte_shift) {
    int cycle = search_mask_byte(*byte_joint);
    if (cycle != -1) {
        uint8_t part_one = *byte_joint >> cycle;
        uint8_t part_two = byte_shift << ((len_byte / 2) - cycle);
        *byte_joint = part_one << ((len_byte / 2) + cycle) | part_two >> ((len_byte / 2) - cycle + 1);
        return 1;
    }
    return 0;
}

int search_mask_byte_write(uint8_t *byte_write) {
    int cycle = search_mask_byte(*byte_write);
    if (cycle != -1) {
        uint8_t part_one = *byte_write >> cycle;
        uint8_t part_two = *byte_write << (len_byte - cycle);
        *byte_write = part_one << cycle | part_two >> (len_byte - cycle + 1);
        return 1;
    }
    return 0;
}

int check_count_shift(FILE *stream, int *count_shift, uint8_t *byte_shift) {
    if (*count_shift == len_byte) {
        if (search_mask_byte_write(*byte_shift)) {
            putc(*byte_shift, stream);
            *count_shift = 1;
            *byte_shift <<= (len_byte - *count_shift);
        } else {
            putc(*byte_shift, stream);
            *count_shift = 0;
            *byte_shift = 0;
        }
        return 1;
    }
    return 0;
}

int read_marker(FILE *stream, uint8_t *byte_read, int *count_shift) {
    int symbol_read = 0;
    for (unsigned int i = 0; (symbol_read = getc(stream)) != EOF; ++i) {
        if ((((uint8_t) symbol_read) >> *count_shift | *byte_read) == marker) {
            if (*count_shift != 0) {
                *byte_read = ((uint8_t) symbol_read) << *count_shift;
            }
            return 0;
        } else {
            for (int cycle = 7; cycle >= 0; cycle--) {
                if ((((uint8_t) symbol_read >> cycle) & 0x01) == 0x00) {
                    *count_shift = (7 - cycle);
                    *byte_read = (uint8_t) symbol_read << *count_shift;
                    break;
                }
            }
        }
    }
    return 1;
}