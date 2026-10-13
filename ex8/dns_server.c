tudent@cts-OptiPlex-3070:~$ cat q11.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define SIZE 1024
#define MAX 10
#define CACHE_MAX 100
#define CACHE_FILE "dns_cache.txt"

typedef struct {
    char domain[SIZE];
    char ip[SIZE];
} CacheEntry;

CacheEntry cache[CACHE_MAX];
int cache_count = 0;

/* Check if two addresses are the same */
int same(struct sockaddr_in *a, struct sockaddr_in *b) {
    return a->sin_addr.s_addr == b->sin_addr.s_addr && a->sin_port == b->sin_port;
}

/* Load cache from file into memory */
void load_cache(void) {
    FILE *fp = fopen(CACHE_FILE, "r");
    if (!fp) return;
    cache_count = 0;
    while (cache_count < CACHE_MAX &&
           fscanf(fp, "%s %s", cache[cache_count].domain, cache[cache_count].ip) == 2) {
        cache_count++;
    }
    fclose(fp);
}

/* Save entire cache from memory to file */
void save_cache(void) {
    FILE *fp = fopen(CACHE_FILE, "w");
    if (!fp) return;
    for (int i = 0; i < cache_count; i++) {
        fprintf(fp, "%s %s\n", cache[i].domain, cache[i].ip);
    }
    fclose(fp);
}

/* Search cache in memory */
char *find_in_cache(const char *domain) {
    for (int i = 0; i < cache_count; i++) {
        if (strcmp(cache[i].domain, domain) == 0) {
            return cache[i].ip;
        }
    }
    return NULL;
}

/* Add new entry to cache */
void add_to_cache(const char *domain, const char *ip) {
    if (cache_count >= CACHE_MAX) return;
    /* Avoid duplicates */
    if (find_in_cache(domain)) return;
    strncpy(cache[cache_count].domain, domain, SIZE - 1);
    cache[cache_count].domain[SIZE - 1] = '\0';
    strncpy(cache[cache_count].ip, ip, SIZE - 1);
    cache[cache_count].ip[SIZE - 1] = '\0';
    cache_count++;
}

/* Lookup domain: cache first, then hardcoded list */
void lookup(const char *domain, char *ip) {
    /* 1. Check cache first */
    char *cached_ip = find_in_cache(domain);
    if (cached_ip) {
        strcpy(ip, cached_ip);
        return;
    }

    /* 2. Check hardcoded list */
    const char *names[] = {"google.com", "yahoo.com", "example.com",
                           "github.com", "facebook.com", "twitter.com"};
    const char *ips[]   = {"142.250.0.1", "98.137.11.163", "93.184.216.34",
                           "140.82.121.4", "157.240.0.35", "104.244.42.1"};

    for (int i = 0; i < 6; i++) {
        if (strcmp(domain, names[i]) == 0) {
            strcpy(ip, ips[i]);
            /* Save to cache for future requests */
            add_to_cache(domain, ips[i]);
            save_cache();
            return;
        }
    }

    strcpy(ip, "Not found");
}

int main() {
    int sockfd, n, has = 0, next = 1, cur = 0;
    struct sockaddr_in server, from, current;
    struct {
        struct sockaddr_in addr;
        int id;
        char domain[SIZE];
    } wait[MAX];
    int wn = 0;
    char domain[SIZE], ip[SIZE];

    /* Load existing cache from file */
    load_cache();

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    bind(sockfd, (struct sockaddr *)&server, sizeof(server));

    printf("Ready\n");
    fflush(stdout);

    while (1) {
        socklen_t len = sizeof(from);
        n = recvfrom(sockfd, domain, SIZE - 1, 0,
                     (struct sockaddr *)&from, &len);
        if (n < 0) continue;
        domain[n] = '\0';

        int widx = -1;
        for (int i = 0; i < wn; i++) {
            if (same(&wait[i].addr, &from)) {
                widx = i;
                break;
            }
        }

        if (!has) {
            current = from;
            cur = next++;
            has = 1;
            printf("C%d connected\n", cur);
            fflush(stdout);
        } else if (!same(&from, &current)) {
            if (widx == -1) {
                if (wn >= MAX) continue;
                wait[wn].addr = from;
                wait[wn].id = next++;
                strncpy(wait[wn].domain, domain, SIZE - 1);
                wait[wn].domain[SIZE - 1] = '\0';
                /* REQUIREMENT 2: Server prints NOTHING for waiting clients */
                wn++;
            } else {
                strncpy(wait[widx].domain, domain, SIZE - 1);
                wait[widx].domain[SIZE - 1] = '\0';
            }
            continue;
        }

        if (strcmp(domain, "-1") == 0) {
            printf("C%d left\n", cur);
            fflush(stdout);
            has = 0;
            while (wn > 0 && !has) {
                current = wait[0].addr;
                cur = wait[0].id;
                strncpy(domain, wait[0].domain, SIZE - 1);
                domain[SIZE - 1] = '\0';
                for (int i = 0; i < wn - 1; i++) wait[i] = wait[i + 1];
                wn--;
                if (strcmp(domain, "-1") == 0) {
                    printf("C%d left\n", cur);
                    fflush(stdout);
                } else {
                    has = 1;
                    printf("C%d connected\n", cur);
                    fflush(stdout);
                    lookup(domain, ip);
                    sendto(sockfd, ip, strlen(ip), 0,
                          (struct sockaddr *)&current, sizeof(current));
                    printf("C%d: %s\n", cur, domain);
                    fflush(stdout);
                }
            }
            if (!has) cur = 0;
            continue;
        }

        lookup(domain, ip);
        sendto(sockfd, ip, strlen(ip), 0,
               (struct sockaddr *)&from, sizeof(from));
        printf("C%d: %s\n", cur, domain);
        fflush(stdout);
    }

    close(sockfd);
    return 0;
}
