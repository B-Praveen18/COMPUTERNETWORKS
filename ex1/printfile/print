#include <stdio.h>
#include <string.h>

#define max 100
#define max_bit (max * 8)
#define max_stuff (max_bit * 2)

int data[max_bit], stuff[max_stuff], destuff[max_bit];
char str[max];
char output_str[max];
int flag[] = {0, 1, 1, 1, 1, 1, 1, 0};
int i, b, flaglen = 8;

void print(const char *label, int arr[], int n) {
    printf("%s ", label);
    for(i = 0; i < n; i++) {
        printf("%d", arr[i]);
    }
    printf("\n");
}

int main() {
    int choice;

    while(1) {
        printf("___ Menu ___\n");
        printf("1. Standard String Transmission \n");
        printf("2. Manual Binary Input (Test Invalid Stuffing Error)\n");
        printf("3. Exit\n");
        printf("Enter your choice : ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input type. Exiting program.\n");
            break;
        }
        getchar(); // Clear the newline character from the buffer

        if (choice == 3) {
            printf("\nExiting>>>>\n");
            break;
        }

        int ones = 0, j = 0, bit = 0;

        switch(choice) {
            case 1: {
                printf("\n--- Case 1: Standard String Transmission ---\n");
                printf("enter a string: ");
                fgets(str, sizeof(str), stdin);
                str[strcspn(str, "\n")] = '\0';

                int str_len = strlen(str);

                // Step 1: String to Binary
                for(i = 0; i < str_len; i++) {
                    unsigned char ch = (unsigned char)str[i];
                    for(b = 7; b >= 0; b--) {
                        data[bit++] = (ch >> b) & 1;
                    }
                }
                print("original data:", data, bit);

                // Step 2: Bit Stuffing
                for(i = 0; i < bit; i++) {
                    stuff[j++] = data[i];
                    ones = (data[i] == 1) ? ones + 1 : 0;
                    if(ones == 5) {
                        stuff[j++] = 0;
                        ones = 0;
                    }
                }
                int stufflen = j;

                // Step 3: Framing
                int framed[max_stuff], k = 0;
                for(i = 0; i < flaglen; i++) framed[k++] = flag[i];
                for(i = 0; i < stufflen; i++) framed[k++] = stuff[i];
                for(i = 0; i < flaglen; i++) framed[k++] = flag[i];
                int framelen = k;

                print("stuffed:       ", stuff, stufflen);
                print("framed:        ", framed, framelen);

                // Step 4: Bit Destuffing
                ones = 0;
                j = 0;
                int error_detected = 0;
                for(i = flaglen; i < framelen - flaglen; i++) {
                    destuff[j++] = framed[i];
                    ones = (framed[i] == 1) ? ones + 1 : 0;
                    if(ones == 5) {
                        if(framed[i+1] != 0) {
                            printf("\n[ERROR] Invalid stuffing detected during automatic processing!\n");
                            error_detected = 1;
                            break;
                        }
                        i++;
                        ones = 0;
                    }
                }

                if (error_detected) break; // Return to menu loop instead of killing program

                int destufflen = j;
                print("destuffed:     ", destuff, destufflen);

                // Step 5: Binary Back to String
                int out_idx = 0;
                for(i = 0; i < destufflen; i += 8) {
                    unsigned char ch = 0;
                    for(b = 0; b < 8; b++) {
                        ch = (ch << 1) | destuff[i+b];
                    }
                    output_str[out_idx++] = ch;
                }
                output_str[out_idx] = '\0';
                printf("output string: %s\n", output_str);
                break;
            }

            case 2: {
                char binary_str[max_stuff];
                int input_framed[max_stuff];

                printf("\n--- Case 2: Manual Binary Input Mode ---\n");
                printf("Enter raw binary stream to destuff): ");
                fgets(binary_str, sizeof(binary_str), stdin);
                binary_str[strcspn(binary_str, "\n")] = '\0';

                int input_len = strlen(binary_str);
                int valid_input = 1;
                for(i = 0; i < input_len; i++) {
                    if(binary_str[i] == '1') input_framed[i] = 1;
                    else if(binary_str[i] == '0') input_framed[i] = 0;
                    else {
                        printf("Error: Input must only contain 0s and 1s.\n");
                        valid_input = 0;
                        break;
                    }
                }

                if (!valid_input) break;

                printf("\nProcessing your binary stream...\n");
                print("Input stream:  ", input_framed, input_len);

                ones = 0;
                j = 0;
                int discard_frame = 0;

                for(i = 0; i < input_len; i++) {
                    destuff[j++] = input_framed[i];
                    ones = (input_framed[i] == 1) ? ones + 1 : 0;

                    if(ones == 5) {
                        if(i + 1 >= input_len) {
                            printf("\n[DISCARDED] Frame error: Stream ended
                            abruptly right after 5 consecutive 1s.\n");
                            printf("Description: HDLC protocol requires a
                            trailing bit (either a stuffed 0 or a flag sequence) after five 1s. None was found.\n");
                            discard_frame = 1;
                            break;
                        }

                        if(input_framed[i+1] != 0) {
                            printf("\n[DISCARDED] Error: Invalid stuffing pattern detected!\n");
                            printf("Description:\n");
                            printf(" -> Found 5 consecutive '1' bits ending at index %d.\n", i);
                            printf(" -> According to bit-stuffing rules, the next bit (index %d) MUST be a stuffed '0'.\n", i + 1);
                            printf(" -> Instead, found a '%d' bit.\n", input_framed[i+1]);
                            printf(" ->DISCARDED\n");
                            discard_frame = 1;
                            break;
                        }

                        i++;
                        ones = 0;
                    }
                }

                if (discard_frame) break; // Return back to menu loop gracefully

                int destufflen = j;
                print("destuffed:     ", destuff, destufflen);
                break;
            }

            default:
                printf("Invalid selection. Please choose option 1, 2, or 3.\n");
                break;
        }
    }

    return 0;
}

[24bcs056@mepcolinux ex1]$./bit
___ Menu ___
1. Standard String Transmission
2. Manual Binary Input (Test Invalid Stuffing Error)
3. Exit
Enter your choice : 1

--- Case 1: Standard String Transmission ---
enter a string: ~~
original data: 0111111001111110
stuffed:        011111010011111010
framed:         0111111001111101001111101001111110
destuffed:      0111111001111110
output string: ~~
___ Menu ___
1. Standard String Transmission
2. Manual Binary Input (Test Invalid Stuffing Error)
3. Exit
Enter your choice : 2

--- Case 2: Manual Binary Input Mode ---
Enter raw binary stream to destuff): 01111110

Processing your binary stream...
Input stream:   01111110

[DISCARDED] Error: Invalid stuffing pattern detected!
Description:
 -> Found 5 consecutive '1' bits ending at index 5.
 -> According to bit-stuffing rules, the next bit (index 6) MUST be a stuffed '0'.
 -> Instead, found a '1' bit.
 ->DISCARDED
___ Menu ___
1. Standard String Transmission
2. Manual Binary Input (Test Invalid Stuffing Error)
3. Exit
Enter your choice : 2

--- Case 2: Manual Binary Input Mode ---
Enter raw binary stream to destuff): 01111110

Processing your binary stream...
Input stream:   01111110

[DISCARDED] Error: Invalid stuffing pattern detected!
Description:
 -> Found 5 consecutive '1' bits ending at index 5.
 -> According to bit-stuffing rules, the next bit (index 6) MUST be a stuffed '0'.
 -> Instead, found a '1' bit.
 ->DISCARDED
___ Menu ___
1. Standard String Transmission
2. Manual Binary Input (Test Invalid Stuffing Error)
3. Exit
Enter your choice :2

--- Case 2: Manual Binary Input Mode ---
Enter raw binary stream to destuff): 01111101

Processing your binary stream...
Input stream:   01111101
destuffed:      0111111
___ Menu ___
1. Standard String Transmission
2. Manual Binary Input (Test Invalid Stuffing Error)
3. Exit
Enter your choice : 3

Exiting>>>>
[24bcs056@mepcolinux ex1]$cat byte.c
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
[24bcs056@mepcolinux ex1]$./byte
Enter 8 bits Flag: 0 1 0 0 0 0 0 1
Enter 8 bits ESC: 0 1 0 0 0 0 1 0
--- Menu ---
1. Standard String Transmission
2. Manual Byte Stream Input (Test Corrupt Framing Errors)
3. Exit
Enter your choice: 1

--- Case 1: Standard String Transmission ---
Enter Message: AB

FLAG: 01000001
ESC:  01000010

Original Data: 01000001 01000010
Stuffed Data : 01000010 01000001 01000010 01000010
Framed Data  : 01000001 01000010 01000001 01000010 01000010 01000001
Destuffed Data: 01000001 01000010

Destuffing successful!
Received Message : AB
--- Menu ---
1. Standard String Transmission
2. Manual Byte Stream Input (Test Corrupt Framing Errors)
3. Exit
Enter your choice: 2

--- Case 2: Manual Byte Stream Input Mode ---
How many bytes? 2
Enter the 2 integer values
4
5

Processing manual byte stream...
Input Stream Data: 00000100 00000101

[DISCARDED] Framing Failure: Missing correct Start or End boundary marker!
Detailed Description:
 -> Expected Flag Byte decimal value: 65
 -> First stream element: 4, Last stream element: 5
 -> Framing Rules dictate frames must start and conclude exactly with the specified control Flag.
--- Menu ---
1. Standard String Transmission
2. Manual Byte Stream Input (Test Corrupt Framing Errors)
3. Exit
Enter your choice: 2

--- Case 2: Manual Byte Stream Input Mode ---
How many bytes? 2
Enter the 2 integer values
65
66

Processing manual byte stream...
Input Stream Data: 01000001 01000010

[DISCARDED] Framing Failure: Missing correct Start or End boundary marker!
Detailed Description:
 -> Expected Flag Byte decimal value: 65
 -> First stream element: 65, Last stream element: 66
 -> Framing Rules dictate frames must start and conclude exactly with the specified control Flag.
--- Menu ---
1. Standard String Transmission
2. Manual Byte Stream Input (Test Corrupt Framing Errors)
3. Exit
Enter your choice: 2

--- Case 2: Manual Byte Stream Input Mode ---
How many bytes? 3
Enter the 3 integer values
65
3
65

Processing manual byte stream...
Input Stream Data: 01000001 00000011 01000001
Successfully Destuffed Data: 00000011
Decoded payload contains non-printable binary control bytes.
--- Menu ---
1. Standard String Transmission
2. Manual Byte Stream Input (Test Corrupt Framing Errors)
3. Exit
Enter your choice: 3

Exiting program. Goodbye!
