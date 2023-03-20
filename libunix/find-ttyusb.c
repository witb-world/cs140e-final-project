// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
	"ttyUSB",	// linux
    "ttyACM",   // linux
	// "cu.SLAB_USB", // mac os
    // "cu.usbserial", // mac os
    "tty.usbserial-1410", // mac os (added EW 1/31/23)
    // if your system uses another name, add it.
	0
};

static int num_prefixes = sizeof ttyusb_prefixes / sizeof ttyusb_prefixes[0];

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
  for (int i = 0; i < num_prefixes; i++) {
    if (ttyusb_prefixes[i] == 0) break;
    if (strncmp(d->d_name, ttyusb_prefixes[i], strlen(ttyusb_prefixes[i])) == 0 )
      return 1;
  }
  // no match found, return 0
  return 0;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    char *dirname = strdupf("/dev/");
    struct dirent ***namelist =  (struct dirent***) malloc(1024*1024);
    output("running scandir. Addr of namelist: %p\n", &alphasort);
    int result = scandir(dirname, namelist, filter, alphasort);
    if (result != 1) {
      panic("Found %d devices, needed one and only one.\n", result);
    }
    const char *device_name = strdupf((*namelist[0])->d_name);
    size_t dst_size = strlen(dirname) + strlen(device_name);
    strlcat(dirname,device_name, dst_size + 1);
    return dirname;
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) {
    char *dirname = strdupf("/dev/");
    struct dirent **namelist;
    int result = scandir(dirname, &namelist, filter, alphasort);
    if (result == 0) {
      panic("No devices found\n");
    }
    struct dirent *most_recent_ttyusb = namelist[0];
    unsigned most_recent_time = 0;
    struct stat stat_buf;

    for (int i = 0; i < result; i++) {
      int s = stat(namelist[i]->d_name, &stat_buf);
      if (stat_buf.st_mtimespec.tv_nsec > most_recent_time) {
        most_recent_time = stat_buf.st_mtimespec.tv_nsec;
        most_recent_ttyusb = namelist[i];
      } 
    }
    size_t dst_size = strlen(dirname) + strlen(most_recent_ttyusb->d_name);
    const char* device_name = strdupf(most_recent_ttyusb->d_name);
    strlcat(dirname,device_name, dst_size + 1);
    return dirname;
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
    char *dirname = strdupf("/dev/");
    struct dirent **namelist;
    int result = scandir(dirname, &namelist, filter, alphasort);
    if (result == 0) {
      panic("No devices found\n");
    }
    struct dirent *oldest_ttyusb = namelist[0];
    unsigned oldest_time = UINT_MAX;
    struct stat stat_buf;

    for (int i = 0; i < result; i++) {
      int s = stat(namelist[i]->d_name, &stat_buf);
      if (stat_buf.st_mtimespec.tv_nsec < oldest_time) {
        oldest_time = stat_buf.st_mtimespec.tv_nsec;
        oldest_ttyusb = namelist[i];
      } 
    }

    const char* device_name = strdupf(oldest_ttyusb->d_name);
    size_t dst_size = strlen(dirname) + strlen(device_name);
    strlcat(dirname, device_name, dst_size + 1);
    return dirname;
}
