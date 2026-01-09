#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "noka/replay.h"
#include "noka/util.h"
#include "noka/save.h"

// replay_YYYYMMDD_HHMMSS.mp4 형식인지 (SAVED 제외)
static int is_plain_replay_mp4(const char* name) {
    // 이미 저장된 것 제외
    if (strstr(name, "_SAVED") != NULL) return 0;

    // 확장자 체크
    if (!ends_with(name, ".mp4")) return 0;

    // 포맷 체크
    int y, m, d, hh, mm, ss;
    if (sscanf(name, "replay_%4d%2d%2d_%2d%2d%2d.mp4",
               &y, &m, &d, &hh, &mm, &ss) != 6) {
        return 0;
    }
    return 1;
}

// scandir 정렬용 (이름 오름차순: 과거 -> 최신)
static int cmp_dirent_name(const struct dirent** a, const struct dirent** b) {
    return strcmp((*a)->d_name, (*b)->d_name);
}

// dst가 존재하면 _SAVED_2, _SAVED_3 ... 로 유니크하게
static int build_unique_saved_path(char* out_dst, size_t out_sz,
                                  const char* dir, const char* filename) {
    // filename에서 ".mp4" 제거한 base 만들기
    char base[512];
    strncpy(base, filename, sizeof(base) - 1);
    base[sizeof(base) - 1] = '\0';

    char* dot = strrchr(base, '.');
    if (!dot) return -1;
    *dot = '\0'; // base = "replay_YYYYMMDD_HHMMSS"

    // 1순위: _SAVED.mp4
    snprintf(out_dst, out_sz, "%s/%s_SAVED.mp4", dir, base);
    if (access(out_dst, F_OK) != 0) {
        // 존재 안 함 -> 사용 가능
        return 0;
    }

    // 2순위부터: _SAVED_2.mp4 ...
    for (int i = 2; i < 10000; i++) {
        snprintf(out_dst, out_sz, "%s/%s_SAVED_%d.mp4", dir, base, i);
        if (access(out_dst, F_OK) != 0) {
            return 0;
        }
    }
    return -1;
}

int rename_last_replays(const char* dir, int N) {
    if (!dir || N <= 0) return 0;

    struct dirent** namelist = NULL;
    int total = scandir(dir, &namelist, NULL, cmp_dirent_name);
    if (total < 0) {
        perror("noka: scandir");
        return -1;
    }

    int renamed = 0;

    for (int i = total - 1; i >= 0 && renamed < N; i--) {
        const char* name = namelist[i]->d_name;

        if (!is_plain_replay_mp4(name)) {
            continue;
        }

        char src[1024];
        char dst[1024];

        snprintf(src, sizeof(src), "%s/%s", dir, name);

        if (build_unique_saved_path(dst, sizeof(dst), dir, name) != 0) {
            fprintf(stderr, "noka: cannot build unique saved name for %s\n", src);
            continue;
        }

        if (rename(src, dst) == 0) {
            printf("saved: %s -> %s\n", src, dst);
            renamed++;
        } else {
            perror("noka: rename");
        }
    }

    // 여기서 딱 한 번만 free
    for (int i = 0; i < total; i++) {
        free(namelist[i]);
    }
    free(namelist);

    return renamed;
}


int noka_save_replay(int count) {
    if (noka_replay_disable()) 
        return 1; // 현재 녹화본 저장

    if (rename_last_replays(get_replay_path(), count) == -1)
        return 1;

    if (noka_replay_enable())
        return 1;
    
    return 0;
}