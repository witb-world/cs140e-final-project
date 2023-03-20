#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// allocate buffer, read entire file into it, return it.   
// buffer is zero padded to a multiple of 4.
//
//  - <size> = exact nbytes of file.
//  - for allocation: round up allocated size to 4-byte multiple, pad
//    buffer with 0s. 
//
// fatal error: open/read of <name> fails.
//   - make sure to check all system calls for errors.
//   - make sure to close the file descriptor (this will
//     matter for later labs).
// 
void *read_file(unsigned *size, const char *name) {
    // How: 
    //    - use stat to get the size of the file.
    //    - round up to a multiple of 4.
    //    - allocate a buffer
    //    - zero pads to a multiple of 4.
    //    - read entire file into buffer.  
    //    - make sure any padding bytes have zeros.
    //    - return it.   
    struct stat file_stat;
    int res = stat(name, &file_stat);
    if (res != 0) {
      panic("Could not stat\n");
    }
    unsigned pad_size = 0, 
             file_size_raw = file_stat.st_size;
    // printf("Raw file size: %d\n", file_size_raw);
    unsigned rem;
    if ((rem = file_size_raw % 4) != 0) {
      pad_size = (4 - rem); // round up to 4.
                            // presume this won't overflow ¯\_(ツ)_/¯
    }

    // printf("Pad size: %d\n", pad_size);
    char *buf = malloc(file_size_raw + pad_size);
    // zero padding
    for (int i = 0; i < pad_size; i++) {
      *(buf + file_size_raw + i) = 0;
    }
    int fd = open(name, O_RDONLY);
    if (fd < 0) {
      panic("invalid fd\n");
    }
    read(fd, buf, file_size_raw);
    close(fd);
    // set size arg to file_size_raw
    *size = file_size_raw;
    // if any padding pytes not zeroed, return with error
    for (int i = 0; i < pad_size; i++) {
      if (*(buf + file_size_raw + i) != 0)
        panic("padding bytes not zeroed.");
    }
    // printf("Current buf: %s\n", buf);

    return buf;
}
