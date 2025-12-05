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

#include "noka/replay.h"
#include "noka/record.h"

char* get_pid_path() {
    static char path[512];
    snprintf(path, sizeof(path), "%s/noka/temp/noka_replay.pid", getenv("HOME"));
    return path;
}

int noka_replay_enable() {
    // 이미 실행 중인지 체크
    if (access(get_pid_path(), F_OK) == 0) {
        printf("noka: replay already enabled\n");
        return 1;
    }
    
    printf("Create Daemon...\n");
    
    int pid = fork();
    if (pid < 0) {
        perror("noka: fork");
        return 1;
    } else if (pid == 0) {
        signal(SIGHUP, SIG_IGN);
        chdir("/");
        setsid(); 
        
        FILE* fp = fopen(get_pid_path(), "w");
        if (fp) {
            fprintf(fp, "%d", getpid());
            fclose(fp);
        }
        
        close(0);
        close(1);
        close(2);
        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);
        
        noka_record("~/noka/replay");
        exit(0);
    } else {
        printf("noka: replay recording enabled\n");
        return 0;
    }
}

int noka_replay_disable() {
    int pid;
    FILE* fp = fopen(get_pid_path(), "r");
    if (fp) {
        fscanf(fp, "%d", &pid);
        fclose(fp);
    }

    if (kill(pid, SIGTERM) == 0) {
        printf("noka: replay recording disabled\n");
    } else {
        perror("noka: kill");
    }

    unlink(get_pid_path());

    return 0;
}