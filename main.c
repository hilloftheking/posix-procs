#include <dirent.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Get path for pseudo-file inside /proc/PID
// Example: /proc/23/cmdline
const char *procfile(const char *pidname, const char *fname) {
  // TODO: Avoid PATH_MAX?
  static char path[PATH_MAX] = "/proc/";
  static const size_t path_head = 6;

  // Append pidname after /proc/
  size_t pidlen = strlen(pidname);
  size_t i;
  for (i = 0; i < pidlen; i++) {
    path[i + path_head] = pidname[i];
  }
  if (path[i + path_head] != '/')
    path[(i++) + path_head] = '/'; // Add a separator if there isn't one

  // Append fname after /proc/PID/
  size_t flen = strlen(fname);
  size_t j;
  for (j = 0; j < flen; j++) {
    path[i + path_head + j] = fname[j];
  }
  path[i + path_head + j] = '\0';

  return path;
}

int main() {
  DIR *proc = opendir("/proc");

  struct dirent *ent;
  while ((ent = readdir(proc)) != NULL) {
    // Check if the name is a PID by seeing if the first char is a number
    char f = ent->d_name[0];
    if (f >= '0' && f <= '9') {
      // Get path to the cmdline information for a process
      const char *cmdpath = procfile(ent->d_name, "cmdline");
      FILE *cmdline = fopen(cmdpath, "r");
      if (!cmdline) {
        continue;
      }

      printf("%s : ", ent->d_name);

      // Read cmdline into a buffer and output it
      char buf[50];
      bool rd = false; // Sometimes cmdline has nothing to read
      size_t crd = sizeof buf;
      do {
        crd = fread(buf, 1, sizeof buf, cmdline);
        if (crd) {
          rd = true;
          fwrite(buf, 1, crd, stdout);
        }
      } while (crd == sizeof buf);

      if (!rd)
        puts("NULL"); // Couldn't read cmdline
      else
        puts("");
    }
  }
}