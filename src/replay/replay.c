#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "noka/replay.h"
#include "noka/record.h"

#define PID_PATH "/var/run"

char* get_replay_path() {
    static char path[512];
    snprintf(path, sizeof(path), "%s/noka/replay", getenv("HOME"));
    return path;
}

int delete_old_replay() {
    DIR* dp = opendir(get_replay_path());
    if (dp) {
        struct dirent* entry;
        while ((entry = readdir(dp)) != NULL)
        {
            printf("%s\n", entry->d_name);
        }
        closedir(dp);
    }

}

int noka_replay_enable() {
    if (access(PID_PATH, F_OK) == 0) {
        printf("noka: replay already enabled\n");
        return 1;
    }
    
    printf("noka: create daemon...\n");
    
    int pid = fork();
    if (pid < 0) {
        perror("noka: fork");
        return 1;
    } else if (pid == 0) {
        signal(SIGHUP, SIG_IGN);
        chdir("/");
        setsid(); 
        
        FILE* fp = fopen(PID_PATH, "w");
        if (fp != NULL) {
            fprintf(fp, "%d", getpid());
            fclose(fp);
        }
        
        close(0);
        close(1);
        close(2);
        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);
        
        noka_record(get_replay_path());
        exit(0);
    } else {
        printf("noka: replay recording enabled\n");
        return 0;
    }
}

int noka_replay_disable() {
    int pid;
    FILE* fp = fopen(PID_PATH, "r");
    if (fp != NULL) {
        fscanf(fp, "%d", &pid);
        fclose(fp);
    }

    if (kill(pid, SIGTERM) == 0) {
        printf("noka: replay recording disabled\n");
    } else {
        perror("noka: kill");
    }

    unlink(PID_PATH);

    return 0;
}