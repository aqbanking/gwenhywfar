


#ifndef GWEN_PALMOS_UNISTD_H
#define GWEN_PALMOS_UNISTD_H


#define O_CREAT 0
#define O_RDWR  0
#define O_RDONLY 0
#define O_TRUNC 0
#define O_APPEND 0

#define S_IRUSR 0
#define S_IWUSR 0
#define S_ISDIR(x) (0)
#define S_ISREG(x) (0)

struct stat {
  int st_mode;
};


#endif

