[24bcs056@mepcolinux ex3]$cat ham.c
#include <stdio.h>
#include <string.h>

/* Helper: Returns 1 if pos is a power of 2 (1, 2, 4, 8...) */
int isP2(int pos) {
    return pos > 0 && (pos & (pos - 1)) == 0;
}

/* Renders the visual frame layout matching your board style */
void show(int bits[], int len, int pCount) {
    printf("\nPos : "); for (int i = len; i >= 1; i--) printf("%3d", i);
    printf("\nTyp : ");
    for (int i = len; i >= 1; i--) {
        if (isP2(i)) {
            int k = 0, tmp = i; while (tmp > 1) { tmp >>= 1; k++; }
            printf(" P%d", k + 1);
        } else {
            /* Calculates data label index dynamically */
            int dNum = 0; for (int j = 1; j <= i; j++) if (!isP2(j)) dNum++;
            printf(" D%d", dNum);
        }
    }
    printf("\nVal : "); for (int i = len; i >= 1; i--) printf("%3d", bits[i]);
    printf("\n");
}

int main(void) {
    int choice, mode, len, p;
    char str[128];

    while (1) {
        /* Reset tracking variables for each iteration */
        len = 0;
        p = 0;
        int bits[128] = {0};

        printf("\n=========================\n");
        printf("1. Encode\n2. Decode\n3. Exit\nChoice: ");
        if (scanf("%d", &choice) != 1) {
            /* Clear input buffer if user types non-numeric characters */
            while (getchar() != '\n');
            continue;
        }

        if (choice == 3) {
            printf("Exiting program.\n");
            break;
        }
        if (choice != 1 && choice != 2) {
            printf("Invalid choice. Try again.\n");
            continue;
        }

        printf("Parity (0=Even, 1=Odd): ");
        if (scanf("%d", &mode) != 1) {
            while (getchar() != '\n');
            continue;
        }

        printf("Enter string: ");
        scanf("%s", str);

        if (choice == 1) {
            int n = strlen(str);
            while ((1 << p) < n + p + 1) p++; /* Find p bits */
            len = n + p;

            /* Map data string characters right-to-left into non-power-of-2 slots */
            int idx = 0;
            for (int i = len; i >= 1; i--) {
                if (!isP2(i)) bits[i] = str[idx++] - '0';
            }

            /* Calculate parity bits */
            for (int k = 1; k <= p; k++) {
                int pPos = 1 << (k - 1), sum = 0;
                for (int i = 1; i <= len; i++) {
                    if ((i & pPos) && i != pPos) sum += bits[i];
                }
                bits[pPos] = (sum % 2 == mode) ? 0 : 1;
            }
            printf("\nEncoded Frame:");
            show(bits, len, p);
        }
        else if (choice == 2) {
            len = strlen(str);
            while ((1 << p) < len + 1) p++;

            /* Read raw incoming bits straight into array positions */
            for (int i = len, idx = 0; i >= 1; i--, idx++) bits[i] = str[idx] - '0';

            /* Calculate error syndrome vector */
            int syndrome = 0;
            for (int k = 1; k <= p; k++) {
                int pPos = 1 << (k - 1), sum = 0;
                for (int i = 1; i <= len; i++) {
                    if (i & pPos) sum += bits[i];
                }
                if (sum % 2 != mode) syndrome += pPos;
            }

            /* Error detection & correction execution */
            if (syndrome == 0) printf("\nNo errors.\n");
            else if (syndrome <= len) {
                printf("\nError found at position %d! Fixing bit.\n", syndrome);
                bits[syndrome] ^= 1; /* Quick bit-flip */
            } else printf("\nMultiple severe corruption errors suspected.\n");

            printf("\nCorrected Frame:");
            show(bits, len, p);

            printf("\nExtracted Data: ");
            for (int i = len; i >= 1; i--) if (!isP2(i)) printf("%d", bits[i]);
            printf("\n");
        }
    }
    return 0;
}
