#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "noka/record.h"

char* get_system_audio() {
    static char device[256];
    FILE* fp = popen("pactl get-default-sink", "r");
    if (fp) {
        fgets(device, sizeof(device), fp);
        device[strcspn(device, "\n")] = 0;
        strcat(device, ".monitor");
        pclose(fp);
        printf("%s", device);
        return device;
    }
    return "default";
}

void noka_record(char path[256]) {
    char output_path[300];
    char audio_device[256];
    
    // 미리 복사해두기
    strncpy(audio_device, get_system_audio(), sizeof(audio_device) - 1);
    audio_device[255] = '\0';
    
    sprintf(output_path, "%s/replay_%%Y%%m%%d_%%H%%M%%S.mp4", path);
    
    execlp("ffmpeg", "ffmpeg", 
           "-f", "x11grab", 
           "-i", ":0.0",
           "-f", "pulse",
           "-i", audio_device,  // 복사한 거 사용
           "-f", "segment", 
            "-segment_time", "180",  // 3분
           "-reset_timestamps", "1",
           output_path, 
           NULL);
    
    perror("ffmpeg exec failed");
    exit(1);
}