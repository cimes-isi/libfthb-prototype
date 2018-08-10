/**
 * Test heartbeats
 *
 * @author Connor Imes
 * @date 2018-08-09
 */
#include <stdlib.h>
#include <stdio.h>
#include "fthb.h"

#define TIMEOUT 100

int main(int argc, char** argv) {
  fthb* hb;
  int uid = 0;
  if (argc > 1) {
    uid = atoi(argv[1]);
  }
  // normal case
  if ((hb = fthb_create(uid, TIMEOUT)) == NULL) {
    perror("fthb_create");
    return -1;
  }
  if (fthb_issue(hb)) {
    perror("fthb_issue");
    return -1;
  }
  if (fthb_destroy(hb)) {
    perror("fthb_destroy");
    return -1;
  }
  // opaque case
  if (fthb_create_opaque(uid, TIMEOUT)) {
    perror("fthb_create_opaque");
    return -1;
  }
  if (fthb_issue_opaque(uid)) {
    perror("fthb_issue_opaque");
    return -1;
  }
  if (fthb_destroy_opaque(uid)) {
    perror("fthb_destroy_opaque");
    return -1;
  }
  return 0;
}
