#include "tcl.h"
#include "blink1-lib.h"

typedef struct Blink {
  int someInfo;
} Blink;

typedef struct BlinkState {
  Tcl_HashTable hash;
  int uid;
} BlinkState;

int BlinkCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]);
int BlinkCreate(Tcl_Interp *interp, BlinkState *statePtr);
void BlinkCleanup(ClientData data);
int BlinkDelete(Blink *blinkPtr, Tcl_HashEntry *entryPtr);

int BlinkEnumerate(Tcl_Interp *interp, BlinkState *statePtr);

int Blink_Init(Tcl_Interp *interp) {
  BlinkState *statePtr;

  statePtr = (BlinkState *)Tcl_Alloc(sizeof(BlinkState));
  Tcl_InitHashTable(&statePtr->hash, TCL_STRING_KEYS);
  statePtr->uid = 0;
  Tcl_CreateObjCommand(interp, "blink", BlinkCmd, (ClientData)statePtr, BlinkCleanup);

  return TCL_OK;
}

void BlinkCleanup(ClientData data) {
  BlinkState *statePtr = (BlinkState *)data;
  Blink *blinkPtr;
  Tcl_HashEntry *entryPtr;
  Tcl_HashSearch search;

  entryPtr = Tcl_FirstHashEntry(&statePtr->hash, &search);
  while (entryPtr != NULL) {
    blinkPtr = Tcl_GetHashValue(entryPtr);
    BlinkDelete(blinkPtr, entryPtr);
    entryPtr = Tcl_FirstHashEntry(&statePtr->hash, &search);
  }
  Tcl_Free((char *)statePtr);
}

int BlinkCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
  BlinkState *statePtr = (BlinkState *)data;

  char *subCmds[] = {
    "enumerate", NULL
  };
  enum BlinkIx {
    EnumerateIx
  };
  int result, index;

  if (objc == 1 || objc > 4 /* ??? */) {
    Tcl_WrongNumArgs(interp, 1, objv, "option ... TBD ...");
    return TCL_ERROR;
  }

  // More processing

  if (index == EnumerateIx) {
    return BlinkEnumerate(interp, statePtr);
  }
  
  return TCL_OK;
}

int BlinkCreate(Tcl_Interp *interp, BlinkState *statePtr) {
  Tcl_HashEntry *entryPtr;
  Blink *blinkPtr;
  int new;
  char name[20];

  statePtr->uid++;
  sprintf(name, "blink%d", statePtr->uid);
  entryPtr = Tcl_CreateHashEntry(&statePtr->hash, name, &new);

  blinkPtr = (Blink *)Tcl_Alloc(sizeof(Blink));
  blinkPtr->someInfo = 0;
  Tcl_SetHashValue(entryPtr, (ClientData)blinkPtr);

  Tcl_SetStringObj(Tcl_GetObjResult(interp), name, -1);
  return TCL_OK;
}

int BlinkDelete(Blink *blinkPtr, Tcl_HashEntry *entryPtr) {
  Tcl_DeleteHashEntry(entryPtr);

  Tcl_EventuallyFree((char *)blinkPtr, Tcl_Free);
  return TCL_OK;
}

int BlinkEnumerate(Tcl_Interp *interp, BlinkState *statePtr) {
  Tcl_Obj *objPtr;

  objPtr = Tcl_NewIntObj(blink1_enumerate());

  return TCL_OK;
}

  
