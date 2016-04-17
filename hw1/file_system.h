#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include <sys/types.h>

#define O_RDONLY (1 << 0)
#define O_WRONLY (1 << 1)
#define O_RDWR (1 << 2)
#define O_APPEND (1 << 3)
#define O_CREAT (1 << 4)
#define O_EXCL (1 << 5)
#define O_TRUNC (1 << 6)

typedef struct node* DIR;

void file_system_init();
void file_system_print();

int open_file(const char* pathname, int flags);
int close_file(int fd);

ssize_t read(int fd, void* buf, size_t nbyte);
ssize_t write(int fd, const void* buf, size_t nbyte);

int mkdir(const char *path);
DIR* opendir(const char * path);
struct node* readdir(DIR* dirp);
void closedir(DIR* dir);

#endif /* __FILE_SYSTEM_H__ */