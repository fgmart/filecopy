#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>  // for basename()
#include <utime.h>   // for utime()
#include <errno.h>
#include <fcntl.h>   // for AT_FDCWD
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/stat.h> // for statx

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source file> <destination file or directory>\n", argv[0]);
        return 1;
    }

    struct stat statbuf;
    struct statx statxbuf;
    char destPath[1024];

    // Check if the source file can be opened
    FILE *sourceFile = fopen(argv[1], "rb");
    if (sourceFile == NULL) {
        perror("Error opening source file");
        return 1;
    }

    // Check if the destination is a directory
    if (stat(argv[2], &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
        // Construct the destination path
        snprintf(destPath, sizeof(destPath), "%s/%s", argv[2], basename(argv[1]));
    } else {
        // Use the provided destination path
        strncpy(destPath, argv[2], sizeof(destPath));
        destPath[sizeof(destPath) - 1] = '\0';
    }

    // Open the destination file
    FILE *destFile = fopen(destPath, "wb");
    if (destFile == NULL) {
        perror("Error opening destination file");
        fclose(sourceFile);
        return 1;
    }

    // Byte-by-byte copy
    int byte;
    while ((byte = fgetc(sourceFile)) != EOF) {
        if (fputc(byte, destFile) == EOF) {
            perror("Error writing to destination file");
            fclose(sourceFile);
            fclose(destFile);
            return 1;
        }
    }

    fclose(sourceFile);
    fclose(destFile);

    // Get the source file's times including birth time
    if (statx(AT_FDCWD, argv[1], AT_STATX_SYNC_AS_STAT, STATX_BTIME | STATX_ATIME | STATX_MTIME, &statxbuf) != 0) {
        perror("Error getting source file status with statx");
        return 1;
    }

    // Set the destination file's times
    struct timespec times[2];
    times[0].tv_sec = statxbuf.stx_atime.tv_sec;  // Access time
    times[0].tv_nsec = statxbuf.stx_atime.tv_nsec;
    times[1].tv_sec = statxbuf.stx_mtime.tv_sec;  // Modification time
    times[1].tv_nsec = statxbuf.stx_mtime.tv_nsec;

    if (utimensat(AT_FDCWD, destPath, times, 0) != 0) {
        perror("Error setting destination file times");
        return 1;
    }

    // Print the birth time
    printf("File copied successfully to %s with preserved access, modification times, and birth time: %ld.%ld.\n",
           destPath, statxbuf.stx_btime.tv_sec, statxbuf.stx_btime.tv_nsec);
    return 0;
}
