#include <dirent.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  char *pid;
  unsigned long ticks[2];
  double usage;
} procinfo;

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
    path[i + path_head] = '/'; // Add a separator if there isn't one
  i++;

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

  procinfo *table = malloc(50 * sizeof(procinfo));
  size_t tablesize = 0;

  struct dirent *ent;
  while ((ent = readdir(proc)) != NULL) {
    // Check if the name is a PID by seeing if the first char is a number
    char f = ent->d_name[0];
    if (f >= '0' && f <= '9') {
      // Allocate more memory for procinfo if needed
      if (tablesize != 0 && tablesize % 50 == 0) {
        procinfo *new = malloc((tablesize + 50) * sizeof(procinfo));
        memcpy(new, table, tablesize * sizeof(procinfo));

        free(table);
        table = new;
      }

      procinfo *this = table + tablesize;

      // Copy ent->d_name
      size_t pid_len = strlen(ent->d_name);
      this->pid = malloc(pid_len + 1);
      memcpy(this->pid, ent->d_name, pid_len);
      this->pid[pid_len] = '\0';

      // Calculate CPU usage
      const char *stat_path = procfile(ent->d_name, "stat");
      FILE *stat = fopen(stat_path, "r");
      if (!stat) {
        perror("fopen()");
        puts(stat_path);
        return -1;
      }

#if 0
      char buf[50];
      while (true) {
        int read = fread(buf, 1, sizeof buf, stat);
        fwrite(buf, 1, read, stdout);
        if (read < 50)
          break;
      }
      rewind(stat);
#endif

      unsigned long utime, stime;
      // TODO: Clean this up
      fscanf(stat,
             "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu "
             "%lu %lu" /* ignore all but these */,
             &utime, &stime);
      fclose(stat);

      this->ticks[0] = utime;
      this->ticks[1] = stime;

      tablesize++;
    }
  }

  for (size_t i = 0; i < tablesize; i++) {
    printf("%s: %lu & %lu\n", table[i].pid, table[i].ticks[0],
           table[i].ticks[1]);
  }
}