#include <protocol.h>

int write_message(FILE *stream, const void *buf, size_t nbyte) {
    if (nbyte > MAX_MESSAGE_LEN) {
        error("Number of bytes cannot be more than MAX_MESSAGE_LEN\n");
        return EOF;
    }

    if (putc(marker, stream) == EOF) {
        error("Cannot write byte\n");
        return EOF;
    }

    uint8_t *buffer = (uint8_t *) buf;
    int count_byte_write = 0;
    uint8_t byte_write = zero;
    uint8_t byte_joint = zero;
    uint8_t byte_shift = zero;
    int count_shift = 0;

    for (size_t index = 0; index < nbyte; ++index) {
        byte_joint = (byte_write << (len_byte / 2)) | (byte_shift >> (len_byte / 2)) 
                   | (buffer[index] >> ((len_byte / 2) + count_shift));

        if (search_mask_byte_joint(&byte_joint)) {
            count_shift++;
            if (count_shift <= 4) {
                uint8_t part_two = buffer[index] << ((len_byte / 2) - (count_shift - 1));
                byte_joint |= part_two >> ((len_byte / 2) - (count_shift - 1) + count_shift);
            } else {
                uint8_t part_two = byte_shift << (len_byte / 2);
                byte_joint |= (part_two >> ((len_byte / 2) + 1)) | (buffer[index] >> (count_shift));
            }
            
            byte_write = byte_joint;

            if (check_count_shift_after_joint(stream, &byte_write, &byte_joint, 
                                              &byte_shift, buffer[index], &count_shift)) {
                return EOF;
            }
        } else {
            byte_write = byte_shift | (buffer[index] >> count_shift);
        }

        if (search_mask_byte_write(&byte_write)) {
            count_shift++;
            if ((byte_write & mask) == mask) {
                byte_shift = buffer[index] << (len_byte - (count_shift - 1));
                byte_shift >>= 1;
            } else {
                byte_shift = buffer[index] << (len_byte - count_shift);
            }
        } else {
            byte_shift = buffer[index] << (len_byte - count_shift);
        }

        if (putc(byte_write, stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
        count_byte_write++;

        if (check_count_shift_after_write(stream, &byte_write, &byte_joint, 
                                          &byte_shift, &count_shift)) {
            return EOF;
        }
    }
    byte_joint = (byte_write << (len_byte / 2)) | (byte_shift >> (len_byte / 2));
    
    if (search_mask_byte_joint(&byte_joint)) {
        count_shift++;
        if (count_shift > 4) {
            uint8_t part_two = byte_shift << (len_byte / 2);
            byte_joint |= part_two >> ((len_byte / 2) + 1);
            byte_write = byte_joint;
            
            if (check_count_shift_last_write(stream, &byte_write, byte_joint, 
                                             &byte_shift, &count_shift)) {
                return EOF;
            }
        }
    } else {
        byte_write = byte_shift;
    }

    if (search_mask_byte_write(&byte_write)) {
        count_shift++;

        if (count_shift == len_byte) {
            count_shift = 0;
            if (putc(byte_write, stream) == EOF) {
                error("Cannot write byte\n");
                return EOF;
            }
            byte_write = zero;
        }
    }

    if (write_end_message(stream, byte_write, count_shift)) {
        return EOF;
    }

    return count_byte_write;
}

int read_message(FILE *stream, void *buf) {
    uint8_t byte_read = zero; 
    int count_bits_read = 0; // count significant bits in byte_read
    uint8_t byte_check_units = zero;
    int count_bits_check = 0; // count significant bits in byte_check_units
    int symbol_read = 0;

    for (int i = 0; (symbol_read = getc(stream)) != EOF; ++i) {
        uint8_t byte_read_tmp = (uint8_t) symbol_read;
        int index_start_read = EOF;

        for (int index = 7; index >= 0; index--) {
            if (((byte_read_tmp >> index) & unit) == zero) {
                if (count_bits_check) {
                    byte_check_units = zero;
                    count_bits_check = 1;
                    index_start_read = index;
                }
                count_bits_check = 1;
            } else if (count_bits_read) {
                byte_check_units = (byte_check_units << 1) | unit;
                count_bits_check++;
            }
        }

        for (int index = index_start_read; index >= 0; index--) {
            if (((byte_read_tmp >> index) & unit) == unit) {
                byte_check_units = (byte_check_units << 1) | unit;
                count_bits_check++;
                byte_read = (byte_read << 1) | unit;
                count_bits_read++;
            } else {
                if (count_bits_check == (len_byte - 1)) {
                    if (!count_bits_read) {
                        error("The payload contains a non-integer number of bytes\n");
                        return EOF;
                    }
                }

                if ((byte_check_units & mask) != mask) {
                    count_bits_read++;
                    byte_read <<= 1;
                }

                byte_check_units = zero;
                count_bits_check = 1;
            }
        }

        if (index_start_read != EOF) {
            break;
        }
    }
    if (feof(stream)) {
        error("Cannot read start marker\n");
        return EOF;
    }

    uint8_t *buffer = (uint8_t *) buf;
    int count_byte_read = 0;

    for (int i = 0; (symbol_read = getc(stream)) != EOF; ++i) {
        uint8_t byte_read_tmp = (uint8_t) symbol_read;

        for (int index = 7; index >= 0; index--) {
            if (count_bits_check == len_incorrect_byte 
            && (byte_check_units & incorrect_byte) == incorrect_byte) {
                error("Incorrect bit sequence\n");
                return EOF;
            }
            
            if (((byte_read_tmp >> index) & unit) == unit) {
                byte_check_units = (byte_check_units << 1) | unit;
                count_bits_check++;
                byte_read = (byte_read << 1) | unit;
                count_bits_read++;

                if (count_bits_read == len_byte) {
                    buffer[count_byte_read++] = byte_read;
                    count_bits_read = 0;
                }
            } else {
                if (count_bits_check == (len_byte - 1)) {
                    if (!count_bits_read) {
                        error("The payload contains a non-integer number of bytes\n");
                        return EOF;
                    }

                    if (check_end_message(byte_read_tmp, index)) {
                        return EOF;
                    }

                    return count_byte_read; 
                }

                if ((byte_check_units & mask) != mask) {
                    count_bits_read++;
                    byte_read <<= 1;

                    if (count_bits_read == len_byte) {
                        buffer[count_byte_read++] = byte_read;
                        count_bits_read = 0;
                    }
                }

                byte_check_units = zero;
                count_bits_check = 1;
            }
        }
    }
    if (ferror(stream)) {
        error("Cannot read byte\n");
        return EOF;
    }

    error("Cannot read end marker\n");
    return EOF;
}

int search_mask_byte(const uint8_t byte_check) {
    for (int step = 3; step >= 0; step--) {
        if (((byte_check >> step) & mask) == mask) {
            return step;
        }
    }

    return -1;
}

int search_mask_byte_joint(uint8_t *byte_joint) {
    int step = search_mask_byte(*byte_joint);

    if (step != -1) {
        uint8_t part_one = *byte_joint << (len_byte / 2);
        uint8_t part_two = part_one << ((len_byte / 2) - step);
        part_one >>= (len_byte - ((len_byte / 2) - step));
        *byte_joint = (part_one << (len_byte - ((len_byte / 2) - step))) 
                    | (part_two >> ((len_byte / 2) - step + 1));
        return 1;
    }

    return 0;
}

int search_mask_byte_write(uint8_t *byte_write) {
    int step = search_mask_byte(*byte_write);

    if (step != -1) {
        uint8_t part_one = *byte_write >> step;
        uint8_t part_two = *byte_write << (len_byte - step);
        *byte_write = (part_one << step) | (part_two >> (len_byte - step + 1));
        return 1;
    }

    return 0;
}

int check_count_shift_after_joint(FILE *stream, uint8_t *byte_write, uint8_t *byte_joint, uint8_t *byte_shift, const uint8_t byte_buffer, int *count_shift) {
    if (*count_shift == len_byte) {
        *count_shift = 0;
        *byte_shift = 0;
        
        if (search_mask_byte_write(byte_write)) {
            (*count_shift)++;
            if ((*byte_write & mask) != mask) {
                *byte_shift = *byte_joint << (len_byte - *count_shift);
            }
        }

        if (putc(*byte_write, stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }

        *byte_joint = (*byte_write << (len_byte / 2)) | (*byte_shift >> (len_byte / 2))
                    | (byte_buffer >> ((len_byte / 2) + *count_shift));

        if (search_mask_byte_joint(byte_joint)) {
            (*count_shift)++;
            uint8_t part_two = byte_buffer << ((len_byte / 2) - (*count_shift - 1));
            *byte_joint |= part_two >> ((len_byte / 2) - (*count_shift - 1) + *count_shift);
        } else {
            *byte_write = *byte_shift | (byte_buffer >> *count_shift);
        }
    }

    return 0;
}

int check_count_shift_after_write(FILE *stream, uint8_t *byte_write, uint8_t *byte_joint, uint8_t *byte_shift, int *count_shift) {
    if (*count_shift == len_byte) {
        *byte_joint = (*byte_write << (len_byte / 2)) | (*byte_shift >> (len_byte / 2));

        if (search_mask_byte_joint(byte_joint)) {
            *count_shift = 1;
            uint8_t part_two = *byte_shift << (len_byte / 2);
            *byte_joint |= part_two >> ((len_byte / 2) + 1);
            *byte_write = *byte_joint;
            *byte_joint = *byte_write;
            *byte_shift <<= (len_byte - (*count_shift + 1));
        } else {
            *count_shift = 0;
            *byte_write = *byte_shift;
            *byte_shift <<= (len_byte - (*count_shift + 1));
        }

        if (search_mask_byte_write(byte_write)) {
            (*count_shift)++;
            if ((*byte_write & mask) == mask) {
                if (*count_shift == 1) {
                    *byte_shift = 0;
                } else {
                    *byte_shift <<= (*count_shift - 1);
                    *byte_shift >>= (*count_shift - 1);
                }
            }
        } else {
            *byte_shift <<= 1;
        }

        if (putc(*byte_write, stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
    }

    return 0;
}

int check_count_shift_last_write(FILE *stream, uint8_t *byte_write, const uint8_t byte_joint, uint8_t *byte_shift, int *count_shift) {
    if (*count_shift == len_byte) {
        *count_shift = 0;
        *byte_shift = 0;

        if (search_mask_byte_write(byte_write)) {
            (*count_shift)++;
            if ((*byte_write & mask) == mask) {
                *byte_shift = 0;
            } else {
                *byte_shift = byte_joint << (len_byte - *count_shift);
            }
        }

        if (putc(*byte_write, stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
    }

    return 0;
}

int write_end_message(FILE *stream, const uint8_t byte_write, const int count_shift) {
    if (count_shift) {
        if (putc(byte_write | (marker >> count_shift), stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
        if (putc((marker << (len_byte - count_shift)) 
               | (spare_units >> count_shift), stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
        return 0;
    }

    if (putc(marker, stream) == EOF) {
        error("Cannot write byte\n");
        return EOF;
    }

    return 0;
}

int check_end_message(const uint8_t byte_end_message, const int len_spare_units) {
    for (int index = 0; index < len_spare_units; ++index) {
        if ((byte_end_message & unit) != unit) {
            error("Incorrect end message\n");
            return EOF;
        }
    }

    return 0;
}
