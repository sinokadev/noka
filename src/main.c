#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "noka/replay.h"

int main(int argc, char *argv[]) {
    printf("noka version 1.0.0-ubuntu24.04 Copyright (c) 2025 sinokadev(noka)\n");

    if (argc < 2) {
        printf("Usage: %s <command> [subcommand] [action]\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "replay") == 0) {
        if (argc < 3) {  // 이거 추가!
            printf("Usage: noka replay <enable|disable>\n");
            return 1;
        }

        system("mkdir -p ~/noka/temp");
        system("mkdir -p ~/noka/replay");
        
        if (strcmp(argv[2], "enable") == 0) {
            return noka_replay_enable();  // ← return 추가
        } else if (strcmp(argv[2], "disable") == 0) {
            return noka_replay_disable();  // ← return 추가
        } else {
            printf("Unknown subcommand: %s\n", argv[2]);
            return 1;
        }
    }
    else {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }
    
    return 0;
}