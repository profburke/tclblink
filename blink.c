#include "blink.h"

void BlinkCleanup(ClientData data) {
  BlinkState *statePtr = (BlinkState *)data;
  Blinker *blinkPtr;
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

// TODO: is statePtr necessary?
// Return a count of attached Blink(1) devices.
static int BlinkEnumerate(Tcl_Interp *interp, BlinkState *statePtr) {
  Tcl_Obj *objPtr = Tcl_NewIntObj(blink1_enumerate());
  Tcl_SetObjResult(interp, objPtr);
  
  return TCL_OK;
}

// TODO: is statePtr necessary?
static int BlinkList(Tcl_Interp *interp, BlinkState *statePtr) {
  Tcl_Obj *const objects[0];
  Tcl_Obj *listPtr = Tcl_NewListObj(0, objects);

  Tcl_Obj *devIDKeyPtr = Tcl_NewStringObj("Device ID", -1);
  Tcl_Obj *serialKeyPtr = Tcl_NewStringObj("Serial", -1);
  Tcl_Obj *typeKeyPtr = Tcl_NewStringObj("Type", -1);
  
  int n = blink1_enumerate();
  for (int i = 0; i < n; i++) {
    Tcl_Obj *dict = Tcl_NewDictObj();
    Tcl_DictObjPut(interp, dict, devIDKeyPtr, Tcl_NewIntObj(i));
    Tcl_DictObjPut(interp, dict, serialKeyPtr, Tcl_NewStringObj(blink1_getCachedSerial(i), -1));
    Tcl_DictObjPut(interp, dict, typeKeyPtr, Tcl_NewIntObj(blink1_deviceTypeById(i)));

    if (Tcl_ListObjAppendElement(interp, listPtr, dict) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, listPtr);
  return TCL_OK;
}

static int BlinkSetRGB(Blinker *blinkPtr, int red, int green, int blue) {
  if (blinkPtr == NULL) {
    // TODO: some sort of error message
    return TCL_ERROR;
  }

  if (blinkPtr->device == NULL) {
    // TODO: return an error message
    return TCL_ERROR;
  }

  int result = blink1_setRGB(blinkPtr->device, red, green, blue);
  // TODO: use result

  return TCL_OK;
}

#define SET(Color, r, g, b) \
  static int Blink##Color(Blinker *blinkPtr) { \
    return BlinkSetRGB(blinkPtr, r, g, b); \
  }

SET(On, 255, 255, 255)

SET(Off, 0, 0, 0)

SET(Black, 0, 0, 0)

SET(White, 255, 255, 255)

SET(Red, 255, 0, 0)

SET(Green, 0, 255, 0)

SET(Blue, 0, 0, 255)

SET(Cyan, 0, 255, 255)

SET(Magenta, 255, 0, 255)

SET(Yellow, 255, 255, 0)

SET(Orange, 255, 165, 0)

// Given a device ID, "open" the corresponding device.
// TODO: for now we're only implementing opening by device ID
// we'll come back and do serial # later
static int BlinkOpen(Tcl_Interp *interp, BlinkState *statePtr, Tcl_Obj *objPtr) {
  Tcl_HashEntry *entryPtr;
  int new;
  Blinker *blinkPtr;
  char name[20];
  int devid;

  if (objPtr != NULL) {
    if (Tcl_GetIntFromObj(interp, objPtr, &devid) != TCL_OK) {
      return TCL_ERROR;
    }
  } else {
    // shouldn't be possible
    return TCL_ERROR;
  }

  int n = blink1_enumerate(); // need to enumerate before trying to create a device

  blinkPtr = Tcl_Alloc(sizeof(Blinker));
  blinkPtr->device = NULL;
  blinkPtr->device = blink1_openById(devid);

  if (blinkPtr->device == NULL) {
    // signal error
    printf("Got an error opening device\n");

    return TCL_ERROR; // TCL_ERROR or a blink-specific error?
  }

  sprintf(name, "blink%d", devid);
  entryPtr = Tcl_CreateHashEntry(&statePtr->hash, name, &new);
  Tcl_SetHashValue(entryPtr, (ClientData)blinkPtr);

  Tcl_SetStringObj(Tcl_GetObjResult(interp), name, -1);
  // Tcl_Obj *resultPtr = Tcl_NewStringObj(name, -1);
  // Tcl_SetObjResult(interp, resultPtr);

  return TCL_OK;
}

static int BlinkClose(Blinker *blinkPtr) {
  if (blinkPtr->device != NULL) {
    blink1_setRGB(blinkPtr->device, 0, 0, 0);
    blink1_close(blinkPtr->device);
    blinkPtr->device = NULL;
  }

  return TCL_OK;
}

int BlinkDelete(Blinker *blinkPtr, Tcl_HashEntry *entryPtr) {
  Tcl_DeleteHashEntry(entryPtr);

  if (blinkPtr->device != NULL) {
    // close the blink device ...
  }

  Tcl_EventuallyFree((char *)blinkPtr, Tcl_Free);
  return TCL_OK;
}

static int BlinkVid(Tcl_Interp *interp) {
  Tcl_Obj *objPtr = Tcl_NewIntObj(blink1_vid());
  Tcl_SetObjResult(interp, objPtr);

  return TCL_OK;
}

static int BlinkPid(Tcl_Interp *interp) {
  Tcl_Obj *objPtr = Tcl_NewIntObj(blink1_pid());
  Tcl_SetObjResult(interp, objPtr);

  return TCL_OK;
}

// TODO: would this be simpler as an ensemble command?
int BlinkCmd(ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[]) {
  BlinkState *statePtr = (BlinkState *)data;
  Tcl_Obj *valueObjPtr;

  char *subCmds[] = {
    "vid", "pid", "enumerate", "list",
    "open", "close", "set",
    "on", "off", "black", "white",
    "red", "green", "blue", "cyan",
    "magenta", "yellow", "orange",
    NULL
  };

  enum BlinkIx {
    VidIx, PidIx, EnumerateIx, ListIx,
    OpenIx, CloseIx, SetIx, 
    OnIx, OffIx, BlackIx, WhiteIx,
    RedIx, GreenIx, BlueIx, CyanIx,
    MagentaIx, YellowIx, OrangeIx,
  };
  int result, index;

  if (objc == 1 || objc > 6 /* ??? */) {
    Tcl_WrongNumArgs(interp, 1, objv, "option ... TBD ...");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1], subCmds, "option", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }
  
  // More processing
  // valueObjPtr = objv[3];
  valueObjPtr = NULL;

  switch(index) {
  case EnumerateIx:
    return BlinkEnumerate(interp, statePtr);
  case ListIx:
    return BlinkList(interp, statePtr);
  case OpenIx:
    return BlinkOpen(interp, statePtr, objv[2]);
  case VidIx:
    return BlinkVid(interp);
  case PidIx:
    return BlinkPid(interp);
  }

  // The following commands take a blink name as the third parameter:

  Tcl_HashEntry *entryPtr = Tcl_FindHashEntry(&statePtr->hash, Tcl_GetString(objv[2]));
  if (entryPtr == NULL) {
    Tcl_AppendResult(interp, "Unknown blink: ", Tcl_GetString(objv[2]), NULL);
    return TCL_ERROR;
  }
  Blinker *blinkPtr = (Blinker *)Tcl_GetHashValue(entryPtr);
  
  switch(index) {
  case CloseIx:
    return BlinkClose(blinkPtr);
  case OnIx:
    return BlinkOn(blinkPtr);
  case OffIx:
    return BlinkOff(blinkPtr);
  case BlackIx:
    return BlinkBlack(blinkPtr);
  case WhiteIx:
    return BlinkWhite(blinkPtr);
  case RedIx:
    return BlinkRed(blinkPtr);
  case GreenIx:
    return BlinkGreen(blinkPtr);
  case BlueIx:
    return BlinkBlue(blinkPtr);
  case CyanIx:
    return BlinkCyan(blinkPtr);
  case MagentaIx:
    return BlinkMagenta(blinkPtr);
  case YellowIx:
    return BlinkYellow(blinkPtr);
  case OrangeIx:
    return BlinkOrange(blinkPtr);
  case SetIx:
    {
      // TODO: validating ...
      int r, g, b;
      Tcl_GetIntFromObj(interp, objv[3], &r);
      Tcl_GetIntFromObj(interp, objv[4], &g);
      Tcl_GetIntFromObj(interp, objv[5], &b);
      return BlinkSetRGB(blinkPtr, r, g, b);
    }
  }

  // TODO: shouldn't really allow dropping through ... is it even possible?
  return TCL_OK;
}

/********************************************
 *
 * Library entry point.
 *
 ********************************************/
int Blink_Init(Tcl_Interp *interp) {
  BlinkState *statePtr;

  if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
    return TCL_ERROR;
  }
  
  statePtr = (BlinkState *)Tcl_Alloc(sizeof(BlinkState));
  Tcl_InitHashTable(&statePtr->hash, TCL_STRING_KEYS); // TCL_STRING_KEYS or something else ?
  statePtr->uid = 0;
  
  Tcl_CreateObjCommand(interp, "blink", BlinkCmd, (ClientData)statePtr, BlinkCleanup);

  return TCL_OK;
}
