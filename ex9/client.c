#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>

#define PORT 7070
#define BUF 4096

void get_password(char *pass, int max) {
    struct termios old, new_t;
    int i = 0;
    char ch;

    tcgetattr(STDIN_FILENO, &old);
    new_t = old;
    new_t.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t);

    while (i < max - 1) {
        ch = getchar();
        if (ch == '\n' || ch == '\r')
            break;
        else if (ch == 127 || ch == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        } else {
            pass[i++] = ch;
            printf("*");
            fflush(stdout);
        }
    }
    pass[i] = '\0';
    printf("\n");

    tcsetattr(STDIN_FILENO, TCSANOW, &old);
}

void do_login(int sfd, char *user) {
    char pass[50], buf[BUF];

    printf("Username: ");
    scanf("%s", user);
    getchar();

    printf("Password: ");
    get_password(pass, 50);

    send(sfd, user, strlen(user) + 1, 0);
    usleep(100000);
    send(sfd, pass, strlen(pass) + 1, 0);

    memset(buf, 0, BUF);
    recv(sfd, buf, BUF - 1, 0);

    if (strcmp(buf, "OK") != 0) {
        printf("\n530 Login failed!\n");
        close(sfd);
        exit(0);
    }

    printf("\n230 Login successful! Welcome, %s.\n", user);
}

int main() {
    int sfd, bytes;
    struct sockaddr_in addr;
    char buf[BUF], user[50], cmd[BUF];

    printf("=============================\n");
    printf("        FTP Client\n");
    printf("=============================\n\n");

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to FTP Server.\n\n");

    do_login(sfd, user);
    printf("Type HELP for commands. Use !<cmd> for local shell.\n");

    while (1) {
        printf("\nftp> ");
        fflush(stdout);

        if (fgets(cmd, BUF, stdin) == NULL)
            break;
        cmd[strcspn(cmd, "\n")] = '\0';

        if (strlen(cmd) == 0)
            continue;

        if (cmd[0] == '!') {
            if (strlen(cmd) > 1)
                system(cmd + 1);
            else
                printf("Usage: !<command>  (e.g., !ls, !echo hello > test.txt)\n");
            continue;
        }

        if (strcmp(cmd, "QUIT") == 0) {
            send(sfd, cmd, strlen(cmd) + 1, 0);
            memset(buf, 0, BUF);
            recv(sfd, buf, BUF - 1, 0);
            printf("%s\n", buf);
            break;
        }

        else if (strncmp(cmd, "STOR ", 5) == 0) {
            char *filename = cmd + 5;

            FILE *file = fopen(filename, "rb");
            if (!file) {
                printf("Local file '%s' not found.\n", filename);
                printf("Tip: Use !echo \"content\" > %s to create it.\n", filename);
                continue;
            }

            send(sfd, cmd, strlen(cmd) + 1, 0);

            memset(buf, 0, BUF);
            recv(sfd, buf, BUF - 1, 0);

            if (strcmp(buf, "ERROR") == 0) {
                printf("550 Server cannot create file.\n");
                fclose(file);
                continue;
            }

            printf("%s\n", buf);
            printf("Storing %s on server...\n", filename);

            char fbuf[BUF];
            size_t r;
            while ((r = fread(fbuf, 1, BUF, file)) > 0) {
                send(sfd, fbuf, r, 0);
            }
            fclose(file);
            usleep(100000);
            send(sfd, "EOF_MARKER", 10, 0);

            memset(buf, 0, BUF);
            recv(sfd, buf, BUF - 1, 0);
            printf("%s\n", buf);
        }

        else if (strncmp(cmd, "RETR ", 5) == 0) {
            char *filename = cmd + 5;
            send(sfd, cmd, strlen(cmd) + 1, 0);

            memset(buf, 0, BUF);
            recv(sfd, buf, BUF - 1, 0);

            if (strcmp(buf, "ERROR") == 0) {
                printf("550 File not found on server.\n");
                continue;
            }

            printf("%s\n", buf);

            FILE *file = fopen(filename, "wb");
            if (!file) {
                printf("Cannot create local file.\n");
                continue;
            }

            printf("Retrieving %s from server...\n", filename);
            while ((bytes = recv(sfd, buf, BUF, 0)) > 0) {
                if (bytes >= 10 && strncmp(buf + bytes - 10, "EOF_MARKER", 10) == 0) {
                    fwrite(buf, 1, bytes - 10, file);
                    break;
                }
                fwrite(buf, 1, bytes, file);
            }
            fclose(file);
            printf("226 Retrieve complete: %s\n", filename);
        }

        else if (strncmp(cmd, "APPE ", 5) == 0) {
            char *filename = cmd + 5;

            FILE *file = fopen(filename, "rb");
            if (!file) {
                printf("Local file '%s' not found.\n", filename);
                continue;
            }

            send(sfd, cmd, strlen(cmd) + 1, 0);

            memset(buf, 0, BUF);
            recv(sfd, buf, BUF - 1, 0);

            if (strcmp(buf, "ERROR") == 0) {
                printf("550 Server cannot open file.\n");
                fclose(file);
                continue;
            }

            printf("%s\n", buf);
            printf("Appending %s to server...\n", filename);

            char fbuf[BUF];
            size_t r;
            while ((r = fread(fbuf, 1, BUF, file)) > 0) {
                send(sfd, fbuf, r, 0);
            }
            fclose(file);
            usleep(100000);
            send(sfd, "EOF_MARKER", 10, 0);

            memset(buf, 0, BUF);
            recv(sfd, buf, BUF - 1, 0);
            printf("%s\n", buf);
        }

        else if (strcmp(cmd, "REIN") == 0) {
            send(sfd, cmd, strlen(cmd) + 1, 0);

            memset(buf, 0, BUF);
            recv(sfd, buf, BUF - 1, 0);
            printf("%s\n", buf);

            do_login(sfd, user);
        }

        else {
            send(sfd, cmd, strlen(cmd) + 1, 0);

            memset(buf, 0, BUF);
            recv(sfd, buf, BUF - 1, 0);
            printf("%s\n", buf);
        }
    }

    close(sfd);
    return 0;
}
