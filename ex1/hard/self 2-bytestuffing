#include <stdio.h>
#include <string.h>

#define MAX 100

int dataBytes[MAX];
int dataBits[MAX * 8];
int stuffedBytes[MAX * 2];
int framedBytes[MAX * 2];
int destuffedBytes[MAX];
int flagByte, escByte;

int bitsToByte(int bits[]) {
   int val = 0;
   for (int i = 0; i < 8; i++) {
      val = val * 2 + bits[i];
   }
   return val;
}

void printByteBinary(int Byte) {
   for (int i = 7; i >= 0; i--) {
      printf("%d", (Byte >> i) & 1);
   }
}

void printByteAsBinary(const char * label, int ass[], int n) {
   printf("%s: ", label);
   for (int i = 0; i < n; i++) {
      printByteBinary(ass[i]);
      printf(" ");
   }
   printf("\n");
}

int main() {
   int fbits[8], ebits[8];
   char message[MAX];
   int choice;

   printf("Enter 8 bits Flag: ");
   for (int i = 0; i < 8; i++) {
      scanf("%d", &fbits[i]);
   }
   flagByte = bitsToByte(fbits);

   printf("Enter 8 bits ESC: ");
   for (int i = 0; i < 8; i++) {
      scanf("%d", &ebits[i]);
   }
   escByte = bitsToByte(ebits);
   getchar(); // Consume trailing newline

   while (1) {

      printf("--- Menu ---\n");
      printf("1. Standard String Transmission \n");
      printf("2. Manual Byte Stream Input (Test Corrupt Framing Errors)\n");
      printf("3. Exit\n");
      printf("Enter your choice: ");

      if (scanf("%d", &choice) != 1) {
         printf("Invalid input type. Exiting.\n");
         break;
      }
      getchar(); // Clear raw newline

      if (choice == 3) {
         printf("\nExiting program. Goodbye!\n");
         break;
      }

      switch (choice) {
         case 1: {
            printf("\n--- Case 1: Standard String Transmission ---\n");
            printf("Enter Message: ");
            fgets(message, sizeof(message), stdin);
            message[strcspn(message, "\n")] = '\0';

            int numBytes = strlen(message);
            int bitIdx = 0;

            // Convert characters to bits
            for (int i = 0; i < numBytes; i++) {
               int asciiVal = (int)message[i];
               for (int b = 7; b >= 0; b--) {
                  dataBits[bitIdx++] = (asciiVal >> b) & 1;
               }
            }

            // Internal conversion back to dataBytes
            for (int i = 0; i < numBytes; i++) {
               int tmp[8];
               for (int b = 0; b < 8; b++) {
                  tmp[b] = dataBits[i * 8 + b];
               }
               dataBytes[i] = bitsToByte(tmp);
            }

            printf("\nFLAG: "); printByteBinary(flagByte); printf("\n");
            printf("ESC:  "); printByteBinary(escByte);  printf("\n\n");

            printByteAsBinary("Original Data", dataBytes, numBytes);

            // Stuffing Process
            int j = 0;
            for (int i = 0; i < numBytes; i++) {
               if (dataBytes[i] == flagByte || dataBytes[i] == escByte) {
                  stuffedBytes[j++] = escByte;
                  stuffedBytes[j++] = dataBytes[i];
               } else {
                  stuffedBytes[j++] = dataBytes[i];
               }
            }
            int stuffedLen = j;
            printByteAsBinary("Stuffed Data ", stuffedBytes, stuffedLen);

            // Framing Process
            int k = 0;
            framedBytes[k++] = flagByte;
            for (int i = 0; i < stuffedLen; i++) {
               framedBytes[k++] = stuffedBytes[i];
            }
            framedBytes[k++] = flagByte;
            int framedLen = k;

            printByteAsBinary("Framed Data  ", framedBytes, framedLen);

            // Destuffing Process
            int m = 0, error = 0;
            if (framedBytes[0] != flagByte || framedBytes[framedLen - 1] != flagByte) {
               printf("\n[DISCARDED] ERROR: Frame Missing Start/End Delimiter Flags\n");
               break;
            }

            for (int i = 1; i < framedLen - 1; i++) {
               if (framedBytes[i] == escByte) {
                  if (i + 1 >= framedLen - 1) {
                     printf("\n[DISCARDED] ERROR: Dangling ESC byte detected without an escaped following payload byte.\n");
                     error = 1;
                     break;
                  }
                  i++;
                  destuffedBytes[m++] = framedBytes[i];
               } else if (framedBytes[i] == flagByte) {
                  printf("\n[DISCARDED] ERROR: Unexpected raw FLAG byte found unescaped inside payload zone.\n");
                  error = 1;
                  break;
               } else {
                  destuffedBytes[m++] = framedBytes[i];
               }
            }

            if (!error) {
               printByteAsBinary("Destuffed Data", destuffedBytes, m);
               if (m == numBytes) {
                  int match = 1;
                  char outputString[MAX];

                  for (int i = 0; i < numBytes; i++) {
                     if (destuffedBytes[i] != dataBytes[i]) {
                        match = 0;
                        break;
                     }
                     outputString[i] = (char)destuffedBytes[i];
                  }
                  outputString[numBytes] = '\0';

                  if (match) {
                     printf("\nDestuffing successful!\n");
                     printf("Received Message : %s\n", outputString);
                  } else {
                     printf("\nMismatch after Destuffing\n");
                  }
               } else {
                  printf("\nLength mismatch after destuffing\n");
               }
            }
            break;
         }

         case 2: {
            int inputLen;
            int manualFramed[MAX * 2];
            printf("\n--- Case 2: Manual Byte Stream Input Mode ---\n");
            printf("How many bytes? ");
            if (scanf("%d", &inputLen) != 1 || inputLen <= 0 || inputLen > (MAX * 2)) {
               printf("Invalid length entered.\n");
               getchar();
               break;
            }

            printf("Enter the %d integer values\n", inputLen);
            for (int i = 0; i < inputLen; i++) {
               scanf("%d", &manualFramed[i]);
            }
            getchar(); // Clear raw buffer

            printf("\nProcessing manual byte stream...\n");
            printByteAsBinary("Input Stream Data", manualFramed, inputLen);

            // Directly run checking logic on custom byte stream
            int m = 0, error = 0;

            // Check boundary flags
            if (manualFramed[0] != flagByte || manualFramed[inputLen - 1] != flagByte) {
               printf("\n[DISCARDED] Framing Failure: Missing correct Start or End boundary marker!\n");
               printf("Detailed Description:\n");
               printf(" -> Expected Flag Byte decimal value: %d\n", flagByte);
               printf(" -> First stream element: %d, Last stream element: %d\n", manualFramed[0], manualFramed[inputLen - 1]);
               printf(" -> Framing Rules dictate frames must start and conclude exactly with the specified control Flag.\n");
               error = 1;
               break;
            }

            // Core destuff processing loop
            for (int i = 1; i < inputLen - 1; i++) {
               if (manualFramed[i] == escByte) {
                  // Check if escape is placed right at the edge of the trailing flag boundary
                  if (i + 1 >= inputLen - 1) {
                     printf("\n[DISCARDED] Protocol Failure: Dangling ESC byte detected at index %d!\n", i);
                     printf("Detailed Description:\n");
                     printf(" -> An ESC control marker (%d) was found parsing payload, but it lacks a following data byte.\n", escByte);
                     printf(" -> It directly hits the trailing frame flag boundary at index %d.\n", inputLen - 1);
                     printf(" -> This corrupt structure violates byte-stuffing syntax rules and cannot be unescaped.\n");
                     error = 1;
                     break;
                  }
                  i++; // Move to look at the escaped character
                  destuffedBytes[m++] = manualFramed[i];
               } else if (manualFramed[i] == flagByte) {
                  printf("\n[DISCARDED] Protocol Failure: Unexpected unescaped raw FLAG inside payload boundary at index %d!\n", i);
                  printf("Detailed Description:\n");
                  printf(" -> Encountered raw flag value (%d) inside data segment.\n", flagByte);
                  printf(" -> Since it lacks a prefix ESC character (%d), the receiver gets confused.\n", escByte);
                  printf(" -> It creates premature framing termination ambiguity, making the packet corrupted.\n");
                  error = 1;
                  break;
               } else {
                  destuffedBytes[m++] = manualFramed[i];
               }
            }

            if (!error) {
               printByteAsBinary("Successfully Destuffed Data", destuffedBytes, m);
               char manualOutput[MAX] = {0};
               int printable = 1;

               for (int i = 0; i < m && i < (MAX - 1); i++) {
                  if (destuffedBytes[i] >= 32 && destuffedBytes[i] <= 126) {
                     manualOutput[i] = (char)destuffedBytes[i];
                  } else {
                     printable = 0;
                  }
               }

               if (printable && m > 0) {
                  printf("Decoded String Message: %s\n", manualOutput);
               } else {
                  printf("Decoded payload contains non-printable binary control bytes.\n");
               }
            }
            break;
         }

         default:
            printf("Invalid selection. Choose option 1, 2, or 3.\n");
            break;
        }
      }
      return 0;
}
