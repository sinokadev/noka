gcc -DBUILD_DATE=\"$(date +%Y-%m-%d)\" \
    src/main.c \
    src/replay/replay.c \
    src/replay/record.c \
    src/replay/util.c \
    src/replay/save.c \
    -Iinclude -o noka