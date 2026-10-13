#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082tudent@cts-OptiPlex-3070:~$ cat q22.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8095
#define SIZE 1024

int main(void)
{
    int s, n, running = 1;
    char b[SIZE];
    struct sockaddr_in addr = {0};
    socklen_t len = sizeof(addr);
    fd_set readfds;
    struct timeval timeout;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* Set 2-second receive timeout for handshake retries */
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    /* ================================================================ */
    /* PHASE 1: Connect to server - retry HELLO until connected         */
    /* If server sends WAIT, block until server sends CONNECTED later   */
    /* ================================================================ */
    int connected = 0;
    while (!connected) {
        if (sendto(s, "HELLO", 5, 0,
                   (struct sockaddr *)&addr, len) < 0) {
            perror("sendto");
            close(s);
            return 1;
        }

        n = recvfrom(s, b, SIZE - 1, 0, NULL, NULL);
        if (n > 0) {
            b[n] = '\0';

            if (strcmp(b, "CONNECTED") == 0) {
                printf("Connected to server. Type EXIT to quit.\n\n");
                connected = 1;
            } else if (strcmp(b, "WAIT") == 0) {
                printf("Server is busy. You are in queue. Waiting for your turn...\n");
                /* Server will send CONNECTED when active client exits */
                /* Wait with periodic retry in case CONNECTED is lost */
                while (!connected) {
                    n = recvfrom(s, b, SIZE - 1, 0, NULL, NULL);
                    if (n > 0) {
                        b[n] = '\0';
                        if (strcmp(b, "CONNECTED") == 0) {
                            printf("Connected to server. Type EXIT to quit.\n\n");
                            connected = 1;
                        }
                    } else {
                        /* Timeout - remind server we are still waiting */
                        printf("Still waiting... retrying HELLO.\n");
                        sendto(s, "HELLO", 5, 0,
                               (struct sockaddr *)&addr, len);
                    }
                }
            } else if (strcmp(b, "BUSY") == 0) {
                printf("Server busy. Retrying in 1 second...\n");
                sleep(1);
            } else {
                printf("Unexpected response: %s. Retrying...\n", b);
                sleep(1);
            }
        } else {
            /* Timeout - no response from server, retry HELLO */
            printf("No response from server. Retrying HELLO...\n");
        }
    }

    /* Remove timeout for normal full-duplex chat */
    struct timeval no_timeout = {0, 0};
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &no_timeout, sizeof(no_timeout));

    printf("Client> ");
    fflush(stdout);

    /* ================================================================ */
    /* PHASE 2: Full-duplex chat                                        */
    /* ================================================================ */
    while (running) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(s, &readfds);

        if (select(s + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        /* Message received from server */
        if (FD_ISSET(s, &readfds)) {
            n = recvfrom(s, b, SIZE - 1, 0, NULL, NULL);
            if (n <= 0)
                continue;

            b[n] = '\0';
            printf("\r\033[K");

            if (strcmp(b, "EXIT") == 0) {
                printf("Server left the chat.\n");
                running = 0;
            } else {
                printf("Server: %s\nClient> ", b);
                fflush(stdout);
            }
        }

        /* Input typed by user */
        if (running && FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!fgets(b, sizeof(b), stdin))
                strcpy(b, "EXIT");
            else
                b[strcspn(b, "\n")] = '\0';

            if (b[0] == '\0') {
                printf("Client> ");
                fflush(stdout);
                continue;
            }

            if (sendto(s, b, strlen(b), 0,
                       (struct sockaddr *)&addr, len) < 0) {
                perror("sendto");
                break;
            }

            if (strcmp(b, "EXIT") == 0) {
                printf("You left the chat.\n");
                running = 0;
            } else {
                printf("Client> ");
                fflush(stdout);
            }
        }
    }

    close(s);
    return 0;
}

#define SIZE 1024

int main() {
    int sockfd, n;
    struct sockaddr_in server;
    char domain[SIZE], ip[SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    do {
        printf("Domain: ");
        fgets(domain, SIZE, stdin);
        domain[strcspn(domain, "\n")] = '\0';

        sendto(sockfd, domain, strlen(domain), 0,
               (struct sockaddr *)&server, sizeof(server));

        if (strcmp(domain, "-1") == 0)
            break;

        n = recvfrom(sockfd, ip, SIZE - 1, 0, NULL, NULL);
        if (n < 0) {
            perror("recvfrom");
            close(sockfd);
            return 1;
        }

        ip[n] = '\0';
        printf("IP: %s\n", ip);
    } while (1);

    close(sockfd);
    return 0;
}
