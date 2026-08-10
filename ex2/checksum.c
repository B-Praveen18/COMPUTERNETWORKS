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

unsigned int bits_to_uint(const char *bits, int n) {
    unsigned int v = 0;
    for (int i = 0; i < n; i++) v = (v << 1) | (bits[i] - '0');
    return v;
}

void uint_to_bits(unsigned int v, int n, char *bits) {
    for (int i = n - 1; i >= 0; i--) {
        bits[i] = (v & 1) ? '1' : '0';
        v >>= 1;
    }
    bits[n] = '\0';
}

unsigned int add_1s_complement(unsigned int a, unsigned int b, int n) {
    unsigned int mask = (n >= 32) ? 0xFFFFFFFFu : ((1u << n) - 1);
    unsigned int sum = a + b;
    if (sum > mask) sum = (sum & mask) + 1;
    return sum & mask;
}

int main() {
    char input[MAX_STR];
    char bits[MAX_BITS];
    int n;

    printf("\n---- CHECKSUM : SENDER SIDE ----\n");
    printf("Enter string message: ");
    scanf("%s", input);
    str_to_bits(input, bits);

    printf("Enter block size n : ");
    scanf("%d", &n);
    if (n <= 0) n = 8;
    int len = strlen(bits);

    int pad = (n - (len % n)) % n;
    for (int i = 0; i < pad; i++) bits[len + i] = '0';
    len += pad;
    bits[len] = '\0';
    printf("Data (padded to multiple of %d) : %s\n", n, bits);

    int nblocks = len / n;
    unsigned int sum = 0;
    printf("Blocks:\n");
    for (int i = 0; i < nblocks; i++) {
        char block[33];
        strncpy(block, bits + i * n, n);
        block[n] = '\0';
        printf(" Block %d : %s\n", i + 1, block);
        unsigned int val = bits_to_uint(block, n);
        sum = add_1s_complement(sum, val, n);
    }

    char sum_bits[33];
    uint_to_bits(sum, n, sum_bits);
    printf("Sum of all blocks : %s\n", sum_bits);

    unsigned int mask = (n >= 32) ? 0xFFFFFFFFu : ((1u << n) - 1);
    unsigned int checksum = (~sum) & mask;
    char checksum_bits[33];
    uint_to_bits(checksum, n, checksum_bits);
    printf("Checksum  : %s\n", checksum_bits);

    char transmitted[MAX_BITS];
    strcpy(transmitted, bits);
    strcat(transmitted, checksum_bits);
    printf("Data to be transmitted  : %s\n", transmitted);

    printf("\n---- CHECKSUM : RECEIVER SIDE ----\n");
    char received[MAX_BITS];
    strcpy(received, transmitted);
    simulate_error(received);
    printf("Received data : %s\n", received);

    int rlen = strlen(received);
    int rblocks = rlen / n;
    unsigned int rsum = 0;
    printf("Blocks (data blocks + checksum block):\n");
    for (int i = 0; i < rblocks; i++) {
        char block[33];
        strncpy(block, received + i * n, n);
        block[n] = '\0';
        printf(" Block %d : %s\n", i + 1, block);
        unsigned int val = bits_to_uint(block, n);
        rsum = add_1s_complement(rsum, val, n);
    }

    char rsum_bits[33];
    uint_to_bits(rsum, n, rsum_bits);
    printf("Sum of all blocks : %s\n", rsum_bits);

    unsigned int rchecksum = (~rsum) & mask;
    char rchecksum_bits[33];
    uint_to_bits(rchecksum, n, rchecksum_bits);
    printf("Complement of sum (result) : %s\n", rchecksum_bits);

    if (rchecksum == 0) {
        printf("Reason : Complement of sum is all zeros -> as expected.\n");
        printf("Result : NO ERROR DETECTED\n");

        char recovered_bits[MAX_BITS], recovered_str[MAX_STR];
        strncpy(recovered_bits, received, len);
        recovered_bits[len] = '\0';
        bits_to_str(recovered_bits, recovered_str);
        printf("Recovered message (string) : %s\n", recovered_str);
    } else {
        printf("Reason : Complement of sum is NOT all zeros (%s) -> mismatch!\n", rchecksum_bits);
        printf("Result : ERROR DETECTED\n");
    }
    return 0;
}
