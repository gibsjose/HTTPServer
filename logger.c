#include "logger.h"

FILE * gLogFile_FILE;

pthread_mutex_t gFileMutex;

/*
 * Initialize the logger.
 *
 * If a non-null filename is specified, attempt to open it for writing.
 */
void log_init(const char * aFilename)
{
  // Initialize the mutex for accessing the log file.
  pthread_mutex_init(&gFileMutex, NULL);

  if(aFilename != NULL)
  {
    gLogFile_FILE = fopen(aFilename, "w");
  }
  else
  {
    gLogFile_FILE = stdout;
  }
}

/*
 * Clean up the logger.
 *
 * Destroy the mutex used to protect file access.
 * Close the open file only if it is not stdout.
 */
void log_close()
{
  pthread_mutex_destroy(&gFileMutex);

  if(gLogFile_FILE != stdout)
    fclose(gLogFile_FILE);
}

/*
 * Log an INFO level message.
 */
void log_info(const char * aMessage, ...)
{
  va_list lArgList;
  va_start(lArgList, aMessage);
  log_write(LOG_LEVEL_INFO, aMessage, lArgList);
  va_end(lArgList);
}

/*
* Log an ERROR level message.
*/
void log_error(const char * aMessage, ...)
{
  va_list lArgList;
  va_start(lArgList, aMessage);
  log_write(LOG_LEVEL_ERROR, aMessage, lArgList);
  va_end(lArgList);
}


/*
 * Write the log message to the file with the prefix prepended to the message.
 */
void log_write(const char * aPrefix, const char * aMessage, va_list aArgs)
{
  // Prepend the datetime and prefix to the log message indicating its log level.
  // time_t t = time(NULL);
  // struct tm tm = *localtime(&t);
  // printf("now: %d-%d-%d %d:%d:%d\n",
  //   tm.tm_year + 1900,
  //   tm.tm_mon + 1,
  //   tm.tm_mday,
  //   tm.tm_hour,
  //   tm.tm_min,
  //   tm.tm_sec
  // );

  char * lNewMessage = malloc( (strlen(aPrefix) + strlen(aMessage) + 1) * sizeof(char));
  strcpy(lNewMessage, aPrefix);
  strcat(lNewMessage, aMessage);

  //Lock before attempting to write to the log file so that writing is thread safe.
  pthread_mutex_lock(&gFileMutex);
  vfprintf(gLogFile_FILE, lNewMessage, aArgs);
  pthread_mutex_unlock(&gFileMutex);

  free(lNewMessage);
}
