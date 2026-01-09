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

#include "replay.h"
#include "record.h"
#include "util.h"

#define REPLAY_KEEP_DAYS   60
#define REPLAY_SAFE_HOURS  24
#define REPLAY_ALL_SAVE    0

int delete_old_replay()
{   
    if (REPLAY_ALL_SAVE) {
        printf("noka: no replay delete flag is true\n");
        printf("hint: If you want to delete the replay, change the \"REPLAY_ALL_SAVE\" constant in the src/replay/replay.c file to 0 before compiling.\n");
        return 1;
    }

    DIR* dp = opendir(get_replay_path());
    if (!dp) {
        perror("noka: opendir");
        return -1;
    }

    time_t now = time(NULL);
    struct dirent* entry;

    char fullpath[1024];

    while ((entry = readdir(dp)) != NULL)
    {
        // 디렉터리 스킵
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        
        if (strstr(entry->d_name, "_SAVED") != NULL && ends_with(entry->d_name, ".mp4"))
            continue;

        int y, m, d, hh, mm, ss;

        // 파일명 형식 체크
        if (sscanf(entry->d_name,
                   "replay_%4d%2d%2d_%2d%2d%2d.mp4",
                   &y, &m, &d, &hh, &mm, &ss) != 6)
        {
            continue; // 형식에 맞지 않으면 스킵
        }

        struct tm tm_file = {0};
        tm_file.tm_year = y - 1900;
        tm_file.tm_mon  = m - 1;
        tm_file.tm_mday = d;
        tm_file.tm_hour = hh;
        tm_file.tm_min  = mm;
        tm_file.tm_sec  = ss;
        tm_file.tm_isdst = -1;

        time_t file_time = mktime(&tm_file);
        if (file_time == (time_t)-1)
            continue;

        double diff_sec = difftime(now, file_time);

        // 60일 초과 & 24시간 안전 버퍼
        if (diff_sec > (REPLAY_KEEP_DAYS * 86400) &&
            diff_sec > (REPLAY_SAFE_HOURS * 3600))
        {
            snprintf(fullpath, sizeof(fullpath),
                     "%s/%s", get_replay_path(), entry->d_name);

            if (unlink(fullpath) == 0) {
                printf("deleted: %s\n", fullpath);
            } else {
                perror("noka: unlink");
            }
        }
    }

    closedir(dp);
    return 0;
}

int noka_replay_enable() {
    int pid_file;
    FILE* fp = fopen(get_pid_path(), "r");
    if (fp && fscanf(fp, "%d", &pid_file) == 1) {
        if (kill(pid_file, 0) == 0) {
            printf("noka: replay already enabled\n");
            fclose(fp);
            return 0;
        }
    }
    if (fp) fclose(fp);

    
    printf("noka: create daemon...\n");
    
    int pid = fork();
    if (pid < 0) {
        perror("noka: fork");
        return 1;
    } else if (pid == 0) {
        signal(SIGHUP, SIG_IGN);
        chdir("/");
        setsid(); 
        
        FILE* fp = fopen(get_pid_path(), "w");
        if (fp != NULL) {
            fprintf(fp, "%d", getpid());
            printf("noka: %d\n", getpid());
            fclose(fp);
        }
        
        close(0);
        close(1);
        close(2);
        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);

        // running replay
        delete_old_replay();
        noka_record(get_replay_path());
        exit(0);
    } else {
        printf("noka: replay recording enabled\n");
        return 0;
    }
}


int noka_replay_disable() {
    pid_t pid = -1;

    FILE* fp = fopen(get_pid_path(), "r");
    if (!fp) {
        // pid 파일 없음 = 이미 꺼져 있음
        return 0;
    }

    if (fscanf(fp, "%d", &pid) != 1 || pid <= 1) {
        fclose(fp);
        // pid 파일이 깨져 있으면 정리만 하고 성공 처리
        unlink(get_pid_path());
        printf("noka: replay already disabled\n");
        return 0;
    }
    fclose(fp);

    // 프로세스가 이미 없으면 pid 파일만 정리
    if (kill(pid, 0) != 0) {
        unlink(get_pid_path());
        return 0;
    }

    // ffmpeg 정상 종료 요청
    if (kill(pid, SIGINT) != 0) {
        perror("noka: kill(SIGINT)");
        return 1;
    }

    // 종료 대기 (최대 5초)
    for (int i = 0; i < 50; i++) {
        if (kill(pid, 0) != 0) {
            unlink(get_pid_path());
            printf("noka: replay recording disabled\n");
            return 0;
        }
        usleep(100 * 1000);
    }

    fprintf(stderr, "noka: disable timeout (still running)\n");
    return 1;
}
