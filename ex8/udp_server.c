student@cts-OptiPlex-3070:~$ cat q21.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8095
#define SIZE 1024

/* Compare two sockaddr_in addresses */
int addr_equal(struct sockaddr_in *a, struct sockaddr_in *b) {
    return (a->sin_addr.s_addr == b->sin_addr.s_addr &&
            a->sin_port == b->sin_port);
}

int main(void)
{
    int s, n;
    char b[SIZE];
    struct sockaddr_in addr = {0}, client = {0};
    socklen_t len = sizeof(client);
    fd_set readfds;

    /* Active client tracking */
    int has_active_client = 0;
    struct sockaddr_in active_client;

    /* Waiting client tracking */
    int has_waiting_client = 0;
    struct sockaddr_in waiting_client;
    socklen_t waiting_len;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s);
        return 1;
    }

    printf("=== Sequential Full-Duplex UDP Server ===\n");
    printf("Only ONE client at a time. Waiting on port %d...\n\n", PORT);

    while (1) {
        /* ============================================================ */
        /* AUTO-PROMOTE: If a client was waiting, grant access immediately  */
        /* ============================================================ */
        if (has_waiting_client) {
            active_client = waiting_client;
            len = waiting_len;
            has_active_client = 1;
            has_waiting_client = 0;

            sendto(s, "CONNECTED", 9, 0,
                   (struct sockaddr *)&active_client, len);
            printf("[AUTO-CONNECT] Promoted waiting client %s:%d\n",
                   inet_ntoa(active_client.sin_addr), ntohs(active_client.sin_port));
            printf("Type EXIT to release client and wait for next.\n\n");
            printf("Server> ");
            fflush(stdout);
        } else {
            /* ========================================================= */
            /* PHASE 1: Block and wait for HELLO from any client           */
            /* ========================================================= */
            memset(&client, 0, sizeof(client));
            len = sizeof(client);

            printf("[WAITING] No active client. Waiting for HELLO...\n");
            fflush(stdout);

            n = recvfrom(s, b, SIZE - 1, 0,
                         (struct sockaddr *)&client, &len);

            if (n <= 0) {
                perror("recvfrom");
                continue;
            }

            b[n] = '\0';

            if (strcmp(b, "HELLO") != 0) {
                printf("[REJECT] Expected HELLO, got: %s from %s:%d\n",
                       b, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
                continue;
            }

            /* Grant access to first client */
            active_client = client;
            has_active_client = 1;

            sendto(s, "CONNECTED", 9, 0,
                   (struct sockaddr *)&active_client, len);

            printf("[LOCK] Client %s:%d acquired access.\n",
                   inet_ntoa(active_client.sin_addr), ntohs(active_client.sin_port));
            printf("Type EXIT to release client and wait for next.\n\n");
            printf("Server> ");
            fflush(stdout);
        }

        /* ============================================================ */
        /* PHASE 2: Full-duplex chat with active client ONLY              */
        /* ============================================================ */
        int running = 1;
        while (running) {
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(s, &readfds);

            if (select(s + 1, &readfds, NULL, NULL, NULL) < 0) {
                perror("select");
                running = 0;
                break;
            }

            /* Data received from network */
            if (FD_ISSET(s, &readfds)) {
                struct sockaddr_in from = {0};
                socklen_t from_len = sizeof(from);

                n = recvfrom(s, b, SIZE - 1, 0,
                             (struct sockaddr *)&from, &from_len);

                if (n <= 0)
                    continue;

                b[n] = '\0';

                /* Message from non-active client */
                if (!addr_equal(&from, &active_client)) {
                    if (strcmp(b, "HELLO") == 0) {
                        if (!has_waiting_client) {
                            /* Store this client as waiting */
                            waiting_client = from;
                            waiting_len = from_len;
                            has_waiting_client = 1;
                            sendto(s, "WAIT", 4, 0,
                                   (struct sockaddr *)&from, from_len);
                            printf("\r\033[K[WAITING LIST] Client %s:%d queued.\nServer> ",
                                   inet_ntoa(from.sin_addr), ntohs(from.sin_port));
                            fflush(stdout);
                        } else if (addr_equal(&from, &waiting_client)) {
                            /* Same client retrying - re-acknowledge */
                            sendto(s, "WAIT", 4, 0,
                                   (struct sockaddr *)&from, from_len);
                        } else {
                            /* Already have someone waiting */
                            sendto(s, "BUSY", 4, 0,
                                   (struct sockaddr *)&from, from_len);
                        }
                    } else {
                        sendto(s, "BUSY", 4, 0,
                               (struct sockaddr *)&from, from_len);
                    }
                    continue;
                }

                /* Message from active client */
                printf("\r\033[K");

                if (strcmp(b, "EXIT") == 0) {
                    printf("Client left the chat. Access released.\n\n");
                    running = 0;
                } else {
                    printf("Client: %s\nServer> ", b);
                    fflush(stdout);
                }
            }

            /* Data typed by server user on keyboard */
            if (running && FD_ISSET(STDIN_FILENO, &readfds)) {
                if (!fgets(b, sizeof(b), stdin))
                    strcpy(b, "EXIT");
                else
                    b[strcspn(b, "\n")] = '\0';

                if (b[0] == '\0') {
                    printf("Server> ");
                    fflush(stdout);
                    continue;
                }

                if (sendto(s, b, strlen(b), 0,
                           (struct sockaddr *)&active_client, len) < 0) {
                    perror("sendto");
                    running = 0;
                    break;
                }

                if (strcmp(b, "EXIT") == 0) {
                    printf("You ended the chat. Access released.\n\n");
                    running = 0;
                } else {
                    printf("Server> ");
                    fflush(stdout);
                }
            }
        }

        /* ============================================================ */
        /* Chat ended - release active client and check for waiting       */
        /* ============================================================ */
        has_active_client = 0;

        if (has_waiting_client) {
            printf("[UNLOCK] Waiting client found. Auto-connecting now...\n");
            /* Loop back to top - outer while will auto-promote */
        } else {
            printf("[UNLOCK] No waiting client. Back to waiting for HELLO...\n\n");
        }
    }

    close(s);
    return 0;
}
