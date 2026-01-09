#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

const char* get_replay_path(void) {
    static char path[512];
    snprintf(path, sizeof(path), "%s/noka/replay", getenv("HOME"));
    return path;
}

const char* get_pid_path(void) {
    static char path[512];
    snprintf(path, sizeof(path),
             "/run/user/%d/noka.pid", getuid());
    return path;
}

int ends_with(const char* s, const char* suf) {
    size_t n = strlen(s), m = strlen(suf);
    return (n >= m) && (strcmp(s + (n - m), suf) == 0);
}
