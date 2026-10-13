#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024
#define MAX 10

int same(struct sockaddr_in *a, struct sockaddr_in *b) {
    return a->sin_addr.s_addr == b->sin_addr.s_addr && a->sin_port == b->sin_port;
}

int main() {
    int sockfd, n, has = 0, cur = 0, next = 1, wn = 0;
    struct sockaddr_in server, from, current, waddr[MAX];
    int wids[MAX];
    char domain[SIZE], ip[SIZE];

    const char *names[] = {"google.com", "yahoo.com", "example.com", "github.com", "facebook.com", "twitter.com"};
    const char *ips[] = {"142.250.0.1", "98.137.11.163", "93.184.216.34", "140.82.121.4", "157.240.0.35", "104.244.42.1"};

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    bind(sockfd, (struct sockaddr *)&server, sizeof(server));

    printf("Ready\n");

    while (1) {
        socklen_t len = sizeof(from);
        n = recvfrom(sockfd, domain, SIZE - 1, 0, (struct sockaddr *)&from, &len);
        if (n < 0) continue;
        domain[n] = '\0';

        if (!has) {
            int f = -1;
            for (int i = 0; i < wn; i++) if (same(&waddr[i], &from)) { f = i; break; }
            if (f >= 0) {
                current = waddr[f]; cur = wids[f];
                waddr[f] = waddr[wn - 1]; wids[f] = wids[wn - 1]; wn--;
            } else {
                current = from; cur = next++;
            }
            has = 1;
            printf("C%d connected\n", cur);
        } else if (!same(&from, &current)) {
            int f = -1;
            for (int i = 0; i < wn; i++) if (same(&waddr[i], &from)) { f = i; break; }
            if (f < 0 && wn < MAX) {
                waddr[wn] = from; wids[wn] = next++;
                printf("C%d wait\n", wids[wn]);
                wn++;
            }
            sendto(sockfd, "Wait", 4, 0, (struct sockaddr *)&from, sizeof(from));
            continue;
        }

        if (strcmp(domain, "-1") == 0) {
            printf("C%d left\n", cur);
            has = 0; cur = 0;
            continue;
        }

        strcpy(ip, "Not found");
        for (int i = 0; i < 6; i++) {
            if (strcmp(domain, names[i]) == 0) {
                strcpy(ip, ips[i]);
                break;
            }
        }

        sendto(sockfd, ip, strlen(ip), 0, (struct sockaddr *)&from, sizeof(from));
        printf("C%d: %s\n", cur, domain);
    }

    close(sockfd);
    return 0;
}
