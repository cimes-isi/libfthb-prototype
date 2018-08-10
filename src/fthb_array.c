/**
 * A mock backend using a global fixed-size array, perhaps like might back a hashmap.
 *
 * @author Connor Imes
 * @date 2018-08-08
 */
#include <errno.h>
#include <stdlib.h>
#include "fthb.h"

typedef struct fthb {
  unsigned int timeout;
  unsigned int counter;
  int uid;
  int tracking;
} fthb;

#define PAGE_SIZE 4096
#define MAX_HBS (PAGE_SIZE / sizeof(fthb))

// This is essentially our shared memory region
static fthb HB_ARR[MAX_HBS];

static int uid_to_idx(int uid) {
  // A practical implementation would be more like a hashmap
  return uid % MAX_HBS;
}

static fthb* uid_to_fthb(int uid) {
  int i;
  if ((i = uid_to_idx(uid)) < 0) {
    return NULL;
  }
  return &HB_ARR[i];
}

fthb* fthb_create(int uid, unsigned int timeout) {
  fthb* hb;
  if ((hb = uid_to_fthb(uid)) == NULL) {
    return NULL;
  }
  if (hb->tracking) {
    if (hb->uid == uid) {
      // heartbeat is already being used, I guess that's OK
      return hb;
    }
    // conflict in the hashing algorithm
    errno = EADDRINUSE;
    return NULL;
  }
  hb->timeout = timeout;
  hb->counter = 0;
  hb->uid = uid;
  hb->tracking = 1;
  return hb;
}

int fthb_destroy(fthb* hb) {
  hb->tracking = 0;
  return 0;
}

int fthb_issue(fthb* hb) {
  hb->counter++;
  return 0;
}

fthb* fthb_get_hb(int uid) {
  return uid_to_fthb(uid);
}

void fthb_put_hb(fthb* hb) {
  (void) hb;
  // nothing to do
}

unsigned int fthb_get_timeout(const fthb* hb) {
  return hb->timeout;
}

unsigned int fthb_read_counter(fthb* hb) {
  return hb->counter;
}

int fthb_create_opaque(int uid, unsigned int timeout) {
  return fthb_create(uid, timeout) == NULL ? -1 : 0;
}

int fthb_destroy_opaque(int uid) {
  fthb* hb;
  if ((hb = uid_to_fthb(uid)) == NULL) {
    return -1;
  }
  return fthb_destroy(hb);
}

int fthb_issue_opaque(int uid) {
  fthb* hb;
  if ((hb = uid_to_fthb(uid)) == NULL) {
    return -1;
  }
  return fthb_issue(hb);
}
