#pragma once

#include <stdio.h>
#include <inttypes.h>

#define error(...) (fprintf(stderr, __VA_ARGS__))

#define marker 0x7e
#define mask 0x1f
#define spare_units 0xff
#define incorrect_byte 0x3f
#define zero 0x00
#define unit 0x01

#define len_byte 8
#define len_incorrect_byte 6
#define MAX_MESSAGE_LEN 256

int write_message(FILE *stream, const void *buf, size_t nbyte);

int read_message(FILE *stream, void *buf);

int search_mask_byte(const uint8_t byte_check);

int search_mask_byte_joint(uint8_t *byte_joint);

int search_mask_byte_write(uint8_t *byte_write);

int check_count_shift_after_joint(FILE *stream, uint8_t *byte_write, uint8_t *byte_joint, uint8_t *byte_shift, const uint8_t byte_buffer, int *count_shift);

int check_count_shift_after_write(FILE *stream, uint8_t *byte_write, uint8_t *byte_joint, uint8_t *byte_shift, int *count_shift);

int check_count_shift_last_write(FILE *stream, uint8_t *byte_write, const uint8_t byte_joint, uint8_t *byte_shift, int *count_shift);

int write_end_message(FILE *stream, const uint8_t byte_write, const int count_shift);

int read_start_message(FILE *stream, uint8_t *byte_read, int *count_bits_read);

int check_end_message(const uint8_t byte_end_message, const int len_spare_units);