#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define error(...) (fprintf(stderr, __VA_ARGS__))

// 2,147,483,647
#define DEGREE 9
#define MODULE 1000000000 // 10 ^ DEGREE
typedef struct list_bigint_t {
    uint32_t number;
    struct list_bigint_t *next;
} list_bigint_s;

typedef struct bigint_t {
    list_bigint_s *list_bigint;
    size_t count_parts_number;
} bigint_s;

int read_arguments(int argc, char **argv);

bigint_s *create_bigint(char *str);

void reverse_string(char *tmp_str, int index_tmp_str); // xor

list_bigint_s *push_back(bigint_s *bigint, int tmp_number, list_bigint_s *back);

void print_bigint(list_bigint_s *head);

bigint_s *add_two_arguments(bigint_s *first_bigint, bigint_s *second_bigint);

int main(int argc, char **argv) {
    if (read_arguments(argc, argv)) {
        return error("Cannot read");
    }

    return 0;
}

bigint_s *add_two_arguments(bigint_s *first_bigint, bigint_s *second_bigint) {
    bigint_s *result_bigint = (bigint_s *)malloc(sizeof(bigint_s));
    list_bigint_s *back;
    uint32_t number_correction = 0;

    list_bigint_s *first_bigint_part = (list_bigint_s *)malloc(sizeof(list_bigint_s));
    list_bigint_s *second_bigint_part = (list_bigint_s *)malloc(sizeof(list_bigint_s));
    first_bigint_part = first_bigint->list_bigint;
    second_bigint_part = second_bigint->list_bigint;

    while (first_bigint_part != NULL || second_bigint_part != NULL) {
        uint32_t tmp_number = (first_bigint_part->number + second_bigint_part->number 
                                                        + number_correction) % MODULE;
        back = push_back(result_bigint, tmp_number, back);
        (result_bigint->count_parts_number)++;

        number_correction = (first_bigint_part->number + second_bigint_part->number) / (MODULE);
        first_bigint_part = first_bigint_part->next;
        second_bigint_part = second_bigint_part->next;
    }

    free(first_bigint_part);
    free(second_bigint_part);
    return result_bigint;
}

int read_arguments(int argc, char **argv) {
    int index = 0;
    bigint_s *queue[2]; 

    for (int element = 1; element < argc; element++) {
        if ((argv[element][0]) == '+') {
            printf("+\n");
        }
        else if ((argv[element][0]) == '-') {
            printf("-\n");
        }
        else {
            queue[index++] = create_bigint(argv[element]);

            print_bigint(queue[index - 1]->list_bigint);
            printf("%zu\n", queue[index - 1]->count_parts_number);
            printf("\n");
        }
    }

    bigint_s *print = add_two_arguments(queue[0], queue[1]);
    print_bigint(print->list_bigint);
    return 0;
}

bigint_s *create_bigint(char *str) {
    size_t length_string = strlen(str);
    char *tmp_str = (char *)calloc(DEGREE, sizeof(char));
    int len_tmp_str = 0;

    bigint_s *bigint = (bigint_s *)malloc(sizeof(bigint_s));
    bigint->count_parts_number = 0;
    list_bigint_s *back;

    for (int index = (length_string - 1); index >= 0; index--) {
        tmp_str[len_tmp_str++] = str[index];

        if (len_tmp_str == DEGREE) {
            reverse_string(tmp_str, len_tmp_str - 1);
            back = push_back(bigint, atoi(tmp_str), back);
            // if (bigint->count_parts_number == 0) { 
            //     bigint->list_bigint = (list_bigint_s *)malloc(sizeof(list_bigint_s));
            //     bigint->list_bigint->number = atoi(tmp_str);
            //     back = bigint->list_bigint;
            // } else {
            //     list_bigint_s *list_bigint = (list_bigint_s *)malloc(sizeof(list_bigint_s));
            //     list_bigint->number = atoi(tmp_str);
            //     back->next = list_bigint;
            //     back = list_bigint;
            // }
            (bigint->count_parts_number)++;

            len_tmp_str = 0;
            memset(tmp_str, 0, DEGREE);
        }
    }

    if (len_tmp_str != 0) {
        reverse_string(tmp_str, len_tmp_str - 1);
        back = push_back(bigint, atoi(tmp_str), back);
        // list_bigint_s *list_bigint = (list_bigint_s *)malloc(sizeof(list_bigint_s));
        // list_bigint->number = atoi(tmp_str);
        // back->next = list_bigint;
        // back = list_bigint;
        (bigint->count_parts_number)++;
    }

    free(tmp_str);
    return bigint;
}

void reverse_string(char *str, int len_str) {
    char tmp_symbol;

    for (int index = 0; index <= (len_str / 2); index++) {
        tmp_symbol = str[index];
        str[index] = str[len_str - index];
        str[len_str - index] = tmp_symbol;
    }
}

list_bigint_s *push_back(bigint_s *bigint, int tmp_number, list_bigint_s *back) {
    if (bigint->count_parts_number == 0) {
        bigint->list_bigint = (list_bigint_s *)malloc(sizeof(list_bigint_s));
        bigint->list_bigint->number = tmp_number;

        return bigint->list_bigint;
    } else {
        list_bigint_s *list_bigint = (list_bigint_s *)malloc(sizeof(list_bigint_s));
        list_bigint->number = tmp_number;
        back->next = list_bigint;
        
        return list_bigint;
    }
}

void print_bigint(list_bigint_s *head) {
    while (head != NULL) {
        printf("%u\n", head->number);
        head = head->next;
    }
}
