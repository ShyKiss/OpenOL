#pragma once
#include <stddef.h>  // size_t

// POSIX-like functions implemented via raw Linux syscalls (OLLinuxBridge.cpp + OLLinuxSyscall.asm)
int   OLLinux_shm_open (const char* name, int oflag, unsigned int mode);
int   OLLinux_ftruncate(int fd, long long length);
void* OLLinux_mmap     (void* addr, size_t length, int prot, int flags, int fd, long long offset);
int   OLLinux_munmap   (void* addr, size_t length);
int   OLLinux_socket   (int domain, int type, int protocol);
int   OLLinux_bind     (int sockfd, const void* addr, unsigned int addrlen);
int   OLLinux_listen   (int sockfd, int backlog);
int   OLLinux_accept   (int sockfd, void* addr, unsigned int* addrlen);
long  OLLinux_recv     (int sockfd, void* buf, size_t len, int flags);
long  OLLinux_send     (int sockfd, const void* buf, size_t len, int flags);
int   OLLinux_close    (int fd);
int   OLLinux_fcntl    (int fd, int cmd, long arg);
int   OLLinux_unlink   (const char* path);

// mmap result sentinel
#define OL_MAP_FAILED ((void*)-1)

bool OLLinuxBridge_Load();
void OLLinuxBridge_Unload();
