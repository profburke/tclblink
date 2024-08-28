#ifndef TCLBLINK_H
#define TCLBLINK_H

#include <tcl.h>
#include "blink1-lib.h"

#define PACKAGE_VERSION "1.0"

typedef struct Blinker {
  blink1_device *device;
} Blinker;

typedef struct BlinkState {
  Tcl_HashTable hash;
  int uid;
} BlinkState;

int BlinkCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
static int BlinkOpen(Tcl_Interp *interp, BlinkState *statePtr, Tcl_Obj *objPtr);
static int BlinkClose(Blinker *blinkPtr);
void BlinkCleanup(ClientData data);
int BlinkDelete(Blinker *blinkPtr, Tcl_HashEntry *entryPtr);

#endif /* TCLBLINK_H */
