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
#define TIMEOUT_US 100000
#define NUM_HBS 20

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
    usleep(timeout_us * 2);
    // TODO: Need to know when heartbeat is no longer active rather than just counting them - register a callback? Poll?
    if (counter == NUM_HBS) {
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
  pthread_t provider_threads[MAX_UIDS];
  pthread_t consumer_threads[MAX_UIDS];
  unsigned int i;
  for (i = 1; i < argc; i++) {
    if (i >= MAX_UIDS) {
      fprintf(stderr, "Too many UIDs, max=%u\n", MAX_UIDS);
      return -1;
    }
    uids[i - 1] = atoi(argv[i]);
  }
  for (i = 0; i < argc - 1; i++) {
    errno = pthread_create(&provider_threads[i], NULL, heartbeat_provider_thread, &uids[i]);
    if (errno) {
      perror("pthread_create: provider");
    }
    errno = pthread_create(&consumer_threads[i], NULL, heartbeat_consumer_thread, &uids[i]);
    if (errno) {
      perror("pthread_create: consumer");
    }
  }
  for (i = 0; i < argc - 1; i++) {
    errno = pthread_join(consumer_threads[i], NULL);
    if (errno) {
      perror("pthread_join: consumer");
    }
    errno = pthread_join(provider_threads[i], NULL);
    if (errno) {
      perror("pthread_join: provider");
    }
  }
  return 0;
}
