#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "replay.h"
#include "save.h"

int main(int argc, char *argv[]) {
    printf("noka version 1.1.2 (built " BUILD_DATE ") Copyright (c) 2025 sinokadev\n");

    if (argc < 2) {
        printf("Usage: %s <command> [subcommand] [action]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "replay") == 0) {
        if (argc < 3) {
            printf("Usage: noka replay <enable|disable|save>\n");
            return 1;
        }

        system("mkdir -p ~/noka/temp");
        system("mkdir -p ~/noka/replay");

        if (strcmp(argv[2], "enable") == 0) {
            return noka_replay_enable();
        }
        else if (strcmp(argv[2], "disable") == 0) {
            return noka_replay_disable();
        }
        else if (strcmp(argv[2], "ps") == 0) {
            return noka_replay_ps();
        }
        else if (strcmp(argv[2], "delete") == 0) {
            return noka_delete_old_replay();
        }
        else if (strcmp(argv[2], "save") == 0) {
            int count = 1;

            // argv[3]부터 옵션 파싱
            for (int i = 3; i < argc; i++) {
                if ((strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--count") == 0) &&
                    i + 1 < argc) {
                    count = atoi(argv[i + 1]);
                    if (count <= 0) count = 1;
                    i++; // 값 하나 소비
                } else {
                    printf("Unknown option: %s\n", argv[i]);
                    return 1;
                }
            }

            return noka_save_replay(count);
        }

        else {
            printf("Unknown subcommand: %s\n", argv[2]);
            return 1;
        }
    }
    else {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }
}
