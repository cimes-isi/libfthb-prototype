/**
 * A utility to monitor heartbeats.
 *
 * @author Connor Imes
 * @date 2018-08-09
 */
// for usleep
#define _XOPEN_SOURCE 500
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fthb.h"

#define MAX_UIDS 1024
#define TIMEOUT_US 1000000
#define NUM_HBS 10

static void* heartbeat_provider_thread(void* arg) {
  int uid = *((int*) arg);
  unsigned int i;
  fthb* hb;
  if ((hb = fthb_create(uid, TIMEOUT_US)) == NULL) {
    perror("fthb_create");
    return NULL;
  }
  for (i = 0; i < NUM_HBS; i++) {
    if (fthb_issue(hb)) {
      perror("fthb_issue");
      // need to break to cleanup before return
      break;
    }
    printf("Heartbeat issued for: %d\n", uid);
    usleep(TIMEOUT_US / 2);
  }
  if (fthb_destroy(hb)) {
    perror("fthb_destroy");
  }
  return NULL;
}

static void* heartbeat_consumer_thread(void* arg) {
  int uid = *((int*) arg);
  unsigned int i;
  unsigned int counter = 0;
  unsigned int counter_last;
  unsigned int timeout_us;
  fthb* hb;
  if ((hb = fthb_get_hb(uid)) == NULL) {
    fprintf(stderr, "Could not find heartbeat with UID=%d\n", uid);
    return NULL;
  }
  timeout_us = fthb_get_timeout(hb);
  for (i = 0; ; i++) {
    counter_last = counter;
    if (counter == NUM_HBS) {
      break;
    }
    usleep(timeout_us * 2);
    if (!fthb_is_tracking(hb)) {
      // producer is finished
      printf("Heartbeat unregistered for: %d\n", uid);
      break;
    }
    counter = fthb_read_counter(hb);
    printf("Heartbeat read for: %d\n", uid);
    if (counter == counter_last) {
      fprintf(stderr, "Heartbeat timed out, UID=%d, counter=%u\n", uid, counter);
      // need to break to put/detach the heartbeat before return
      break;
    }
  }
  fthb_put_hb(hb);
  return NULL;
}

int main(int argc, char** argv) {
  int uids[MAX_UIDS] = { 0 };
  pthread_t threads[MAX_UIDS];
  void* (*fn) (void*);
  unsigned int n;
  unsigned int i;
  if (argc < 3) {
    fprintf(stderr, "Usage: %s 0|1 UID [UID]...\n", argv[0]);
    fprintf(stderr, "0 to be provider, 1 to be consumer\n");
    return EINVAL;
  }
  fn = atoi(argv[1]) ? heartbeat_consumer_thread : heartbeat_provider_thread;
  for (i = 2, n = 0; i < argc; i++, n++) {
    if (i >= MAX_UIDS) {
      fprintf(stderr, "Too many UIDs, max=%u\n", MAX_UIDS);
      return -1;
    }
    uids[n] = atoi(argv[i]);
  }
  for (i = 0; i < n; i++) {
    errno = pthread_create(&threads[i], NULL, fn, &uids[i]);
    if (errno) {
      perror("pthread_create");
    }
  }
  for (i = 0; i < n; i++) {
    errno = pthread_join(threads[i], NULL);
    if (errno) {
      perror("pthread_join");
    }
  }
  return 0;
}
