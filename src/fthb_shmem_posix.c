/**
 * A mock backend using POSIX shared memory.
 *
 * @author Connor Imes
 * @date 2018-08-09
 */
// >= 500 for ftruncate
#define _XOPEN_SOURCE 500
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "fthb.h"

typedef struct fthb {
  unsigned int timeout;
  unsigned int counter;
  int uid;
  int attached;
} fthb;

#define PAGE_SIZE() sysconf(_SC_PAGE_SIZE)

fthb* fthb_create(int uid, unsigned int timeout) {
  fthb* hb;
  int shm_fd;
  char name[16];
  size_t size = PAGE_SIZE();
  sprintf(name, "%d", uid);
  // create the shared memory page
  if ((shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666)) == -1) {
    return NULL;
  }
  if (ftruncate(shm_fd, size) == -1 ||
      (hb = mmap(0, size, PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (fthb*) -1) {
    // (try to) cleanup
    shm_unlink(name);
    close(shm_fd);
    return NULL;
  }
  // seems to be OK to close the fd after mmap completes
  close(shm_fd);
  hb->timeout = timeout;
  hb->counter = 0;
  hb->uid = uid;
  hb->attached = 1;
  return hb;
}

int fthb_destroy(fthb* hb) {
  char name[16];
  sprintf(name, "%d", hb->uid);
  // mark for destruction (so HB monitors know to stop monitoring and detach)
  hb->attached = 0;
  munmap(hb, PAGE_SIZE());
  // close(shm_fd);
  return shm_unlink(name);
}

int fthb_issue(fthb* hb) {
  hb->counter++;
  return 0;
}

fthb* fthb_get_hb(int uid) {
  fthb* hb;
  int shm_fd;
  char name[16];
  sprintf(name, "%d", uid);
  if ((shm_fd = shm_open(name, O_RDWR, 0666)) == -1) {
    return NULL;
  }
  hb = (fthb*) mmap(0, PAGE_SIZE(), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  close(shm_fd);
  return hb;
}

void fthb_put_hb(fthb* hb) {
  munmap(hb, PAGE_SIZE());
}

unsigned int fthb_get_timeout(const fthb* hb) {
  return hb->timeout;
}

unsigned int fthb_read_counter(fthb* hb) {
  return hb->counter;
}

int fthb_create_opaque(int uid, unsigned int timeout) {
  fthb* hb;
  if ((hb = fthb_create(uid, timeout)) == NULL) {
    return -1;
  }
  munmap(hb, PAGE_SIZE());
  return 0;
}

int fthb_destroy_opaque(int uid) {
  fthb* hb;
  char name[16];
  int shm_fd;
  int ret;
  sprintf(name, "%d", uid);
  if ((shm_fd = shm_open(name, O_RDWR, 0666)) == -1) {
    return -1;
  }
  if ((hb = (fthb*) mmap(0, PAGE_SIZE(), PROT_WRITE, MAP_SHARED, shm_fd, 0)) == NULL) {
    close(shm_fd);
    return -1;
  }
  close(shm_fd);
  ret = fthb_destroy(hb);
  return ret;
}

int fthb_issue_opaque(int uid) {
  fthb* hb;
  int shm_fd;
  char name[16];
  int ret;
  size_t size = PAGE_SIZE();
  sprintf(name, "%d", uid);
  if ((shm_fd = shm_open(name, O_RDWR, 0666)) == -1) {
    return -1;
  }
  if ((hb = (fthb*) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == NULL) {
    close(shm_fd);
    return -1;
  }
  close(shm_fd);
  ret = fthb_issue(hb);
  munmap(hb, size);
  return ret;
}
