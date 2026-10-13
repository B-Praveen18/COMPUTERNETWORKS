#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 7070
#define BUF 4096

int auth(const char *u, const char *p) {
    FILE *f = fopen("users.txt", "r");
    char fu[50], fp2[50];
    if (!f) return 0;
    while (fscanf(f, "%s %s", fu, fp2) == 2) {
        if (strcmp(u, fu) == 0 && strcmp(p, fp2) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void handle_client(int cfd) {
    char buf[BUF], user[50], pass[50], resp[BUF];
    int bytes;
    time_t login_time;

    memset(user, 0, 50);
    recv(cfd, user, 49, 0);
    memset(pass, 0, 50);
    recv(cfd, pass, 49, 0);

    if (auth(user, pass)) {
        send(cfd, "OK", 2, 0);
        login_time = time(NULL);
        printf("[+] User '%s' logged in.\n", user);
        fflush(stdout);
    } else {
        send(cfd, "NO", 2, 0);
        printf("[-] Auth failed for '%s'.\n", user);
        fflush(stdout);
        close(cfd);
        exit(0);
    }

    while (1) {
        memset(buf, 0, BUF);
        bytes = recv(cfd, buf, BUF - 1, 0);
        if (bytes <= 0) break;

        buf[strcspn(buf, "\n")] = '\0';
        printf("[%s] Command: %s\n", user, buf);
        fflush(stdout);

        memset(resp, 0, BUF);

        if (strcmp(buf, "QUIT") == 0) {
            send(cfd, "221 Goodbye.", 12, 0);
            printf("[%s] Disconnected.\n", user);
            fflush(stdout);
            break;
        }

        else if (strcmp(buf, "HELP") == 0) {
            strcpy(resp,
                "========== FTP Commands ==========\n"
                "STOR   <file>       - Upload/store file\n"
                "RETR   <file>       - Retrieve/download file\n"
                "APPE   <file>       - Append to file\n"
                "LIST                - List files (detailed)\n"
                "NLST                - List filenames only\n"
                "PWD                 - Print working directory\n"
                "CWD    <dir>        - Change directory\n"
                "CDUP                - Go to parent directory\n"
                "MKD    <dir>        - Make directory\n"
                "RMD    <dir>        - Remove directory\n"
                "DELE   <file>       - Delete file\n"
                "RNFR   <old> <new>  - Rename file\n"
                "SIZE   <file>       - Get file size\n"
                "MDTM   <file>       - File modification time\n"
                "STAT                - Server status\n"
                "SYST                - System info\n"
                "FEAT                - Server features\n"
                "TYPE   <file>       - View file content\n"
                "NOOP                - Ping server\n"
                "REIN                - Reinitialize session\n"
                "!<cmd>              - Local shell command\n"
                "HELP                - Show this help\n"
                "QUIT                - Disconnect\n"
                "==================================\n");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strcmp(buf, "LIST") == 0) {
            DIR *dir = opendir(".");
            struct dirent *entry;
            struct stat st;
            strcpy(resp, "150 Opening data connection.\n");
            if (dir) {
                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_name[0] != '.') {
                        stat(entry->d_name, &st);
                        if (S_ISDIR(st.st_mode))
                            snprintf(resp + strlen(resp), BUF - strlen(resp), "drwxr-xr-x  %8ld  %s\n", (long)st.st_size, entry->d_name);
                        else
                            snprintf(resp + strlen(resp), BUF - strlen(resp), "-rw-r--r--  %8ld  %s\n", (long)st.st_size, entry->d_name);
                    }
                }
                closedir(dir);
            }
            strcat(resp, "226 Transfer complete.");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strcmp(buf, "NLST") == 0) {
            DIR *dir = opendir(".");
            struct dirent *entry;
            strcpy(resp, "");
            if (dir) {
                while ((entry = readdir(dir)) != NULL) {
                    if (entry->d_name[0] != '.') {
                        strcat(resp, entry->d_name);
                        strcat(resp, "\n");
                    }
                }
                closedir(dir);
            }
            if (strlen(resp) == 0)
                strcpy(resp, "Directory is empty.");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strcmp(buf, "PWD") == 0) {
            char cwd[512];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                snprintf(resp, BUF, "257 \"%s\" is current directory.", cwd);
            else
                strcpy(resp, "550 Cannot get directory.");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strncmp(buf, "CWD ", 4) == 0) {
            char *dir = buf + 4;
            if (chdir(dir) == 0)
                strcpy(resp, "250 Directory changed.");
            else
                strcpy(resp, "550 Failed to change directory.");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strcmp(buf, "CDUP") == 0) {
            if (chdir("..") == 0) {
                char cwd[512];
                getcwd(cwd, sizeof(cwd));
                snprintf(resp, BUF, "200 Directory changed to \"%s\".", cwd);
            } else {
                strcpy(resp, "550 Failed to change to parent directory.");
            }
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strncmp(buf, "STOR ", 5) == 0) {
            char *filename = buf + 5;
            FILE *file = fopen(filename, "wb");
            if (!file) {
                send(cfd, "ERROR", 5, 0);
                continue;
            }
            send(cfd, "150 Ready to receive.", 21, 0);

            char fbuf[BUF];
            while ((bytes = recv(cfd, fbuf, BUF, 0)) > 0) {
                if (bytes >= 10 && strncmp(fbuf + bytes - 10, "EOF_MARKER", 10) == 0) {
                    fwrite(fbuf, 1, bytes - 10, file);
                    break;
                }
                fwrite(fbuf, 1, bytes, file);
            }
            fclose(file);
            send(cfd, "226 STOR complete. File saved.", 29, 0);
            printf("[%s] Stored: %s\n", user, filename);
            fflush(stdout);
        }

        else if (strncmp(buf, "RETR ", 5) == 0) {
            char *filename = buf + 5;
            FILE *file = fopen(filename, "rb");
            if (!file) {
                send(cfd, "ERROR", 5, 0);
                continue;
            }
            send(cfd, "150 Opening data connection.", 28, 0);
            usleep(100000);

            char fbuf[BUF];
            size_t r;
            while ((r = fread(fbuf, 1, BUF, file)) > 0) {
                send(cfd, fbuf, r, 0);
            }
            fclose(file);
            usleep(100000);
            send(cfd, "EOF_MARKER", 10, 0);
            printf("[%s] Retrieved: %s\n", user, filename);
            fflush(stdout);
        }

        else if (strncmp(buf, "APPE ", 5) == 0) {
            char *filename = buf + 5;
            FILE *file = fopen(filename, "ab");
            if (!file) {
                send(cfd, "ERROR", 5, 0);
                continue;
            }
            send(cfd, "150 Ready to append.", 20, 0);

            char fbuf[BUF];
            while ((bytes = recv(cfd, fbuf, BUF, 0)) > 0) {
                if (bytes >= 10 && strncmp(fbuf + bytes - 10, "EOF_MARKER", 10) == 0) {
                    fwrite(fbuf, 1, bytes - 10, file);
                    break;
                }
                fwrite(fbuf, 1, bytes, file);
            }
            fclose(file);
            send(cfd, "226 APPE complete. Data appended.", 32, 0);
            printf("[%s] Appended: %s\n", user, filename);
            fflush(stdout);
        }

        else if (strncmp(buf, "DELE ", 5) == 0) {
            char *filename = buf + 5;
            if (remove(filename) == 0)
                strcpy(resp, "250 File deleted.");
            else
                strcpy(resp, "550 File not found.");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strncmp(buf, "RNFR ", 5) == 0) {
            char old_name[256], new_name[256];
            if (sscanf(buf + 5, "%s %s", old_name, new_name) == 2) {
                if (rename(old_name, new_name) == 0)
                    strcpy(resp, "250 File renamed.");
                else
                    strcpy(resp, "550 Rename failed.");
            } else {
                strcpy(resp, "Usage: RNFR <old> <new>");
            }
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strncmp(buf, "MKD ", 4) == 0) {
            char *dirname = buf + 4;
            if (mkdir(dirname, 0755) == 0)
                strcpy(resp, "257 Directory created.");
            else
                strcpy(resp, "550 Failed to create directory.");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strncmp(buf, "RMD ", 4) == 0) {
            char *dirname = buf + 4;
            if (rmdir(dirname) == 0)
                strcpy(resp, "250 Directory removed.");
            else
                strcpy(resp, "550 Failed to remove directory.");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strncmp(buf, "SIZE ", 5) == 0) {
            char *filename = buf + 5;
            struct stat st;
            if (stat(filename, &st) == 0)
                snprintf(resp, BUF, "213 %ld bytes", (long)st.st_size);
            else
                strcpy(resp, "550 File not found.");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strncmp(buf, "MDTM ", 5) == 0) {
            char *filename = buf + 5;
            struct stat st;
            if (stat(filename, &st) == 0) {
                struct tm *tm_info = localtime(&st.st_mtime);
                char timebuf[64];
                strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%S", tm_info);
                snprintf(resp, BUF, "213 %s", timebuf);
            } else {
                strcpy(resp, "550 File not found.");
            }
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strncmp(buf, "TYPE ", 5) == 0) {
            char *filename = buf + 5;
            FILE *file = fopen(filename, "r");
            if (!file) {
                strcpy(resp, "550 File not found.");
                send(cfd, resp, strlen(resp), 0);
                continue;
            }
            strcpy(resp, "");
            char line[512];
            while (fgets(line, sizeof(line), file) != NULL) {
                if (strlen(resp) + strlen(line) < BUF - 1)
                    strcat(resp, line);
                else
                    break;
            }
            fclose(file);
            if (strlen(resp) == 0)
                strcpy(resp, "File is empty.");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strcmp(buf, "STAT") == 0) {
            char cwd[512];
            time_t now;
            int duration;
            getcwd(cwd, sizeof(cwd));
            now = time(NULL);
            duration = (int)(now - login_time);
            snprintf(resp, BUF,
                "211 Server Status:\n"
                "  User: %s\n"
                "  Directory: %s\n"
                "  Port: %d\n"
                "  Protocol: TCP\n"
                "  Mode: Concurrent (fork)\n"
                "  Session Duration: %d seconds\n"
                "  Status: Connected", user, cwd, PORT, duration);
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strcmp(buf, "SYST") == 0) {
            strcpy(resp, "215 UNIX Type: L8");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strcmp(buf, "FEAT") == 0) {
            strcpy(resp,
                "211 Features:\n"
                "  STOR\n"
                "  RETR\n"
                "  APPE\n"
                "  LIST\n"
                "  NLST\n"
                "  PWD\n"
                "  CWD\n"
                "  CDUP\n"
                "  MKD\n"
                "  RMD\n"
                "  DELE\n"
                "  RNFR\n"
                "  SIZE\n"
                "  MDTM\n"
                "  TYPE\n"
                "  STAT\n"
                "  SYST\n"
                "  NOOP\n"
                "  REIN\n"
                "  HELP\n"
                "211 End");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strcmp(buf, "NOOP") == 0) {
            strcpy(resp, "200 OK.");
            send(cfd, resp, strlen(resp), 0);
        }

        else if (strcmp(buf, "REIN") == 0) {
            printf("[%s] Session reinitialized.\n", user);
            fflush(stdout);
            send(cfd, "220 Service ready. Please login again.", 37, 0);

            memset(user, 0, 50);
            recv(cfd, user, 49, 0);
            memset(pass, 0, 50);
            recv(cfd, pass, 49, 0);

            if (auth(user, pass)) {
                send(cfd, "OK", 2, 0);
                login_time = time(NULL);
                printf("[+] User '%s' re-logged in.\n", user);
                fflush(stdout);
            } else {
                send(cfd, "NO", 2, 0);
                printf("[-] Re-auth failed for '%s'.\n", user);
                fflush(stdout);
                break;
            }
        }

        else {
            strcpy(resp, "500 Unknown command. Type HELP for commands.");
            send(cfd, resp, strlen(resp), 0);
        }
    }

    close(cfd);
    exit(0);
}

int main() {
    int sfd;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int opt = 1;

    printf("=============================\n");
    printf("   Concurrent FTP Server\n");
    printf("=============================\n\n");

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(sfd, 10);
    signal(SIGCHLD, SIG_IGN);
    printf("FTP Server listening on port %d...\n\n", PORT);
    fflush(stdout);

    while (1) {
        int cfd = accept(sfd, (struct sockaddr *)&addr, &len);
        if (cfd < 0) continue;

        printf("[+] Client connected.\n");
        fflush(stdout);

        if (fork() == 0) {
            close(sfd);
            handle_client(cfd);
        } else {
            close(cfd);
        }
    }

    close(sfd);
    return 0;
}
