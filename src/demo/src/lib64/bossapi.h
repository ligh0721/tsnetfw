#ifndef BOSSAPI_H
#define BOSSAPI_H

int loginit(unsigned short port);

//int loginit(const char* ip, unsigned short port);

int logprintf(const char* format, ...);

#endif
