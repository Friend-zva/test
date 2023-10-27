#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_ARRAY_LENGTH 100 // guaranteed less than 100 input numbers
#define FROM_LENGTH 7 // length string "--from="
#define TO_LENGTH 5 // length string "--to="

int read_parameters(int argc, char **argv, bool *is_lower_parameter, bool *is_upper_parameter, long long int *lower_parameter, long long int *upper_parameter);

void read_array(long long int *array_stdin, long long int *array_stdout, long long int *array_stderr, int *index_array_stdin, int *index_array_stdout, int *index_array_stderr, bool is_lower_parameter, bool is_upper_parameter, long long int lower_parameter, long long int upper_parameter);

void sort_array(long long int *array_sorted, int index_array_stdin);

void swap_element(long long int *number1, long long int *number2);

int main(int argc, char **argv) {
    if (argc == 1) {
        return -1; 
    }
    if (argc > 3) {
        return -2;
    }

    long long int lower_parameter = 0, upper_parameter = 0;
    bool is_lower_parameter = false, is_upper_parameter = false;
    if (read_parameters(argc, argv, &is_lower_parameter, &is_upper_parameter, &lower_parameter, &upper_parameter)) {
        if (is_lower_parameter == true || is_upper_parameter == true) {
            return -3;
        } else {
            return -4;
        }
    }

    long long int *array_stdin = (long long int *)malloc(sizeof(long long int) * MAX_ARRAY_LENGTH);
    long long int *array_stdout = (long long int *)malloc(sizeof(long long int) * MAX_ARRAY_LENGTH);
    long long int *array_stderr = (long long int *)malloc(sizeof(long long int) * MAX_ARRAY_LENGTH);
    int index_array_stdin = 0, index_array_stdout = 0, index_array_stderr = 0;
    read_array(array_stdin, array_stdout, array_stderr, &index_array_stdin, &index_array_stdout, &index_array_stderr, is_lower_parameter, is_upper_parameter, lower_parameter, upper_parameter);

    if (index_array_stdout != 0) {
        for (int i = 0; i < index_array_stdout; ++i) {
            printf("%lld ", array_stdout[i]);
        }
    }
    free(array_stdout);

    if (index_array_stderr != 0) {
        for (int i = 0; i < index_array_stderr; ++i) {
            fprintf(stderr, "%lld ", array_stderr[i]);
        }
    }
    free(array_stderr);

    if (index_array_stdin == 0) {
        return 0;
    }
    array_stdin = (long long int *)realloc(array_stdin, sizeof(long long int) * index_array_stdin);

    long long int *array_sorted = (long long int *)malloc(sizeof(long long int) * index_array_stdin);
    for (int i = 0; i < index_array_stdin; ++i) {
        array_sorted[i] = array_stdin[i];
    }
    sort_array(array_sorted, index_array_stdin);

    int count_changes = 0;
    for (int i = 0; i < index_array_stdin; ++i) {
        if (array_sorted[i] != array_stdin[i]) {
            count_changes++;
        }
    }

    free(array_stdin);
    free(array_sorted);
    return count_changes;
}

int read_parameters(int argc, char **argv, bool *is_lower_parameter, bool *is_upper_parameter, long long int *lower_parameter, long long int *upper_parameter) {
    if (argc == 2) {
        if ((strncmp(argv[1], "--from=", FROM_LENGTH)) == 0) {
            sscanf(argv[1], "--from=%lld", lower_parameter);
            *is_lower_parameter = true;
        } else if ((strncmp(argv[1], "--to=", TO_LENGTH)) == 0) {
            sscanf(argv[1], "--to=%lld", upper_parameter);
            *is_upper_parameter = true;
        } else {
            return -1;
        }

    } else {
        if (strncmp(argv[1], "--from=", FROM_LENGTH) == 0) {
            sscanf(argv[1], "--from=%lld", lower_parameter);
            *is_lower_parameter = true;

            if ((strncmp(argv[2], "--to=", TO_LENGTH)) == 0) {
                sscanf(argv[2], "--to=%lld", upper_parameter);
                *is_upper_parameter = true;
            } else if (strncmp(argv[2], "--from=", FROM_LENGTH) == 0) {
                sscanf(argv[2], "--from=%lld", lower_parameter);
                *is_lower_parameter = true;
                return -1;
            }
        } else if ((strncmp(argv[1], "--to=", TO_LENGTH)) == 0) {
            sscanf(argv[1], "--to=%lld", upper_parameter);
            *is_upper_parameter = true;

            if (strncmp(argv[2], "--from=", FROM_LENGTH) == 0) {
                sscanf(argv[2], "--from=%lld", lower_parameter);
                *is_lower_parameter = true;
            } else if ((strncmp(argv[2], "--to=", TO_LENGTH)) == 0) {
                sscanf(argv[2], "--to=%lld", upper_parameter);
                *is_upper_parameter = true;
                return -1;
            }
        } else {
            return -1;
        }
    }

    return 0;
}

void read_array(long long int *array_stdin, long long int *array_stdout, long long int *array_stderr, int *index_array_stdin, int *index_array_stdout, int *index_array_stderr, bool is_lower_parameter, bool is_upper_parameter, long long int lower_parameter, long long int upper_parameter) {
    long long int number_analyzed = 0;
    do {
        int scanf_element = scanf("%lld", &number_analyzed);
        if (scanf_element == 0 || scanf_element == EOF) {
            break;
        }
        
        if (is_lower_parameter == true && is_upper_parameter == true) {
            if (lower_parameter < number_analyzed && number_analyzed < upper_parameter) {
                array_stdin[(*index_array_stdin)++] = number_analyzed;
            } else {
                if (number_analyzed <= lower_parameter) {
                    array_stdout[(*index_array_stdout)++] = number_analyzed;
                }
                if (upper_parameter <= number_analyzed) {
                    array_stderr[(*index_array_stderr)++] = number_analyzed;
                }
            }
        } else if (is_lower_parameter == true) {
            if (lower_parameter < number_analyzed) {
                array_stdin[(*index_array_stdin)++] = number_analyzed;
            } else {
                array_stdout[(*index_array_stdout)++] = number_analyzed;
            }
        } else {
            if (upper_parameter < number_analyzed) {
                array_stdin[(*index_array_stdin)++] = number_analyzed;
            } else {
                array_stderr[(*index_array_stderr)++] = number_analyzed;
            }
        }
    } while (getchar() != '\n');
}
