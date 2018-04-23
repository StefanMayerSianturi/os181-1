/*
 Copyright (C) 2013-2018 CC BY-SA 3.0
 adapted from https://stackoverflow.com/
 questions/5656530/
 how-to-use-shared-memory-with-linux-in-c

 Copyright 2018 Rahmat M. Samik-Ibrahim
 You are free to SHARE (copy and 
 redistribute the material in any medium
 or format) and to ADAPT (remix, 
 transform, and build upon the material 
 for any purpose, even commercially).
 This program is distributed in the hope 
 that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied 
 warranty of MERCHANTABILITY or FITNESS 
 FOR A PARTICULAR PURPOSE.

 * REV02 Mon Apr 23 11:54:46 WIB 2018
 * REV00 Wed Apr 18 19:50:01 WIB 2018
 * START Xxx Xxx XX XX:XX:XX WIB 2013
 */

// DO NOT USE THE SAME SEMAPHORE NAME!!!!
#define SEM_SYNC  "/semaphore-sync-rms46"
#define SEM_MUTEX "/semaphore-mutex-rms46"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SEM_PERMS  \
 (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define O_FLAGS (O_CREAT | O_EXCL)
#define PROT    (PROT_READ | PROT_WRITE)
#define VISIBILITY \
            (MAP_ANONYMOUS | MAP_SHARED)

#define KIRIM 0
#define AMBIL 1
#define LOOP  5

typedef struct {
   int    produk;
   int    turn;
   int    loop;
   sem_t* mutex;
   sem_t* sync;
}  buffer;

buffer* persiapan(buffer* buf) {
   buf = (buffer* ) mmap(NULL, 
            sizeof(buffer), PROT, 
            VISIBILITY, 0, 0);
   buf->produk = 0;
   buf->turn   = KIRIM;
   buf->loop   = 0;
   sem_unlink(SEM_SYNC);
   sem_unlink(SEM_MUTEX);
   buf->sync  = sem_open(SEM_SYNC, 
              O_FLAGS, SEM_PERMS, 0);
   buf->mutex = sem_open(SEM_MUTEX, 
              O_FLAGS, SEM_PERMS, 1);
   return buf;
}

void kirim (buffer* buf) {
   printf("KR KIRIM PID[%d]\n", getpid());
   while (buf->loop < LOOP) {
      sem_wait(buf->mutex);
      if (buf->turn == KIRIM) {
         printf("KR %d\n",++(buf->produk));
         buf->turn = AMBIL;
      }
      if (buf->loop++ == 0) 
         sem_post(buf->sync);
      sem_post(buf->mutex);
   }
}

void ambil (buffer* buf) {
   printf("AM AMBIL PID[%d]\n", getpid());
   sem_wait(buf->sync);
   while (buf->loop < LOOP) {
      sem_wait(buf->mutex);
      if(buf->turn == AMBIL) {
         printf("AM %d\n", buf->produk);
         buf->turn = KIRIM;
      }
      buf->loop++;
      sem_post(buf->mutex);
   }
}

void main(void) {
   printf("START PID[%d]\n", getpid());
   buffer* share_buf=persiapan(share_buf);
   if (fork()) kirim (share_buf);
   else        ambil (share_buf);
   wait(NULL);
   sem_unlink(SEM_SYNC);
   sem_unlink(SEM_MUTEX);
   printf("STOP PID[%d]\n", getpid());
}

//       1         2         3         4
// 4567890123456789012345678901234567890

