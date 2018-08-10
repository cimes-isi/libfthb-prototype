/**
 * A mock backend using SysV shared memory.
 *
 * @author Connor Imes
 * @date 2018-08-08
 */
#define _XOPEN_SOURCE
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include "fthb.h"

typedef struct fthb {
  unsigned int timeout;
  unsigned int counter;
  int uid;
  int attached;
} fthb;

static key_t uid_to_key(int uid) {
  return ftok("fthb", uid);
}

fthb* fthb_create(int uid, unsigned int timeout) {
  fthb* hb;
  int shmid;
  // create the shared memory page
  if ((shmid = shmget(uid_to_key(uid), sizeof(fthb), IPC_CREAT | 0666)) == -1) {
    return NULL;
  }
  // attach to the shared memory
  if ((hb = (fthb*) shmat(shmid, NULL, 0)) == (fthb*) -1) {
    // try and cleanup shmem
    shmctl(shmid, IPC_RMID, NULL);
    return NULL;
  }
  hb->timeout = timeout;
  hb->counter = 0;
  hb->uid = uid;
  hb->attached = 1;
  return hb;
}

static int uid_to_shmid(int uid) {
  return shmget(uid_to_key(uid), 0, 0);
}

int fthb_destroy(fthb* hb) {
  int shmid = uid_to_shmid(hb->uid);
  // mark for destruction (so HB monitors know to stop monitoring and detach)
  hb->attached = 0;
  if (shmdt(hb) || shmctl(shmid, IPC_RMID, NULL)) {
    return -1;
  }
  return 0;
}

int fthb_issue(fthb* hb) {
  hb->counter++;
  return 0;
}

static fthb* uid_to_fthb(int uid) {
  return (fthb*) shmat(uid_to_shmid(uid), NULL, 0);
}

fthb* fthb_get_hb(int uid) {
  return uid_to_fthb(uid);
}

void fthb_put_hb(fthb* hb) {
  shmdt(hb);
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
  shmdt(hb); // should never fail
  return 0;
}

int fthb_destroy_opaque(int uid) {
  fthb* hb;
  int shmid = uid_to_shmid(uid);
  if ((hb = (fthb*) shmat(shmid, NULL, 0)) == NULL) {
    return -1;
  }
  return fthb_destroy(hb);
}

int fthb_issue_opaque(int uid) {
  fthb* hb;
  int ret;
  if ((hb = uid_to_fthb(uid)) == NULL) {
    return -1;
  }
  ret = fthb_issue(hb);
  shmdt(hb); // should never fail
  return ret;
}
