#pragma once

#include <stdio.h>
#include <inttypes.h>

#define error(...) (fprintf(stdout, __VA_ARGS__))

#define MAX_MESSAGE_LEN 256

#define marker 0x7e
#define mask 0x1f
#define spare_units 0xff
#define len_byte 8

int search_mask_byte(const uint8_t byte_check);

int search_mask_byte_joint(uint8_t *byte_joint, const uint8_t byte_shift);

int search_mask_byte_write(uint8_t *byte_write);

int check_count_shift(FILE *stream, int *count_shift, uint8_t *byte_shift);

int write_end_message(FILE *stream, const int count_shift, const uint8_t byte_write, const uint8_t byte_shift);

int read_start_message(FILE *stream, uint8_t *byte_read, int *count_shift);

int search_byte_incorrect(uint8_t byte_incorrect);

int search_mask_byte_read(uint8_t *byte_read);
