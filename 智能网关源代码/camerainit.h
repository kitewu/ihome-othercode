#ifndef CAMERAINIT_H
#define CAMERAINIT_H

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#include <QDebug>
void errno_exit(const char* s);
int xioctl(int fd, int request, void* argp);
void deviceOpen(void);
void deviceInit(void);
void mmapInit(void);
void captureStart(void);
void mainLoop(unsigned char* src);
unsigned char* frameRead(unsigned char* src);
unsigned char* imageProcess(const void* p,unsigned char* dst);
void deviceUninit(void);

inline int fb_init(void);

#endif // CAMERAINIT_H
