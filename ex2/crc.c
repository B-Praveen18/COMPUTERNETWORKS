#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR 100
#define MAX_BITS 1024

void ascii_to_bin(char ch, char *b) {
    for (int i = 7; i >= 0; i--) {
        b[7 - i] = (ch & (1 << i)) ? '1' : '0';
    }
    b[8] = '\0';
}

char bin_to_ascii(const char *b) {
    char ch = 0;
    for (int i = 0; i < 8; i++) {
        ch = (ch << 1) | (b[i] - '0');
    }
    return ch;
}

void str_to_bits(const char *str, char *bits) {
    int len = strlen(str);
    char b[9];
    for (int i = 0; i < len; i++) {
        ascii_to_bin(str[i], b);
        strcpy(bits + (i * 8), b);
    }
    bits[len * 8] = '\0';
}

void bits_to_str(const char *bits, char *str) {
    int nchars = strlen(bits) / 8;
    for (int i = 0; i < nchars; i++) {
        str[i] = bin_to_ascii(bits + (i * 8));
    }
    str[nchars] = '\0';
}

void simulate_error(char *bits) {
    int len = strlen(bits);
    int pos;
    printf("\nSimulate a transmission error?\n");
    printf("Enter bit position to flip (0 to %d), or -1 for NO error: ", len - 1);
    scanf("%d", &pos);
    if (pos >= 0 && pos < len) {
        bits[pos] = (bits[pos] == '0') ? '1' : '0';
        printf("Bit at position %d flipped.\n", pos);
    } else {
        printf("No error introduced.\n");
    }
}

void xor_div(const char *data, const char *gen, char *remainder) {
    int data_len = strlen(data);
    int gen_len = strlen(gen);
    char *temp = (char *)malloc(data_len + 1);
    strcpy(temp, data);
    for (int i = 0; i <= data_len - gen_len; i++) {
        if (temp[i] == '1') {
            for (int j = 0; j < gen_len; j++) {
                temp[i + j] = (((temp[i + j] - '0') ^ (gen[j] - '0')) + '0');
            }
        }
    }
    strcpy(remainder, temp + (data_len - (gen_len - 1)));
    free(temp);
}

int main() {
    char input[MAX_STR];
    char data[MAX_BITS], gen[32];
    char dividend[MAX_BITS], remainder[32], transmitted[MAX_BITS];

    printf("\n---- CRC : SENDER SIDE ----\n");
    printf("Enter string message: ");
    scanf("%s", input);
    str_to_bits(input, data);
    printf("Source data (binary) : %s\n", data);

    printf("Enter generator polynomial bits (e.g. 1001), or press 0 for default 1001: ");
    scanf("%s", gen);
    if (strcmp(gen, "0") == 0) strcpy(gen, "1001");
    printf("Generator G(x) used : %s\n", gen);

    /* Check for a good generator */
    int gen_len = strlen(gen);

    // Conditions:
    // 1. Minimum 2 bits (gen_len >= 2)
    // 2. x^0 coefficient is 1 (gen[gen_len - 1] == '1')
    // 3. Last two bits of the generator are '1' (gen[gen_len - 1] == '1' && gen[gen_len - 2] == '1')
    if (gen_len >= 2 && gen[gen_len - 1] == '1' && gen[gen_len - 2] == '1') {
        printf("Generator quality : GOOD GENERATOR\n");
    } else {
        printf("Generator quality : NOT A GOOD GENERATOR\n");
    }

    int r = strlen(gen) - 1;
    strcpy(dividend, data);
    for (int i = 0; i < r; i++) dividend[strlen(data) + i] = '0';
    dividend[strlen(data) + r] = '\0';
    printf("Data padded with %d zeros : %s\n", r, dividend);

    xor_div(dividend, gen, remainder);
    printf("CRC remainder (redundant bits) : %s\n", remainder);

    strcpy(transmitted, data);
    strcat(transmitted, remainder);
    printf("Data to be transmitted (T = D+CRC): %s\n", transmitted);

    printf("\n---- CRC : RECEIVER SIDE ----\n");
    char received[MAX_BITS];
    strcpy(received, transmitted);
    simulate_error(received);
    printf("Received data : %s\n", received);

    char check_remainder[32];
    xor_div(received, gen, check_remainder);
    printf("Remainder after division by G(x) : %s\n", check_remainder);

    int all_zero = 1;
    for (int i = 0; i < (int)strlen(check_remainder); i++) {
        if (check_remainder[i] != '0') all_zero = 0;
    }

    if (all_zero) {
        printf("Result : NO ERROR DETECTED\n");
        char recovered_bits[MAX_BITS], recovered_str[MAX_STR];
        strncpy(recovered_bits, received, strlen(data));
        recovered_bits[strlen(data)] = '\0';
        bits_to_str(recovered_bits, recovered_str);
        printf("Recovered message (string) : %s\n", recovered_str);
    } else {
        printf("Result : ERROR DETECTED\n");
    }
    return 0;
}
