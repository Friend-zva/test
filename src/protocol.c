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
    uint8_t byte_write = 0;
    uint8_t byte_joint = 0;
    uint8_t byte_shift = 0;
    int count_shift = 0;

    for (size_t index = 0; index < nbyte; ++index) {
        byte_joint = byte_write | (byte_shift >> (len_byte / 2)) 
                                | (buffer[index] >> ((len_byte / 2) + count_shift));
        
        if (search_mask_byte_joint(&byte_joint)) {
            count_shift++;

            if (count_shift <= 4) {
                uint8_t part_two = buffer[index] << ((len_byte / 2) - (count_shift - 1));
                byte_joint |= part_two >> ((len_byte / 2) - (count_shift - 1) + count_shift);
            } else {
                uint8_t part_two = byte_shift << (len_byte / 2);
                byte_joint |= part_two >> ((len_byte / 2) + 1) | buffer[index] >> (count_shift);
            }
            
            byte_write = byte_joint;
        } else {
            byte_write = byte_shift | (buffer[index] >> count_shift);
        }

        if (search_mask_byte_write(&byte_write)) {
            if ((byte_write & mask) == mask) {
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

        if (putc(byte_write, stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
        count_byte_write++;

        // if (check_count_shift(stream, &count_shift, &byte_shift)) {
        //     byte_write = byte_shift;
        // }

        byte_write <<= (len_byte / 2);
    }
    byte_joint = byte_write | (byte_shift >> (len_byte / 2));
    
    if (search_mask_byte_joint(&byte_joint)) {
        count_shift++;
        if (count_shift > 4) {
            uint8_t part_two = byte_shift << (len_byte / 2);
            byte_joint |= part_two >> ((len_byte / 2) + 1);

            if (search_mask_byte_write(&byte_joint)) {
                count_shift++;
            }
        }

        byte_write = byte_joint | (marker >> count_shift);
    } else {
        byte_write = (byte_joint << (len_byte / 2)) | (marker >> count_shift);
    }

    if (write_end_message(stream, count_shift, byte_write, byte_shift)) {
        return EOF;
    }

    return count_byte_write;
}

int read_message(FILE *stream, void *buf) {
    uint8_t byte_read = 0;
    int count_shift = 0;

    if (read_start_message(stream, &byte_read, &count_shift)) {
        return EOF;
    }
    
    uint8_t *buffer = (uint8_t *) buf;
    int count_byte_read = 0;
    int symbol_read = 0;
    uint8_t byte_shift = 0;

    if (!byte_read) {
        if ((symbol_read = getc(stream)) != EOF) {
            if (symbol_read == marker) {
                return count_byte_read;
            }
            if (search_byte_incorrect((uint8_t) symbol_read)) {
                return EOF;
            }

            byte_read = (uint8_t) symbol_read;
        } else {
            error("Cannot read byte\n");
            return EOF;
        }
    }
    
    for (int i = 0; (symbol_read = getc(stream)) != EOF; ++i) {
        byte_shift = ((uint8_t) symbol_read) >> (len_byte - count_shift);
        
        if ((byte_read | byte_shift) == marker) {
            uint8_t byte_units = spare_units >> (len_byte - count_shift);
            
            if (((uint8_t) symbol_read & byte_units) == byte_units) {
                return count_byte_read;
            } else {
                error("The payload contains a non-integer number of bytes\n");
                return EOF;
            }
        }

        if (search_byte_incorrect(byte_read | byte_shift)) {
            return EOF;
        }

        byte_shift = (uint8_t) symbol_read >> (len_byte - count_shift);
        byte_read |= byte_shift;
        
        if (search_mask_byte_read(&byte_read)) {
            count_shift++;
            if (count_shift == len_byte) {
                count_shift = 0;
            }
        }

        buffer[count_byte_read++] = byte_read;
        byte_read = ((uint8_t) symbol_read) << count_shift;
    }
    if (ferror(stream)) {
        error("Cannot read byte\n");
        return EOF;
    }

    error("Cannot read start marker\n");
    return EOF;
}

int search_mask_byte(const uint8_t byte_check) {
    for (int cycle = 3; cycle >= 0; cycle--) {
        if (((byte_check >> cycle) & mask) == mask) {
            return cycle;
        }
    }

    return -1;
}

int search_mask_byte_joint(uint8_t *byte_joint) {
    int cycle = search_mask_byte(*byte_joint);

    if (cycle != -1) {
        uint8_t part_one = *byte_joint << (len_byte / 2);
        uint8_t part_two = part_one << ((len_byte / 2) - cycle);
        part_one >>= (len_byte - ((len_byte / 2) - cycle));
        *byte_joint = part_one << (len_byte - ((len_byte / 2) - cycle)) | part_two >> ((len_byte / 2) - cycle + 1);
        return 1;
    }

    return 0;
}

// int check_count_shift(FILE *stream, int *count_shift, uint8_t *byte_shift) {
//     if (*count_shift == len_byte) {
//         if (search_mask_byte_write(byte_shift)) {
//             if (putc(*byte_shift, stream) == EOF) {
//                 error("Cannot write byte\n");
//                 return EOF;
//             }
//             *count_shift = 1;
//             *byte_shift <<= (len_byte - *count_shift);
//         } else {
//             if (putc(*byte_shift, stream) == EOF) {
//                 error("Cannot write byte\n");
//                 return EOF;
//             }
//             *count_shift = 0;
//             *byte_shift = 0;
//         }

//         return 1;
//     }

//     return 0;
// }

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

int write_end_message(FILE *stream, const int count_shift, 
                      const uint8_t byte_write, const uint8_t byte_shift) {
    if (count_shift == 0) {
        if (putc(marker, stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
    } else if (count_shift == len_byte) {
        if (putc(byte_write, stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
        if (putc(marker, stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
    } else {
        if (putc(byte_shift | marker >> count_shift, stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
        if (putc(marker << (len_byte - count_shift) 
              | (spare_units >> count_shift), stream) == EOF) {
            error("Cannot write byte\n");
            return EOF;
        }
    }

    return 0;
}

int read_start_message(FILE *stream, uint8_t *byte_read, int *count_shift) {
    int symbol_read = 0;

    for (int i = 0; (symbol_read = getc(stream)) != EOF; ++i) {
        if ((((uint8_t) symbol_read) >> *count_shift | *byte_read) == marker) {
            if (*count_shift) {
                *count_shift = (len_byte - *count_shift);
                *byte_read = ((uint8_t) symbol_read) << *count_shift;
            }
            return 0;
        }
        if (*count_shift) {
            break;
        }

        for (int cycle = 7; cycle >= 0; cycle--) {
            if ((((uint8_t) symbol_read >> cycle) & 0x01) == 0x00) {
                *count_shift = cycle + 1;
                *byte_read = ((uint8_t) symbol_read) << (len_byte - *count_shift);
                break;
            }
        }
    }
    if (ferror(stream)) {
        error("Cannot read byte\n");
        return EOF;
    }

    error("Cannot read start marker\n");
    return EOF;
}

int search_byte_incorrect(uint8_t byte_incorrect) {
    for (int i = 0; i < 2; ++i) {
        if ((byte_incorrect >> i & spare_units >> 2) == spare_units >> 2) {
            error("Incorrect bit sequence\n");
            return EOF;
        }
    }

    return 0;
}

int search_mask_byte_read(uint8_t *byte_read) {
    int cycle = search_mask_byte(*byte_read);

    if (cycle != -1) {
        uint8_t part_one = *byte_read >> cycle;
        uint8_t part_two = *byte_read << (len_byte - cycle + 1);
        *byte_read = (part_one << cycle) | (part_two >> (len_byte - cycle));
        return 1;
    }

    return 0;
}
