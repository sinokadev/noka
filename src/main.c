#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "replay.h"
#include "save.h"

void print_help(const char *prog) {
    printf(
        "Usage:\n"
        "  %s <command> [subcommand] [options]\n\n"
        "Commands:\n"
        "  replay <action>        Replay control\n\n"
        "Replay actions:\n"
        "  enable                 Enable replay recording\n"
        "  disable                Disable replay recording\n"
        "  ps                     Show replay daemon status\n"
        "  delete                 Delete old replay files\n"
        "  save [-c N]             Save last N replays (default: 1)\n\n"
        "Options:\n"
        "  -h, --help              Show this help message\n",
        prog
    );
}

int main(int argc, char *argv[]) {
    printf("noka version 1.1.3 (built " BUILD_DATE ") Copyright (c) 2025 sinokadev\n");

    // 전역 help
    if (argc == 1 ||
        strcmp(argv[1], "-h") == 0 ||
        strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "replay") == 0) {
        if (argc < 3 ||
            strcmp(argv[2], "-h") == 0 ||
            strcmp(argv[2], "--help") == 0) {

            printf(
                "Usage:\n"
                "  noka replay <enable|disable|ps|delete|save> [options]\n\n"
                "Options:\n"
                "  save -c, --count N      Number of replays to save\n"
            );
            return 0;
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

            for (int i = 3; i < argc; i++) {
                if ((strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--count") == 0) &&
                    i + 1 < argc) {
                    count = atoi(argv[i + 1]);
                    if (count <= 0) count = 1;
                    i++;
                }
                else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                    printf("Usage: noka replay save [-c N]\n");
                    return 0;
                }
                else {
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
