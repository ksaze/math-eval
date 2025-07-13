#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void logError(const char *message, const char *funcName) {
  if (errno == 0) {
    fprintf(stderr, "%s", message);
  } else
    perror(message);

  FILE *logfile = fopen("log.txt", "a");
  if (logfile == NULL) {
    fprintf(stderr, "Failed to open log file\n");
    return;
  }

  time_t now;
  struct tm *timeinfo;
  char time_str[20];

  time(&now);
  timeinfo = localtime(&now);
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);

  fprintf(logfile, "--------------------------------------------------\n");
  fprintf(logfile, "Timestamp: %s\n", time_str);
  fprintf(logfile, "Function: %s\n", funcName);
  fprintf(logfile, "Error Message: %s\n", message);
  if (errno != 0)
    fprintf(logfile, "System Error: %s\n", strerror(errno));
  else {
    fprintf(logfile, "Application error: %s\n", message);
  }

  fprintf(logfile, "--------------------------------------------------\n\n");

  fclose(logfile);
}
