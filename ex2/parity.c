#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR 100
#define MAX_BITS 1024
#define FRAME_SIZE 7

void ascii_to_bin(char ch, char *b) {
    for (int i = 7; i >= 0; i--) {
        b[7 - i] = (ch & (1 << i)) ? '1' : '0';
    }
    b[8] = '\0';
}

void str_to_bits(const char *str, char *bits) {
    int len = strlen(str);
    char b[9];

    bits[0] = '\0';

    for (int i = 0; i < len; i++) {
        ascii_to_bin(str[i], b);
        strcat(bits, b);
    }
}

int ask_parity_scheme() {
    int choice;

    printf("Choose parity scheme:\n");
    printf("1. Even Parity\n");
    printf("2. Odd Parity\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    return (choice == 2) ? 1 : 0;
}

char compute_parity_bit(int ones_in_data, int scheme) {
    if (scheme == 0)
        return (ones_in_data % 2 == 0) ? '0' : '1';
    else
        return (ones_in_data % 2 == 0) ? '1' : '0';
}

int check_parity_error(int total_ones, int scheme) {
    if (scheme == 0)
        return (total_ones % 2 != 0);
    else
        return (total_ones % 2 == 0);
}

/* Convert 8 binary bits into ASCII character */
char bin_to_ascii(const char *bits) {
    int value = 0;

    for (int i = 0; i < 8; i++) {
        value = value * 2 + (bits[i] - '0');
    }

    return (char)value;
}

/* Extract binary data from received frames */
void extract_received_data(char tx_frames[][16],
                           int num_frames,
                           int total_bits,
                           char *received_bits) {

    int pos = 0;

    for (int i = 0; i < num_frames; i++) {

        /* Copy only the 7 data bits.
           The 8th bit is parity, so ignore it. */
        for (int j = 0; j < FRAME_SIZE; j++) {

            if (pos < total_bits) {
                received_bits[pos] = tx_frames[i][j];
                pos++;
            }
        }
    }

    received_bits[pos] = '\0';
}

/* Convert received binary data back to string */
void bits_to_string(const char *bits, char *str) {

    int bit_len = strlen(bits);
    int char_count = bit_len / 8;

    for (int i = 0; i < char_count; i++) {

        char byte[9];

        for (int j = 0; j < 8; j++) {
            byte[j] = bits[i * 8 + j];
        }

        byte[8] = '\0';

        str[i] = bin_to_ascii(byte);
    }

    str[char_count] = '\0';
}

int main() {

    char input[MAX_STR];
    char full_msg[MAX_BITS];

    printf("\n========== SENDER SIDE ==========\n");

    printf("Enter string message: ");
    scanf("%99s", input);

    int scheme = ask_parity_scheme();

    str_to_bits(input, full_msg);

    int total_bits = strlen(full_msg);

    printf("\nFull Binary Message:\n%s\n", full_msg);

    int num_frames = (total_bits + FRAME_SIZE - 1) / FRAME_SIZE;

    char tx_frames[100][16];

    printf("\n========== FRAME GENERATION ==========\n");

    for (int i = 0; i < num_frames; i++) {

        char current_frame[8] = {0};
        int ones = 0;

        for (int j = 0; j < FRAME_SIZE; j++) {

            int idx = i * FRAME_SIZE + j;

            if (idx < total_bits)
                current_frame[j] = full_msg[idx];
            else
                current_frame[j] = '0';

            if (current_frame[j] == '1')
                ones++;
        }

        current_frame[FRAME_SIZE] = '\0';

        char parity_bit = compute_parity_bit(ones, scheme);

        sprintf(tx_frames[i], "%s%c", current_frame, parity_bit);

        printf("Frame %2d : Data = %s | Parity = %c | Sent = %s\n",
               i + 1,
               current_frame,
               parity_bit,
               tx_frames[i]);
    }

    printf("\n========== RECEIVER SIDE ==========\n");

    int frame_no;

    printf("Enter frame number to corrupt (1-%d)\n", num_frames);
    printf("Enter -1 for NO ERROR: ");
    scanf("%d", &frame_no);

    if (frame_no >= 1 && frame_no <= num_frames) {

        int flips;

        printf("\nHow many bits do you want to flip? ");
        scanf("%d", &flips);

        int frame_index = frame_no - 1;

        for (int i = 0; i < flips; i++) {

            int bit_pos;

            printf("Enter bit position %d to flip (0-%d): ",
                   i + 1, FRAME_SIZE);

            scanf("%d", &bit_pos);

            if (bit_pos >= 0 && bit_pos <= FRAME_SIZE) {

                tx_frames[frame_index][bit_pos] =
                    (tx_frames[frame_index][bit_pos] == '0')
                        ? '1'
                        : '0';

                printf("Bit %d flipped successfully.\n", bit_pos);
            }
            else {
                printf("Invalid bit position!\n");
                i--;
            }
        }

        printf("\nCorrupted Frame %d : %s\n",
               frame_no,
               tx_frames[frame_index]);
    }
    else {
        printf("\nNo error introduced.\n");
    }

    printf("\n========== RECEIVER VERIFICATION ==========\n");

    int global_error = 0;

    for (int i = 0; i < num_frames; i++) {

        int ones = 0;

        for (int j = 0; j < strlen(tx_frames[i]); j++) {

            if (tx_frames[i][j] == '1')
                ones++;
        }

        int error = check_parity_error(ones, scheme);

        printf("Frame %2d : %s | Total 1s = %d | %s\n",
               i + 1,
               tx_frames[i],
               ones,
               error ? "ERROR DETECTED" : "OK");

        if (error)
            global_error = 1;
    }

    printf("\n========== FINAL RESULT ==========\n");

    if (global_error) {

        printf("TRANSMISSION FAILED (Error Detected)\n");

        printf("\nReceiver data is NOT extracted because\n");
        printf("the received transmission contains an error.\n");
    }
    else {

        printf("TRANSMISSION SUCCESSFUL (No Error Detected)\n");

        /* ==========================================
           EXTRACT RECEIVED DATA
           ========================================== */

        char received_bits[MAX_BITS];
        char received_string[MAX_STR];

        extract_received_data(
            tx_frames,
            num_frames,
            total_bits,
            received_bits
        );

        bits_to_string(
            received_bits,
            received_string
        );

        printf("\n========== RECEIVER DATA EXTRACTION ==========\n");

        printf("Received Data Bits:\n%s\n", received_bits);

        printf("\nExtracted Receiver Data:\n");
        printf("%s\n", received_string);

        /* Compare sender and receiver data */

        if (strcmp(input, received_string) == 0) {
            printf("\nData Verification: SUCCESS\n");
            printf("Sender Data = Receiver Data\n");
        }
        else {
            printf("\nData Verification: FAILED\n");
            printf("Sender Data != Receiver Data\n");
        }
    }

    printf("\nNOTE:\n");
    printf("Parity Check detects ODD number of bit errors.\n");
    printf("Parity Check CANNOT detect EVEN number of bit errors.\n");

    return 0;
}
